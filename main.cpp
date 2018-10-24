#pragma comment(lib, "pthreadVC2.lib")
#include <iostream>
#include <stdio.h>
#include <map>
#include <vector>
#include <pthread.h>
#include "var.h"
#include "functions.h"
#include <cstdio>
using namespace std;

#define NUM_THREADS 5

int main()
{
	freopen(".\\data.txt", "r", stdin);
	pthread_t pthreads[NUM_THREADS];	// create 5 threads
	send_router();
	// initialize the system
	initial();
	print_router();
	// Stable Processing
	send_router();		// send to neighbors

	pthread_create(&pthreads[0], NULL, count_upgrade_time, NULL);	// periodically send upgrade
	pthread_create(&pthreads[1], NULL, count_neighbor_time, NULL);	// check if neighbor dead
	pthread_create(&pthreads[2], NULL, listen, NULL);				// start listening
	pthread_create(&pthreads[3], NULL, send_message, NULL);			// automatically sending test message
	
	pthread_join(pthreads[0], NULL);
	pthread_join(pthreads[1], NULL);
	pthread_join(pthreads[2], NULL);
	pthread_join(pthreads[3], NULL);
	return 0;
}