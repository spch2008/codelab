/**
* @file   lower_bound.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 16:15:41
* @brief
**/

#include <stdio.h>

int lower_bound(int *arr, int len, int val)
{
    int l = 0;
    int u = len - 1;

    while (l <= u)
    {
        int mid = l + (u - l)/2; 
        if (arr[mid] < val)
        {
            l = mid + 1;
        }
        else
        {
            u = mid - 1;
        }
    }

    if (arr[l] == val)
    {
        return l;
    }
    else
    {
        return -1;
    }
}

int main()
{
    int arr[] = { 1, 2, 5, 5, 5, 8, 9, 10};
    int len   = sizeof(arr) / sizeof(int);

    int pivot = lower_bound(arr, len, 5);
    if (pivot == -1)
    {
        printf("not found\n");
    }
    else
    {
        printf("found, slot: %d\n", pivot);
    }

    return 0;
}
