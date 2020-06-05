#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<stdlib.h>
#include<stdio.h>

/*fixed buffer size*/
#define BUFFER_SIZE 5
/*buffer item type*/
typedef int buffer_item_t;

/*init semaphore*/
pthread_mutex_t Mutex;

sem_t Full,Empty;
size_t head,tail;
/*bool to indicate whether buffer is full*/
bool full;

/*build buffer, thich is a circular array*/
buffer_item_t buffer[BUFFER_SIZE];

void showBuffer(){
	printf("buffer: ");
	if((head!=tail)||full){
		int cnt=head;
		do{
			printf("%d ",buffer[cnt]);
			cnt=(cnt+1)%BUFFER_SIZE;
		}while(cnt!=tail);
	}else{
		printf("empty");
	}
	printf("\nnext round: \n");
}

int insert_item(buffer_item_t item){
	int condition=-1;

	sem_wait(&Empty);
	pthread_mutex_lock(&Mutex);
	if(!full){/*buffer not full*/
		buffer[tail]=item;
		tail=(tail+1)%BUFFER_SIZE;
		if(tail==head){
			full=true;
		}
		condition=0;
		printf("producer produced: %d\n",item);
		showBuffer();
	}
	pthread_mutex_unlock(&Mutex);
	sem_post(&Full);
}

int remove_item(buffer_item_t *item){
	int condition=-1;

	sem_wait(&Full); /*wait if no block was filled*/
	pthread_mutex_lock(&Mutex);
	if((head!=tail)||full){ /*there is something to consume*/
		*item=buffer[head];
		head=(head+1)%BUFFER_SIZE;
		full=false;
		condition=0;
		printf("consumer consumed: %d\n",*item);
		showBuffer();
	}
	pthread_mutex_unlock(&Mutex);
	sem_post(&Empty);
	return condition;
}

void* producer(void* param);
void* consumer(void* param);

int main(int argc, char**argv){
	if(argc!=4){
		return -1;
	}
	srand((unsigned int)time(NULL));
	/* 1. get args */
	unsigned long SleepTime=atol(argv[1]);
	unsigned producer_thread_num=atoi(argv[2]);
	unsigned consumer_thread_num=atoi(argv[3]);
	printf("%ld, %d, %d\n",SleepTime,producer_thread_num,consumer_thread_num);
	/* 2. init buffer */
	head=0;tail=0;
	full=false;	
	pthread_mutex_init(&Mutex,NULL);
	sem_init(&Full,0,0);
	sem_init(&Empty,0,BUFFER_SIZE);
	/* 3. Create producer threads and consumer threads */
	pthread_attr_t my_attr;
	pthread_attr_init(&my_attr);
	for(unsigned i=0;i<producer_thread_num;++i){
		pthread_t producer_thread;
		pthread_create(&producer_thread,&my_attr,producer,NULL);
	}
	for(unsigned i=0;i<consumer_thread_num;++i){
		pthread_t consumer_thread;
                pthread_create(&consumer_thread,&my_attr,consumer,NULL);
	}

	/* 4. Sleep */
	sleep((float)(SleepTime/1000));

	/* Exit */
	return 0;
}

void* producer(void* param){
	buffer_item_t my_rand;
	/*sleep for a random time*/
	while(true){
		sleep((float)((rand()%1000)/1000));
		my_rand=rand()%20;
		if(insert_item(my_rand)){
			printf("report error condition.\n");
		}
	}
}

void* consumer(void* param){
	buffer_item_t my_rand;
	/*sleep for a random time*/
	while(true){
		sleep((float)((rand()%1000)/1000));
		if(remove_item(&my_rand)){
			printf("report error condition.\n");
		}
	}
}

