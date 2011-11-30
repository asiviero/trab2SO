/*
 * main.c
 *
 *  Created on: Nov 29, 2011
 *      Author: andre
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>

#define MAX_PASSENGERS 15
#define LEN 50
#define FNAME "FIFO"

//sem_t mutex,boat,cross_start,cross_end,mutex2,in,mutex3;
int count=0,inside=0,fatboy,if_fuck=0,crossing=0;
pthread_mutex_t mutext,matagal;
pthread_cond_t line,boat,cross_start,cross_return;

void *passenger(void *id) {
	int tid = *(int *)id;
	printf("Jovem %d vai tentar pegar o barco!\n",tid);
	pthread_mutex_lock(&mutext);
		count++;

		if(count >= 4) {
			printf("Jovem %d entrou na fila, existem %d jovem(ns) na sua frente\n",tid,count-4);
			pthread_cond_wait(&line,&mutext);
			//printf("passou1\n\n");
		}
		inside++;
		//count--;
		printf("Jovem %d entrou no barco e espera que este saia\n",tid);
		if(inside == 3) {
			printf("Barco cheio, vou sinalizar para o barqueiro\n");
			pthread_cond_signal(&cross_start);
			inside=0;
		}
		pthread_cond_wait(&boat,&mutext);
		printf("Jovem %d tentando atravessar o matagal aqui!\n",tid);
	pthread_mutex_unlock(&mutext);



	pthread_mutex_lock(&matagal);
		sleep(1);
		if(tid != fatboy) {
			if(if_fuck) printf("\nPegaram o gordo!\n");
			else printf("Passei!\n");
		}
		else {
			if_fuck = 1;
			printf("\n\n************************\n GORDO SO FAZ GORDICE PQP \n\n**********************\n");
		}
		crossing++;
		if(crossing==3) {
			printf("Todo mundo passou, barqueiro retornando...\n");
			pthread_cond_signal(&cross_return);
			crossing=0;
		}
	pthread_mutex_unlock(&matagal);
}

void *boatman() {
	char msg[LEN];

	while(1) {
		pthread_mutex_lock(&mutext);
			printf("Ahoy there sailors! Barqueiro esperando o barco encher!\n");
			pthread_cond_wait(&cross_start,&mutext);
			printf("Travessia vai comecar...");
			sleep(3);
			pthread_cond_broadcast(&boat);
		pthread_mutex_unlock(&mutext);

		pthread_mutex_lock(&matagal);
			printf("Esperando o pessoal atravessar...\n");
			pthread_cond_wait(&cross_return,&matagal);
			if (if_fuck){
				printf("Opa! Problemas! Os segurancas pegaram um! Acabou meu esquema...\n");
				mkfifo(FNAME,0660);
				int id,fd;
				id = fork();
				if ( id < 0) {
					printf("Erro na criacao do filho...Abortando...\n");
					exit(1);
				}else if (id == 0){
					do {
						fd=open(FNAME,O_WRONLY);
						if (fd==-1) sleep(1);
					}while (fd==-1);

					sprintf(msg,"O caminho mais curto nem sempre eh o mais direito...\n");
					write(fd,msg,strlen(msg)+1);
					close(fd);

					exit(1);
				}else{
					raise(SIGINT);
				}
			}



			/*if(if_fuck) {

				printf("Pqp esse gordo fudeu o bagulho!\n\n"); exit(1);


			}*/
			else printf("Todos atravessaram, barqueiro retornando...\n");
			sleep(2);
			for(int i=0; i<3;i++) pthread_cond_signal(&line);
		pthread_mutex_unlock(&matagal);

		pthread_mutex_lock(&mutext);
			count-=3;
		pthread_mutex_unlock(&mutext);
		//printf("Count: %d\n",count);

		/*sem_wait(&cross_start);
		printf("Barqueiro recolhe ajuda para a comunidade e sai com o barco...\n");
		sleep(3);
		printf("Barqueiro chega na outra margem e libera os 3 passageiros...\n");
		for(int i=0;i<3;i++) sem_post(&cross_end);
		printf("Barqueiro espera todos atravessarem o matagal...\n");
		sem_wait(&mutex2);
		sem_wait(&mutex3);

		if (if_fuck){
			printf("Opa! Problemas! Os segurancas pegaram um! Acabou meu esquema...\n");
			mkfifo(FNAME,0660);
			int id,fd;

			id = fork();
			if ( id < 0) {
				printf("Erro na criacao do filho...Abortando...\n");
				exit(1);
			}else if (id == 0){
				do {
					fd=open(FNAME,O_WRONLY);
					if (fd==-1) sleep(1);
				}while (fd==-1);

				sprintf(msg,"O caminho mais curto nem sempre eh o mais direito...\n");
				write(fd,msg,strlen(msg)+1);
				close(fd);

				exit(1);
			}else{
				raise(SIGINT);
			}
		}
		sem_post(&mutex3);
		sleep(2);
		sem_wait(&mutex);
		count-=3;
		sem_post(&mutex);
		printf("Tudo certo! Barqueiro retorna para fazer outra viagem...\n");
		for(int i=0;i<3;i++) sem_post(&boat);*/
	}
	//return;
}


int main() {

	pthread_mutex_init(&mutext,NULL);
	pthread_cond_init(&line,NULL);
	pthread_cond_init(&boat,NULL);
	pthread_cond_init(&cross_start,NULL);
	pthread_cond_init(&cross_return,NULL);
	/*sem_init(&mutex,0,1);
	sem_init(&mutex2,0,0);
	sem_init(&cross_start,0,0);
	sem_init(&cross_end,0,0);
	sem_init(&boat,0,3);
	sem_init(&in,0,1);
	sem_init(&mutex3,0,1);
	*/

	srand(time(NULL));
	fatboy = rand()%15;
	printf("\n\n ************************\n GORDO SO FAZ GORDICE PQP\n O GORDO E: %d \n\n**********************\n",fatboy);
	long *taskid[MAX_PASSENGERS];

	for(int i=0;i<MAX_PASSENGERS;i++) {
		taskid[i] = (long *) malloc(sizeof(long));
		*taskid[i] = i;
	}

	pthread_t carrier,passengers[MAX_PASSENGERS];
	pthread_create(&carrier,NULL,boatman,NULL);
	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_create(&passengers[i],NULL,passenger,taskid[i]);
	}

	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_join(passengers[i],NULL);
	}
	pthread_join(carrier,NULL);

	return 0;
}
