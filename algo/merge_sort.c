/**
* @file   merge_sort.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 16:48:05
* @brief
**/

#include <stdio.h>
#include <stdlib.h>

void merge(int *arr, int *help, int l, int mid, int u)
{
    int lbegin = l;
    int lend   = mid;

    int rbegin = mid + 1;
    int rend   = u;

    int num = 0;
    while (lbegin <= lend && rbegin <= rend)
    {
        if (arr[lbegin] < arr[rbegin])
        {
            help[num++] = arr[lbegin++];
        }
        else
        {
            help[num++] = arr[rbegin++];
        }
    }

    while (lbegin <= lend)
    {
        help[num++] = arr[lbegin++];
    }

    while (rbegin <= rend)
    {
        help[num++] = arr[rbegin++];
    }

    int i;
    for (i = 0; i < num; i++)
    {
        arr[l++] = help[i];
    }
}

void merge_impl(int *arr, int l, int u, int *help)
{
    if (l < u)
    {
        int mid = l + (u - l)/2;
        merge_impl(arr, l, mid, help);
        merge_impl(arr, mid+1, u, help);
        merge(arr, help, l, mid, u);
    }
}

void merge_sort(int *arr, int len)
{
    if (len <= 1)
    {
        return;
    }

    int *help = (int*)malloc(len * sizeof(int));
    if (help != NULL)
    {
        merge_impl(arr, 0, len-1, help);
        free(help);
    }
}

int main()
{
    int arr[] = {4, 8, 1, 9, 3, 2, 0, 5};
    int len   = sizeof(arr) / sizeof(int);

    merge_sort(arr, len);
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
