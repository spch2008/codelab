class TimerThread {
public:
  struct Task;
  class Bucket;

  typedef uint64_t TaskId;
  const static TaskId INVALID_TASK_ID;

  TimerThread();
  ~TimerThread();

  // Start the timer thread.
  // This method should only be called once.
  // return 0 if success, errno otherwise.
  int start(const TimerThreadOptions* options);

  // Stop the timer thread. Later schedule() will return INVALID_TASK_ID.
  void stop_and_join();

  // Schedule |fn(arg)| to run at realtime |abstime| approximately.
  // Returns: identifier of the scheduled task, INVALID_TASK_ID on error.
  TaskId schedule(void (*fn)(void*), void* arg, const timespec& abstime);

  // Prevent the task denoted by `task_id' from running. `task_id' must be
  // returned by schedule() ever.
  // Returns:
  //   0   -  Removed the task which does not run yet
  //  -1   -  The task does not exist.
  //   1   -  The task is just running.
  int unschedule(TaskId task_id);

private:
  void run();
  static void* run_this(void* arg);

  bool _started;
  butil::atomic<bool> _stop;

  std::mutex _mutex;
  Bucket* _buckets;
  int64_t _nearest_run_time;

  pthread_t _thread;
};
