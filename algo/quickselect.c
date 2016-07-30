/**
* @file   quickselect.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 15:44:33
* @brief
**/
#include <stdio.h>

void swap(int *arr, int i, int j)
{
    int tmp = arr[i];
    arr[i]  = arr[j];
    arr[j]  = tmp;
}

int select(int *arr, int l, int u, int k)
{
    if (l > u)
    {
        return -1;
    }

    int i = l;
    int j = u+1;

    while (1)
    {
        do 
        {
            i++;
        } while ( i < j && arr[i] < arr[l]);

        do
        {
            j--;
        } while (arr[j] > arr[l]);

        if (i > j)
        {
            break;
        }

        swap(arr, i, j);
    }

    swap(arr, j , l);

    if (j == k)
    {
        return j;
    }
    else if (j > k)
    {
        return select(arr, l, j-1, k);
    }
    else
    {
        return select(arr, j+1, u, k);
    }
}

int qselect(int *arr, int len, int k)
{
    if (k+1 > len)
    {
        return -1;
    }

    return select(arr, 0, len-1, k);
}

int main()
{
    int arr[] = { 7, 9, 3, 0, 1, 6, 2, 4, 5, 8};
    int len   = sizeof(arr) / sizeof(int);

    int i;
    for (i = 0; i <= len; i++)
    {
        int slot = qselect(arr, len, i);
        if (slot == -1)
        {
            printf("k: %d not found\n", i);
        }
        else
        {
            printf("k: %d, slot: %d, val: %d\n", i, slot, arr[slot]);
        }
    }
    return 0;
}
