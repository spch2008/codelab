/**
* @file   partition.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 16:00:46
* @brief
**/

#include <stdio.h>

void swap(int *arr, int i, int j)
{
    int tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

int partition(int *arr, int len)
{
    if (len <= 1)
    {
        return 0;
    }

    int i = -1;
    int j = len + 1;

    while (1)
    {
        do
        {
            i++;
        } while ( i < j && arr[i] < 0);

        do
        {
            j--;
        } while ( j > i && arr[j] >= 0);

        if (i > j)
        {
            break;
        }

        swap(arr, i, j);
    }

    return j;
}

int main()
{
    int arr[] = {1, 4, -4, 0, 8, -5, 0, 7, -12, 55};
    int len   = sizeof(arr) / sizeof(int);

    int pivot = partition(arr, len);
    int i;
    for (i = 0; i <= pivot; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    for (i = pivot+1; i < len; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
