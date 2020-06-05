#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>

#define BUFFER_SIZE 5
/*buffer item type*/
typedef int buffer_item_t;  
/*singnals*/
/*
* Empty: empty "blocks" in buffer
* Full: filled "blocks" in buffer
*/
HANDLE Empty, Full, Mutex;
size_t head, tail;
bool full;

/*build buffer, which is acircular array*/
buffer_item_t buffer[BUFFER_SIZE];  /*the buffer*/

void showBuffer() {
	printf("buffer: ");
	if ((head != tail) || full) {
		int cnt = head;
		do{
			printf("%d  ", buffer[cnt]);
			cnt = (cnt + 1) % BUFFER_SIZE;
		} while (cnt != tail);
	}
	else {
		printf("empty!");
	}
	printf("\nnext round:\n");
}

int insert_item(buffer_item_t item) {  //for creaters
	int condition = -1;

	WaitForSingleObject(Empty, INFINITE); 
	WaitForSingleObject(Mutex, INFINITE);
	if (!full) {  //buffer not full
		buffer[tail] = item;
		tail = (tail + 1) % BUFFER_SIZE;
		if (tail == head) {  
			full = true;
		}
		condition = 0; //produce successful
		printf("producer produced %d\n", item);
		showBuffer();
	}
	ReleaseMutex(Mutex);
	ReleaseSemaphore(Full, 1, NULL);
	return condition;
}

int remove_item(buffer_item_t* item) {
	int condition = -1;
	WaitForSingleObject(Full, INFINITE);
	WaitForSingleObject(Mutex, INFINITE);
	if ((head != tail) || full) {
		*item = buffer[head];
		head = (head + 1) % BUFFER_SIZE;
		full = false;
		condition = 0;
		printf("consumer consumed %d\n", *item);
		showBuffer();
	}
	ReleaseMutex(Mutex);
	ReleaseSemaphore(Empty, 1, NULL);
	return condition;
}

DWORD WINAPI producer(void* param);
DWORD WINAPI consumer(void* param);

int main(int argc, char** argv) {
	if (argc != 4) {
		return -1;  //error 
	}
	srand((unsigned int)time(NULL));
	/* 1. get args */
	unsigned long SleepTime = atoi(argv[1]);
	unsigned producer_thread_num = atoi(argv[2]);
	unsigned consumer_thread_num = atoi(argv[3]);
	/* 2. init buffer */
	Mutex = CreateMutex(NULL, FALSE, NULL);
	Full = CreateSemaphore(NULL, 0, 5, NULL);
	Empty = CreateSemaphore(NULL, 5, 5, NULL);
	head = 0; tail = 0;
	full = false;
	/* 3. Create producer threads and consumer threads */
	for (int i = 0; i < producer_thread_num; ++i) {
		CreateThread(NULL, 0, producer, NULL, 0, NULL);
	}
	for (int i = 0; i < consumer_thread_num; ++i) {
		CreateThread(NULL, 0, consumer, NULL, 0, NULL);
	}

	/* 4. Sleep */
	Sleep(SleepTime);

	/* 5. Exit */
	return 0;
}

DWORD WINAPI producer(void* param) {
	buffer_item_t my_rand;
	/*sleep for a random time*/
	while (TRUE) {
		Sleep(rand() % 1000);
		my_rand = rand() % 20;
		if (insert_item(my_rand)) {
			printf("report error condition");
		}
	}
}

DWORD WINAPI consumer(void* param) {
	buffer_item_t my_rand;
	/*sleep for a random time*/
	while (TRUE) {
		Sleep(rand() % 1000);
		if (remove_item(&my_rand)) {
			printf("report error condition");
		}
	}
}
