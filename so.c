#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int database;
int reading_count = 0;
int new_write = 0;
pthread_mutex_t mutexRC; //reading_count
pthread_mutex_t mutexR; //Reader
pthread_mutex_t mutexW; //Writer
pthread_mutex_t mutexSV; //database

void* reader(void *number){
  printf("Leitura %ld iniciada\n", (long)number);
  if (new_write == 1){               /* Verificar se existe pedido de escrita */
    printf("Leitura %ld esperando escrita\n", (long)number);
    pthread_mutex_lock(&mutexR);    /* Esperar a escrita */
    printf("Prosseguindo leitura na thread %ld\n", (long)number);
  }

  pthread_mutex_lock(&mutexRC);     /* bloquear acesso ao contador */
  reading_count++;                  /* incrementar contador */
  printf("Leitura %ld modificando Reading Count para %d\n", (long)number, reading_count);
  if (reading_count == 1) {          /* Se for a primeira leitura */
    pthread_mutex_lock (&mutexSV);  /* Acessando base */
    printf("Leitura %ld bloqueando a base\n", (long)number);
  }
  pthread_mutex_unlock(&mutexRC);   /* liberar acesso ao contador */

  printf("Leitura %ld lendo base com valor %d\n",
      (long)number, database);    /* Efetuar leitura */
  sleep((rand()%10)/10);
  pthread_mutex_lock(&mutexRC);     /* bloquear acesso ao contador */
  reading_count--;                  /* decrementar contador */
  printf("Leitura %ld modificando Reading Count para %d\n", (long)number, reading_count);
  if(reading_count == 0)
    pthread_mutex_unlock(&mutexSV); /* Liberando a base */
    printf("Leitura %ld liberando a base\n", (long)number);
  pthread_mutex_unlock(&mutexRC);   /* liberar acesso ao contador */
}

void* writer(void *number){
  printf("Escrita %ld iniciada\n", (long)number);
  pthread_mutex_lock(&mutexW);      /* Bloquear escritas */
  printf("Escrita %ld bloqueando leituras\n", (long)number);
  new_write++;                      /* Bloquear novas leituras. */
  printf("Escrita %ld aguadando fim de leituras\n", (long)number);
  pthread_mutex_lock(&mutexSV);     /* Esperar acesso a base / as leituras terminarem */
  printf("Escrita %ld alterando dados\n", (long)number);
  database = rand()%10;           /* Efetuar escrita */
  sleep((rand()%15)/10);
  pthread_mutex_unlock(&mutexSV);   /* Liberar acesso a base */
  printf("Escrita %ld liberando novas operacoes\n", (long)number);
  new_write--;                      /* Liberar novas leituras */
  pthread_mutex_unlock(&mutexR);    /* Sinalizar leituras em espera */
  pthread_mutex_unlock(&mutexW);    /* Liberar escrita */
  printf("Escrita %ld finalizada\n", (long)number);
}

int create_joinable_thread(pthread_t *thread, long number, int random){
  pthread_attr_t attr;


  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  if(random)
    return pthread_create(thread, &attr, reader, (void *)number);
  else
    return pthread_create(thread, &attr, writer, (void *)number);

}

int main(){
  int size;
  size = 50;
  pthread_t thread[size];

  int error;
  long i;

/*
  create_joinable_thread(&thread[0], 0, 0);
  create_joinable_thread(&thread[1], 1, 0);
  create_joinable_thread(&thread[2], 2, 1);
  create_joinable_thread(&thread[3], 3, 1);
  create_joinable_thread(&thread[4], 4, 0);
*/

  for(i = 0; i < size/2; i++){
    error = create_joinable_thread(&thread[i], i, 0);
    if(error) {printf("ERROR CRIACAO ESCRITOR"); return -1;}
  }

	for(i = size/2 + 1; i < size; i++){
    error = create_joinable_thread(&thread[i], i, 1);
    if(error) {printf("ERROR CRIACAO LEITOR"); return -1;}
  }


  for(i = 0; i < size; i++){
    error = pthread_join(thread[i], NULL);
    if(error) {printf("ERROR JOIN"); return -1;}
  }

  return 0;
}
