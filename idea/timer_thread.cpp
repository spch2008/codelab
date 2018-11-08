struct TimerThread::Task {
  Task* next;           // For linking tasks in a Bucket.
  int64_t run_time;     // run the task at this realtime
  void (*fn)(void*);    // the fn(arg) to run
  void* arg;

  TaskId task_id;

  // initial_version:     not run yet
  // initial_version + 1: running
  // initial_version + 2: removed (also reused this struct)
  butil::atomic<uint32_t> version;

  Task() : version(2/*skip 0*/) {}

  // Run this task and delete this struct.
  bool run_and_delete();

  // Delete this struct if this task was unscheduled.
  bool try_delete();
};

// 使用bucket来分散竞争
class TimerThread::Bucket {
 public:
  Bucket()
    : _nearest_run_time(std::numeric_limits<int64_t>::max())
    , _task_head(NULL) { }
  ~Bucket() {}

  struct ScheduleResult {
    TimerThread::TaskId task_id;
    bool earlier;
  };
  
  ScheduleResult schedule(void (*fn)(void*), void* arg,
                          const timespec& abstime);

  Task* consume_tasks();

 private:
  std::mutex _mutex;
  int64_t _nearest_run_time;
  Task* _task_head;
};

void* TimerThread::run_this(void* arg) {
  static_cast<TimerThread*>(arg)->run();
  return NULL;
}

TimerThread::TimerThread()
  : _started(false)
  , _stop(false)
  , _buckets(NULL)
  , _nearest_run_time(std::numeric_limits<int64_t>::max())
  , _thread(0) {
}

TimerThread::~TimerThread() {
  stop_and_join();
  delete [] _buckets;
  _buckets = NULL;
}

int TimerThread::start(const TimerThreadOptions* options_in) {
  if (_started) {
    return 0;
  }

  _buckets = new (std::nothrow) Bucket[_options.num_buckets];
  if (NULL == _buckets) {
    return 0;
  }

  // 创建timmer线程
  pthread_create(&_thread, NULL, TimerThread::run_this, this);
  _started = true;
  return 0;
}

TimerThread::Task* TimerThread::Bucket::consume_tasks() {
  Task* head = NULL;
  if (_task_head) {
    BAIDU_SCOPED_LOCK(_mutex);
    if (_task_head) {
      head = _task_head;
      _task_head = NULL;
      _nearest_run_time = std::numeric_limits<int64_t>::max();
    }
  }
  return head;
}

TimerThread::Bucket::ScheduleResult
TimerThread::Bucket::schedule(void (*fn)(void*), void* arg,
                              const timespec& abstime) {
    butil::ResourceId<Task> slot_id;
    Task* task = butil::get_resource<Task>(&slot_id);

    task->next = NULL;
    task->fn = fn;
    task->arg = arg;
    task->run_time = butil::timespec_to_microseconds(abstime);

    task->version.fetch_add(2, butil::memory_order_relaxed);
    uint32_t version = task->version.load(butil::memory_order_relaxed);

    task->task_id = make_task_id(slot_id, version);
    bool earlier = false;
    {
        BAIDU_SCOPED_LOCK(_mutex);
        task->next = _task_head; // link
        _task_head = task;

        // 加入了要过期的Task
        if (task->run_time < _nearest_run_time) {
          _nearest_run_time = task->run_time;
          earlier = true;
        }
    }

    return {id, earlier};
}

TimerThread::TaskId TimerThread::schedule(
  void (*fn)(void*), void* arg, const timespec& abstime) {
  // Hashing by pthread id is better for cache locality.
  const Bucket::ScheduleResult result = 
      _buckets[pthread_self() % options._num_buckets].schedule(fn, arg, abstime);

  if (result.earlier) { // bucket 内有更小超时的加入
    bool earlier = false;
    const int64_t run_time = butil::timespec_to_microseconds(abstime);
    {
      // 新加入的比当前要过期的时间还早，及时处理
      BAIDU_SCOPED_LOCK(_mutex);
      if (run_time < _nearest_run_time) {
        _nearest_run_time = run_time;
        earlier = true;
      }
    }

    if (earlier) {
      // 唤醒worker进行处理
      wake_up_sleep_thread();
    }
  }
  return result.task_id;
}

