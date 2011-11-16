/*
 * main.c
 *
 *  Created on: Nov 16, 2011
 *      Author: andre
 */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#define MAX_PASSENGERS 15

sem_t mutex,boat,cross_start,cross_end,mutex2,in;
int count=0,inside=0,fatboy;

void *passenger(void *id) {
	int tid = *(int *)id;
	printf("Jovem %d vai tentar pegar o barco!\n",tid);

	sem_wait(&mutex);
		if(count>=3) printf("Jovem %d entrou na fila, existem %d jovem(ns) na sua frente\n",tid,count-3);
		count++;
	sem_post(&mutex);

	sem_wait(&boat);
		printf("Jovem %d entrou no barco e espera que este saia\n",tid);
		sem_wait(&mutex);
			if(count>=3) sem_post(&cross_start);
		sem_post(&mutex);
	sem_wait(&cross_end);
		sleep(1);
		sem_wait(&in);
			inside++;
			if(tid == fatboy) {printf("GORDO SO FAZ GORDICE\n"); exit(1);}
			if(inside==3) {
				inside = 0;
				sem_post(&mutex2);
			}
		sem_post(&in);
		//return;
}

void *boatman() {
	while(1) {
		printf("Count: %d\n",count);
		sem_wait(&cross_start);
		printf("Barqueiro recolhe ajuda para a comunidade e sai com o barco...\n");
		sleep(3);
		printf("Barqueiro chega na outra margem e libera os 3 passageiros...\n");
		for(int i=0;i<3;i++) sem_post(&cross_end);
		printf("Barqueiro espera todos atravessarem o matagal...\n");
		sem_wait(&mutex2);
		sleep(2);
		sem_wait(&mutex);
			count-=3;
		sem_post(&mutex);
		for(int i=0;i<3;i++) sem_post(&boat);
	}
	//return;
}


int main() {
	sem_init(&mutex,0,1);
	sem_init(&mutex2,0,1);
	sem_init(&cross_start,0,0);
	sem_init(&cross_end,0,0);
	sem_init(&boat,0,3);
	sem_init(&in,0,1);
	srand(time(NULL));
	fatboy = rand()%15;
	printf("OLHA O GORDO: %d\n",fatboy);
	sleep(5);
	long *taskid[MAX_PASSENGERS];
	for(int i=0;i<MAX_PASSENGERS;i++) {
		taskid[i] = (long *) malloc(sizeof(long));
		*taskid[i] = i;
	}

	pthread_t carrier,passengers[MAX_PASSENGERS];

	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_create(&passengers[i],NULL,passenger,taskid[i]);
	}
	pthread_create(&carrier,NULL,boatman,NULL);
	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_join(passengers[i],NULL);
	}
	pthread_join(carrier,NULL);
}

