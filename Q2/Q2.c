#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

void* prepare_biryani(void *arg);
int containerCapacity[1000];
int n,m,k;
int num;
int inx=0;
int no_of_slots[10000];

struct robIdx{
    int ind;
};

typedef struct serIdx{
    int ind;
    int state;
    int slots;
    int capacity;
    pthread_mutex_t lock4;
    pthread_t thread_ID;
}vessel;

struct students{
    int state;
    int student_num;
    int vessel_no;
};

struct robIdx robotIndex[10000];
vessel *vessels;
struct students stud[10000];

void student_food(struct students *s)
{
    int p = s->vessel_no;
        
    printf("Student %d is getting food at table %d\n",s->student_num,p);
    fflush(stdout);
    s->state=1;
}

void* student_in_slot(void *args)
{
    struct students *s = (struct students *)args;
    printf("Student %d has arrived in the mess\n",s->student_num);
    printf("Student %d is waiting to be allocated a slot\n",s->student_num);

    while(!s->state)
    {
        for(int i=0;i<n;i++)
        {
            pthread_mutex_lock(&vessels[i].lock4);
    
            if(vessels[i].slots>0)
            {
                if(vessels[i].state==1)
                {
                    printf("Student %d has been alloted a slot on table %d\n",s->student_num,vessels[i].ind);
                    fflush(stdout);
                    s->vessel_no = i+1;
                    vessels[i].capacity--;
                    vessels[i].slots--;
                    s->state=2;
                    k--;

                    pthread_mutex_unlock(&vessels[i].lock4);
                    break;
                }
            }
            pthread_mutex_unlock(&vessels[i].lock4);
            
        }

        if(s->vessel_no>0)
        { 
            if(s->state==2)
            {
                student_food(s);
            }
        }
    }
    pthread_exit(NULL);
}

void ready_to_serve(struct serIdx *v)
{
    while(1)
    {
        if(k==0)
            return;

        v->state=1;
        int slots = v->slots; 
        
        if(slots==0)
        {
            printf("Container %d which had capacity %d has finished all it's slots\n",v->ind,v->capacity);
            fflush(stdout);
            
            v->state=2;

            return;
        }

        if(k==0)
            return;
    }
}

void* serve(void *args)
{
    struct serIdx *v = (struct serIdx *)args;

    while(1)
    {

        if(k==0)
            return NULL;
       
        while(!v->state)
        {
        }

        printf("Container %d entering serving phase\n",v->ind);
        fflush(stdout);

        
        v->slots = ((rand()%10)+1);

        if(v->slots>v->capacity)
            v->slots=v->capacity;

        printf("Container %d with capacity equal to %d is having %d available slots\n",v->ind,v->capacity,v->slots);
        fflush(stdout);

        ready_to_serve(v);

        if(v->capacity==0)
        {
            printf("Container %d is waiting to be filled\n",v->capacity);
            fflush(stdout);
            v->state = 0;
        }

        if(k==0)
            pthread_exit(NULL);

    }
    pthread_exit(NULL);
}

void biryani_ready(int index,int r,int p)
{
    while(r>0)
    {
        if(k==0)
            return;
        
        for(int i=0;i<n;i++)
        {
            pthread_mutex_lock(&vessels[i].lock4);

            if(!vessels[i].state && vessels[i].capacity<=0)
            {
                printf("Biryani is being filled on table number %d\n",i+1);
                fflush(stdout);
                vessels[i].capacity = p;
                r--;
                vessels[i].state=2;
            }

            if(!r)
            {
                printf("RobotChef %d has started cooking again\n",index);
                fflush(stdout);
                pthread_mutex_unlock(&vessels[i].lock4);
                return;
            }

            if(k<=0)
            {
                pthread_mutex_unlock(&vessels[i].lock4);
                return;
            }
            pthread_mutex_unlock(&vessels[i].lock4);
        }
    }
}

void* prepare_biryani(void *arg)
{
    while(1)
    {
        if(k==0)
            return NULL;

        struct robIdx *vals = (struct robIdx*) arg;
        int idx = vals->ind;
        int w = ((rand()%4)+2);
        int r = ((rand()%10)+1);
        int p = ((rand()%26)+25);
        
        printf("RobotChef %d is preparing %d biryani vessels\n",idx,r);
        fflush(stdout);

        sleep(w);
        printf("RobotChef %d took %d sec to prepare %d biryani vessels each of which will serve %d students\n",idx,w,r,p);
        fflush(stdout);
        biryani_ready(idx,r,p);
    
    }
    pthread_exit(NULL);
}

int main()
{
    printf("Number of Tables in mess:- ");
    scanf("%d",&n);
    printf("Number of Chefs in mess:- ");
    scanf("%d",&m);
    printf("Number of students arriving in mess:- ");
    scanf("%d",&k);

    int temp=k;
    pthread_t robotChefs[m];
    pthread_t servingTables[n];
    pthread_t student[k];

    printf("Mess is open now!!!\n");
    fflush(stdout);

    vessels = (vessel *)malloc(sizeof(vessel)*(n+1));

    for(int i=0;i<m;i++)
        robotIndex[i].ind=i+1;

    for(int i=0;i<n;i++)
    {
        vessels[i].ind=i+1;
        vessels[i].state=0;
        vessels[i].slots=0;
        vessels[i].capacity=0;
        pthread_mutex_init(&vessels[i].lock4,NULL);
    }

    for(int i=0;i<k;i++)
    {
        stud[i].state=0;
        stud[i].student_num=i+1;
    }
    
    for(int i=0;i<m;i++)
        pthread_create(&robotChefs[i],NULL,prepare_biryani,&robotIndex[i]);

    for(int i=0;i<n;i++)
        pthread_create(&servingTables[i],NULL,serve,&vessels[i]);

    srand(time(0));
    for(int i=0;i<temp;i++)
    {
        int random_no = rand()%4;
        sleep(random_no);
        pthread_create(&student[i],NULL,student_in_slot,&stud[i]);
    }

    for(int i=0;i<temp;i++)
        pthread_join(student[i],NULL);

    printf("Mess is now closed!!!\n");
    exit(0);
}
