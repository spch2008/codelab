/**
* @file   binsearch.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 16:09:43
* @brief
**/

#include <stdio.h>

int bsearch(int *arr, int len, int val)
{
    int l = 0;
    int u = len - 1;

    while (l <= u)
    {
        int mid = l + (u - l) / 2;

        if (arr[mid] == val)
        {
            return mid;
        }
        else if (arr[mid] > val)
        {
            u = mid - 1;
        }
        else
        {
            l = mid + 1;
        }
    }

    return -1;
}

int main()
{
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int len   = sizeof(arr) / sizeof(int);

    int target = bsearch(arr, len, 6);
    if (target == -1)
    {
        printf("not found\n");
    }
    else
    {
        printf("found slot: %d\n", target);
    }

    return 0;
}
