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

sem_t mutex,boat,cross_start,cross_end,mutex2,in,mutex3;
int count=0,inside=0,fatboy,if_fuck=0;

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
		printf("Jovem %d atravesssando matagal!\n",tid);
		if (if_fuck){
			printf("Opa! Os segurancas pegaram um ... nao vou poder entrar no show... o jeito vai ser  o telao mesmo!\n");
			sem_post(&mutex2);
			pthread_exit(NULL);
		}
		sem_wait(&in);
			if(tid == fatboy) {
				printf("Jovem %d preso na cerca... Shii!! Os segurancas me pegaram!\n",tid);
				sem_wait(&mutex3);
					if_fuck = 1;
				sem_post(&mutex3);
				sem_post(&mutex2);
				//exit(1);
			}else inside++;
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
		sem_wait(&mutex3);
		printf("IF FUCK: %d\n",if_fuck);
		if (if_fuck){
			printf("Opa! Problemas! Os segurancas pegaram um! Acabou meu esquema...\n");

			//TODO: Codigo do named pipe!

			exit(1);
		}
		sem_post(&mutex3);
		sleep(2);
		sem_wait(&mutex);
			count-=3;
		sem_post(&mutex);
		printf("Tudo certo! Barqueiro retorna para fazer outra viagem...\n");
		for(int i=0;i<3;i++) sem_post(&boat);
	}
	//return;
}


int main() {


	sem_init(&mutex,0,1);
	sem_init(&mutex2,0,0);
	sem_init(&cross_start,0,0);
	sem_init(&cross_end,0,0);
	sem_init(&boat,0,3);
	sem_init(&in,0,1);
	sem_init(&mutex3,0,1);


	srand(time(NULL));
	fatboy = rand()%15;
	printf("OLHA O GORDO: %d\n",fatboy);
	sleep(2);
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

	return 0;
}

