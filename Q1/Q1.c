//#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

int* shar_mem_reg(int size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}

struct thread_args{
    int left;
    int right;
    int* arr;    
};

void swap(int *a,int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void insertion_sort(int arr[],int low,int high)
{
    for(int i=low+1;i<=high;i++)
    {
        int element = arr[i];
        int j=i-1;

        while(j>=low && arr[j]>element)
        {
            arr[j+1]=arr[j];
            j--;
        }

        int* p3 = &arr[j+1];
        *p3=element;
    }
}

int sorting(int arr[],int low,int high)
{
    int pivot = arr[high];
    int j=low;
    int i=low-1;

    for(;j<high;j++)
    {
        if(arr[j]<pivot)
        {
            i++;
            swap(&arr[j],&arr[i]);
        }
    }

    swap(&arr[i+1],&arr[high]);
    return i+1;
}

void normal_quicksort(int arr[],int low,int high)
{
    if(low<high)
    {
        int pivot = sorting(arr,low,high);

        if(high-low+1<=5)
        {
            insertion_sort(arr,low,high);
            return;
        }

        else
        {
            normal_quicksort(arr,0,pivot-1);
            normal_quicksort(arr,pivot+1,high);
        }

        return;
    }

    else
        return;
}

void *threaded_quicksort(void* a)
{
    struct thread_args *vals = (struct thread_args*) a;

    int low = vals->left;
    int high = vals->right;
    int *arr = vals->arr;

    if(low<high)
    {
        if(high-low+1<=5)
        {
            insertion_sort(arr,low,high);
            return NULL;
        }

        else
        {
            int pivot = sorting(arr,low,high);

            pthread_t tid1;

            struct thread_args a1;
            a1.left = low;
            a1.right= pivot-1;
            a1.arr = arr;


            pthread_create(&tid1,NULL,threaded_quicksort,&a1);

            pthread_t tid2;
            struct thread_args a2;
            a2.left = pivot+1;
            a2.right= high;
            a2.arr = arr;

            pthread_create(&tid2,NULL,threaded_quicksort,&a2);

            pthread_join(tid1,NULL);
            pthread_join(tid2,NULL);

            return NULL;
        }
    }
    else
        return NULL;
}

void quicksort(int arr[],int low,int high)
{
    if(low<high)
    {
        int pivot = sorting(arr,low,high);

        int pid = fork();

        if(pid==0)
        {
            if(pivot>5)
                quicksort(arr,0,pivot-1);
            else
                insertion_sort(arr,0,pivot-1);
            _exit(1);
        }
        else
        {
            int pid1 = fork();


            if(pid1==0)
            {
                if(high-pivot>5)
                    quicksort(arr,pivot+1,high);
                else
                    insertion_sort(arr,pivot+1,high);
                _exit(1);
            }
            else
            {
                int status;
                waitpid(pid,&status,0);
                waitpid(pid1,&status,0);
            }
        }
        return;
    }
}

void diffSorts(long long int n)
{
    //getting shared memory
    int *arr = shar_mem_reg(sizeof(int)*(n+1));
    int brr[n+1];
    int crr[n+1];

    for(int i=0;i<n;i++) 
    {
        scanf("%d", arr+i);
        brr[i]=*(arr+i);
        crr[i]=*(arr+i);
    }
    
    clock_t t = clock();

    //multiprocess quicksort
    quicksort(arr, 0, n-1);

    clock_t p = clock();

    printf("Time taken for quicksort by multiprocess method %Lf\n",(long double)((p-t)/(long double)CLOCKS_PER_SEC));

// --------------------------------------------------------------------------------------------------------------------------

    pthread_t tid;
    struct thread_args a;
    a.left = 0;
    a.right = n-1;
    a.arr = brr;

    clock_t c = clock();

    //multithreaded quicksort
    pthread_create(&tid,NULL,threaded_quicksort,&a);
    pthread_join(tid,NULL);


    clock_t b = clock();
    
    printf("Time taken for quicksort by thread method %Lf\n",(long double)((b-c)/(long double)CLOCKS_PER_SEC));

 // -------------------------------------------------------------------------------------------------------------------------
    
    clock_t d = clock();
    
    //normal quicksort
    normal_quicksort(crr,0,n-1);


    clock_t e = clock();
    
    printf("Time taken for quicksort by normal method %Lf\n",(long double)((e-d)/(long double)CLOCKS_PER_SEC));

// --------------------------------------------------------------------------------------------------------------------------

    printf("Normal quicksort ran %Lf times faster than multiprocess quicksort\n",(long double)(p-t)/(long double)(e-d));
    printf("Normal quicksort ran %Lf times faster than multithreaded quicksort\n",(long double)(b-c)/(long double)(e-d));

    shmdt(arr);
}

int main()
{
    int n;
    scanf("%d",&n);

    diffSorts(n);

    return 0;
}
