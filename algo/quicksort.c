/**
* @file   quicksort.c
* @author sunpengcheng(spch2008@foxmail.com)
* @date   2016-07-30 15:31:59
* @brief
**/

#include <stdio.h>

void swap(int *arr, int i, int j)
{
    int tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

void qsort(int *arr, int l, int u)
{
    if (l >= u)
    {
        return;
    }

    int i = l;
    int j = u + 1;

    while (1)
    {
        do 
        { 
            i++; 
        } while (i < j && arr[i] < arr[l]);

        do 
        { 
            j--; 
        } while ( arr[j] > arr[l]);

        if ( i > j)
        {
            break;
        }

        swap(arr, i, j);
    }

    swap(arr, j, l);

    qsort(arr, l, j - 1);
    qsort(arr, j+1, u);
}

int main()
{
    int arr[] = { 4, 2, 0, -1,5, 9, 12, 90, -14, 3, 100};
    int len   = sizeof(arr) / sizeof(int);

    qsort(arr, 0, len - 1);

    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
