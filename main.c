/*
 * main.c
 *
 *  Created on: Nov 20, 2011
 *      Authors: Andre, Juan, Thaylo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int count=0,inside=0,bottleneck,fail=0,crossing=0;
pthread_mutex_t mutext,matagal;
pthread_cond_t line,boat,cross_start,cross_return;

void *passenger(void *id) {
	int tid = *(int *)id;
	printf("Jovem %d vai tentar pegar o barco!\n",tid);
	/*
	 * Primeira condicao de espera
	 */
	pthread_mutex_lock(&mutext);
		// Variavel count verifica o numero de jovens ja no esquema, caso seja maior ou igual a 4, o jovem em questao
		// deve ir pra fila
		count++;
		if(count >= 4) {
			printf("Jovem %d entrou na fila, existem %d jovem(ns) na sua frente\n",tid,count-4);
			pthread_cond_wait(&line,&mutext);
		}
		// Depois de ser liberado da fila, a variavel inside coordena quantos jovens estao no barco
		inside++;
		printf("Jovem %d entrou no barco e espera que este saia\n",tid);
		// caso o barco esteja cheio, sinaliza para o barqueiro comecar a travessia e espera o sinal de que a travessia terminou
		if(inside == 3) {
			pthread_cond_signal(&cross_start);
		}
		pthread_cond_wait(&boat,&mutext);
		printf("Jovem %d sai com o barco.\n",tid);
	pthread_mutex_unlock(&mutext);


	/*
	 * Estagio do matagal
	 */
	pthread_mutex_lock(&matagal);
		printf("Jovem %d tentando atravessar o matagal...\n",tid);
		sleep(1);
		// Checa se ele e' o que sera pego
		if(tid != bottleneck) {
			//checa se alguem ja foi pego
			if(fail) printf("Opa! Os seguranças pegaram um ... não vou poder entrar no show... o jeito vai ser o telão mesmo!\n");
			else printf("Jovem %d entrou no show!\n",tid);
		}
		// Se alguem foi pego
		else {
			fail = 1;
			printf("Jovem %d preso na cerca... Shii! Os segurancas me pegaram!\n",tid);
		}
		crossing++;
		// Sinaliza para o barqueiro retornar
		if(crossing==3) {
			pthread_cond_signal(&cross_return);
			crossing=0;
		}
	pthread_mutex_unlock(&matagal);
}

void *boatman() {
	char msg[LEN];

	while(1) {
		pthread_mutex_lock(&mutext);
			// verifica se o barco esta cheio. Senao, espera encher.
			if(inside!=3) pthread_cond_wait(&cross_start,&mutext);
			printf("Barqueiro recolhe ajuda para a comunidade e sai com o barco...\n");
			sleep(3);
			printf("Barqueiro chega na outra margem e espera todos atravessarem o matagal...\n");
			// manda o sinal para os processos no barco
			pthread_cond_broadcast(&boat);
		pthread_mutex_unlock(&mutext);

		pthread_mutex_lock(&matagal);
			// Espera a sinalizacao dos jovens atravessarem
			pthread_cond_wait(&cross_return,&matagal);
			// Caso haja alguem agarrado, fim do esquema
			if (fail){
				printf("Opa! Problemas! Os segurancas pegaram um! Acabou meu esquema...\n");
				// Criacao do FIFO
				mkfifo(FNAME,0660);
				int id,fd;
				// Cria um filho para realizar a leitura e evitar que este processo fique bloqueado eternamente
				id = fork();
				if ( id < 0) {
					printf("Erro na criacao do filho...Abortando...\n");
					exit(1);
				}else if (id == 0){
					do {
						fd=open(FNAME,O_WRONLY);
						if (fd==-1) sleep(1);
					}while (fd==-1);

					sprintf(msg,"O caminho mais curto nem sempre é o mais direito...\n");
					write(fd,msg,strlen(msg)+1);
					close(fd);

					exit(1);
				}else{
					// Barqueiro se mata
					raise(SIGINT);
				}
			}


			else printf("Tudo certo! Barqueiro retorna para fazer outra viagem...\n");
			sleep(2);
			inside=0;
			// Libera 3 jovens da fila para o barco
			for(int i=0; i<3;i++) pthread_cond_signal(&line);
		pthread_mutex_unlock(&matagal);

		pthread_mutex_lock(&mutext);
			// diminui o total de jovens
			count-=3;
		pthread_mutex_unlock(&mutext);
	}

}


int main() {

	pthread_mutex_init(&mutext,NULL);
	pthread_cond_init(&line,NULL);
	pthread_cond_init(&boat,NULL);
	pthread_cond_init(&cross_start,NULL);
	pthread_cond_init(&cross_return,NULL);

	// gera id do jovem que vai ficar agarrado
	srand(time(NULL));
	bottleneck = rand()%15;
	// Geracao dos ids
	long *taskid[MAX_PASSENGERS];
	for(int i=0;i<MAX_PASSENGERS;i++) {
		taskid[i] = (long *) malloc(sizeof(long));
		*taskid[i] = i;
	}


	pthread_t carrier,passengers[MAX_PASSENGERS];

	// Criacao das threads
	pthread_create(&carrier,NULL,boatman,NULL);
	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_create(&passengers[i],NULL,passenger,taskid[i]);
	}

	// Espera pelas threads
	for(int i=0;i<MAX_PASSENGERS;i++) {
		pthread_join(passengers[i],NULL);
	}
	pthread_join(carrier,NULL);

	return 0;
}
