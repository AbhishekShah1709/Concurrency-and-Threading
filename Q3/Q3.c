#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>

int n,m,k;
int finished_count;
int totalRiders;

struct values{
    char cabtype[100];
    int ridingtime;
    int waitingtime;
    pthread_mutex_t lock2;
    int ind;
    int left;   //0 if riding and 1 if left
};

struct values data[10000];

typedef struct Cabs{
    int ind;
    int state;  // 0 means wait and 1 means busy
    int no_of_riders;
    int type;   // 0 means premier and 1 means pool
    int ridetime;
    pthread_mutex_t lock;
    pthread_t thread_ID;
}cab;

struct Cabs taxis[1000];

typedef struct pay{
    int ind;
    int state;
    pthread_mutex_t lock1;
    pthread_t thread_ID;
}pay_server;

struct pay paymentservers[1000];

void* paymentServer(void *args)
{ 
    struct pay *paydata = (struct pay *)args;
    int pay_idx = paydata->ind;

    int rider_idx;
    while(1)
    {
        for(int i=0;i<m;i++)
        {
            pthread_mutex_lock(&data[i].lock2);
            if(data[i].left==1)
            {
                if(paymentservers[pay_idx].state==0)
                {
                    paymentservers[pay_idx].state=1;
                    rider_idx = data[i].ind;
                    data[i].left=0;
                    
                    pthread_mutex_unlock(&data[i].lock2);
 
                    printf("Rider %d is paying through payment server number %d\n",rider_idx,pay_idx);
                    fflush(stdout);
                   
                    sleep(2);

                    printf("Payment is done by rider %d through payment server number %d\n",rider_idx,pay_idx);
                    fflush(stdout);
                    finished_count++;

                    paymentservers[pay_idx].state=0;
                    break;
                }
            }
            pthread_mutex_unlock(&data[i].lock2);
        }
    }
}