int TimerThread::unschedule(TaskId task_id) {
  const butil::ResourceId<Task> slot_id = slot_of_task_id(task_id);
  Task* const task = butil::address_resource(slot_id);

  const uint32_t id_version = version_of_task_id(task_id);
  uint32_t expected_version = id_version;

  // 如果没有运行与删除，标记为运行结束，等timmer_thread销毁
  if (task->version.compare_exchange_strong(
      expected_version, id_version + 2, std::memory_order_acquire)) {
    return 0;
  }

  return (expected_version == id_version + 1) ? 1 : -1;
}

bool TimerThread::Task::run_and_delete() {
  const uint32_t id_version = version_of_task_id(task_id);
  uint32_t expected_version = id_version;

  if (version.compare_exchange_strong(
      expected_version, id_version + 1, butil::memory_order_relaxed)) {
      fn(arg);

      version.store(id_version + 2, butil::memory_order_release);
      butil::return_resource(slot_of_task_id(task_id));
      return true;
  } else if (expected_version == id_version + 2) {
      // 已经调度结束了
      butil::return_resource(slot_of_task_id(task_id));
      return false;
  } else {
      // Impossible.
      return false;
  }
}

bool TimerThread::Task::try_delete() {
    const uint32_t id_version = version_of_task_id(task_id);

    // 相等，是没有运行
    // 调用时候，不可能出现为+1的情况
    // +2 被删除了，清理掉
    
    if (version.load(butil::memory_order_relaxed) != id_version) {
      CHECK_EQ(version.load(butil::memory_order_relaxed), id_version + 2);
      butil::return_resource(slot_of_task_id(task_id));
      return true;
    }
    return false;
}

void TimerThread::run() {
  std::vector<Task*> tasks;
  tasks.reserve(4096);

  while (!_stop.load(butil::memory_order_relaxed)) {
    // 赋值给最大后，能够感知操作过程中，添加的最小超时时间
    {
      BAIDU_SCOPED_LOCK(_mutex);
      _nearest_run_time = std::numeric_limits<int64_t>::max();
    }
    
    for (size_t i = 0; i < _options.num_buckets; ++i) {
      Bucket& bucket = _buckets[i];
      for (Task* p = bucket.consume_tasks(); p != NULL; p = p->next) {
        if (!p->try_delete()) { // remove the task if it's unscheduled
          tasks.push_back(p);
          std::push_heap(tasks.begin(), tasks.end(), task_greater);
        }
      }
    }

    while (!tasks.empty()) {
      Task* task1 = tasks[0];
      if (task1->try_delete()) { // 从放入到执行，有个时间窗口
        std::pop_heap(tasks.begin(), tasks.end(), task_greater);
        tasks.pop_back();
        continue;
      }

      if (butil::gettimeofday_us() < task1->run_time) {
        break;
      }

      std::pop_heap(tasks.begin(), tasks.end(), task_greater);
      tasks.pop_back();
      task1->run_and_delete();
    }

    // The realtime to wait for.
    int64_t next_run_time = std::numeric_limits<int64_t>::max();
    if (tasks.empty()) {
      next_run_time = std::numeric_limits<int64_t>::max();
    } else {
      next_run_time = tasks[0]->run_time;
    }

    BAIDU_SCOPED_LOCK(_mutex);
    if (next_run_time > _nearest_run_time) {
      // a task is earlier that what we would wait for.
      // We need to check buckets.
      continue;
    } else {
      _nearest_run_time = next_run_time;
    }

    const int64_t now = butil::gettimeofday_us();
    const int64_t dif = next_run_time - now;

    wait_for_timeout_or_signal(diff);
  }
}

void TimerThread::stop_and_join() {
  _stop.store(true, butil::memory_order_relaxed);
  if (_started) {
    {
       BAIDU_SCOPED_LOCK(_mutex);
       _nearest_run_time = 0;
       wake_up_sleep_thread();
    }

    if (pthread_self() != _thread) {
      pthread_join(_thread, NULL);
    }
  }
}

// Utilies for making and extracting TaskId.
inline TimerThread::TaskId make_task_id(
    butil::ResourceId<TimerThread::Task> slot, uint32_t version) {
    return TimerThread::TaskId((((uint64_t)version) << 32) | slot.value);
}

inline
butil::ResourceId<TimerThread::Task> slot_of_task_id(TimerThread::TaskId id) {
    butil::ResourceId<TimerThread::Task> slot = { (id & 0xFFFFFFFFul) };
    return slot;
}

inline uint32_t version_of_task_id(TimerThread::TaskId id) {
    return (uint32_t)(id >> 32);
}

inline bool task_greater(const TimerThread::Task* a, const TimerThread::Task* b) {
    return a->run_time > b->run_time;
}