void* check(void *args)
{
    struct values *vals = (struct values*) args;

    char type[100];
    int rdtime = vals->ridingtime;
    int wttime = vals->waitingtime;
    int rid_ind = vals->ind;
    strcpy(type,vals->cabtype);

    printf("Rider %d has requested a cab\n",rid_ind);
    
    time_t start;
    time_t end;
    time(&start);

    while(1)
    {
        time(&end);
        double diff;
        diff = difftime(end,start);
        if(diff>wttime)
        {
            printf("Rider %d did not find any cab\n",rid_ind);
            totalRiders--;
            pthread_exit(NULL);
        }

        if(strcmp(type,"Premier")==0)
        {
            for(int i=0;i<n;i++)
            {
                pthread_mutex_lock(&taxis[i].lock);

                if(taxis[i].state==0)
                {
                    taxis[i].type=0;
                    taxis[i].no_of_riders=1;
                    taxis[i].state=1;
                    taxis[i].ridetime=rdtime;

                    int index = taxis[i].ind;

                    printf("Rider %d got premier ride in cab number %d and has riding time of %d\n",rid_ind,index,rdtime);
                    fflush(stdout);

                    pthread_mutex_unlock(&taxis[i].lock);
                    int sl_time = taxis[index-1].ridetime;

                    sleep(sl_time);

                    printf("Rider %d dropped off at the location by cab number %d\n",rid_ind,index);
                    fflush(stdout);
                    data[rid_ind-1].left=1;
                    
                    taxis[index-1].state=0;
                    taxis[index-1].ridetime=0;
                    taxis[index-1].no_of_riders=0;

                    pthread_exit(NULL);
                }
                pthread_mutex_unlock(&taxis[i].lock);
            }
        }

        else
        {
            for(int i=0;i<n;i++)
            {
                pthread_mutex_lock(&taxis[i].lock);

                if(taxis[i].state==1 && taxis[i].no_of_riders==1 && taxis[i].type==1)
                {
                    taxis[i].type=1;
                    taxis[i].no_of_riders=2;
                    taxis[i].state=1;
                    taxis[i].ridetime=rdtime;

                    int index = taxis[i].ind;

                    printf("Rider %d got pool ride in cab number %d and has riding time of %d\n",rid_ind,index,rdtime);
                    fflush(stdout);

                    pthread_mutex_unlock(&taxis[i].lock);

                    int sl_time = taxis[index-1].ridetime;
                    sleep(sl_time);

                    printf("Rider %d dropped off at the location by cab number %d \n",rid_ind,index);
                    fflush(stdout);
                    taxis[index-1].no_of_riders--;
                    
                    if(taxis[index-1].no_of_riders==1)
                    {
                        taxis[index-1].state=1;
                        taxis[index-1].type=1;
                    }
                    else
                        taxis[index-1].state=0;

                    taxis[index-1].ridetime=0;
                    data[rid_ind-1].left=1;

                    pthread_exit(NULL);
                }
                pthread_mutex_unlock(&taxis[i].lock);
            }

            for(int i=0;i<n;i++)
            {
                pthread_mutex_lock(&taxis[i].lock);

                if(taxis[i].state==0)
                {
                    taxis[i].type=1;
                    taxis[i].no_of_riders=1;
                    taxis[i].state=1;
                    taxis[i].ridetime=rdtime;

                    int index = taxis[i].ind;

                    printf("Rider %d got pool ride in cab number %d and has riding time of %d\n",rid_ind,index,rdtime);
                    fflush(stdout);
                    pthread_mutex_unlock(&taxis[i].lock);
                    
                    int sl_time = taxis[index-1].ridetime;
                    sleep(sl_time);
                    
                    printf("Rider %d dropped off at the location by cab number %d\n",rid_ind,index);
                    fflush(stdout);

                    taxis[index-1].no_of_riders--;
                    if(taxis[index-1].no_of_riders==0)
                    {
                        taxis[index-1].state=0;
                    }
                    else
                    {
                        taxis[index-1].state=1;
                        taxis[index-1].type=1;
                    }

                    taxis[index-1].ridetime=0;

                    data[rid_ind-1].left=1;

                    pthread_exit(NULL);
                }

                pthread_mutex_unlock(&taxis[i].lock);
            }
        }
    }
}

int main()
{
    printf("Number of Riders:- ");
    scanf("%d",&m);
    printf("Number of Cabs:- ");
    scanf("%d",&n);
    printf("Number of Payment Servers:- ");
    scanf("%d",&k);
    
    char type[100];
    totalRiders=m;

    pthread_t riders[m];
    pthread_t payment[k];

    for(int i=0;i<n;i++)
    {
        taxis[i].ind=i+1;
        taxis[i].state=0;
        taxis[i].type=-1;
        taxis[i].no_of_riders=0;
        taxis[i].ridetime=0;

        pthread_mutex_init(&taxis[i].lock,NULL);
    }

    for(int i=0;i<k;i++)
    {
        paymentservers[i].ind=i+1;
        paymentservers[i].state=0;

        pthread_mutex_init(&paymentservers[i].lock1,NULL);
    }

    for(int i=0;i<k;i++)
        pthread_create(&payment[k],NULL,paymentServer,&paymentservers[i]);

    srand(time(0));
    for(int j=0;j<m;j++)
    {
        int time_ran = rand()%4;
        sleep(time_ran);
        int r = rand()%2;
        if(r==0)
            strcpy(type,"Premier");
        else
            strcpy(type,"Pool");

        int rdtime = (rand()%10 + 1);
        int wttime = (rand()%10 + 1);

        pthread_mutex_init(&data[j].lock2,NULL);

        strcpy(data[j].cabtype,type);
        data[j].ridingtime = rdtime;
        data[j].waitingtime = wttime;
        data[j].ind = j+1;
        data[j].left = 0;

        pthread_create(&riders[j],NULL,check,&data[j]);
    }
    
    while(finished_count!=totalRiders)
    {
    }
}
