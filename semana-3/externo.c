/*
** PPD / DC/UFSCar - Helio
** Programa : multiplicacao de matrizes
** Objetivo: paralelizacao om OpenMP
*/
/*
  Aluno: Guilherme Locca Salomão
  RA: 758569
*/

#include <math.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <omp.h>

float *A, *B, *C;

int
main(int argc, char *argv[])
{
  struct timespec begin, end;
	long seconds, nanoseconds;
  double elapsed; 

  int lin_a,col_a,lin_b,col_b,lin_c,col_c;
  int i,j,k;
  int numThread;

  printf("");   scanf("%d",&lin_a);
  printf(""); scanf("%d",&col_a);
  lin_b=col_a;
  printf("");  scanf("%d",&col_b);
  printf("");  scanf("%d",&numThread);
  printf("\n");
  lin_c=lin_a;
  col_c=col_b;

  // Alocacao dinamica das matrizes, com linhas em sequencia 
  A = (float *)malloc(lin_a*col_a*sizeof(float));
  B = (float *)malloc(lin_b*col_b*sizeof(float));
  C = (float *)malloc(lin_c*col_c*sizeof(float));

  // Atribucao de valores iniciais as matrizes 
  srandom(time(NULL));

  for(i=0; i < lin_a * col_a; i++) 
    A[i]=(float)rand() / (float)RAND_MAX; 

  for(i=0; i < lin_b * col_b; i++) 
    B[i]=(float)rand() / (float)RAND_MAX; 

  // calculo da multiplicacao
 
  // Qual/quais loop(s) paralelizar? Vale a pena paralelizar todos?
  /*
    Testando a paralelização de cada loop, foi notavel que apenas a paralelização do loop intermediario
    teve sucesso em reduzir o tempo, com os outros loops não obtive alteração e em certos casos
    aumento de tempo gasto no calculo
  */
  // Qual é o efeito de fazer um parallel for em cada um dos fors abaixo?
  /*
    Utilizar um parallel for em cada um deles causa um aumento de tempo no calculo de cada loop
    Isso deve ocorrer pela falta de compartilharmento correto das variaves e falta de threads
  */

  // É necessários sincronizar alguma operação, garantindo exclusão mútua?
  /*
    Não há necessidade de sincronização de operações já que não há dependencia entre as operações
  */

  clock_gettime(CLOCK_REALTIME, &begin);

  omp_set_num_threads(numThread);
    #pragma omp parallel
        {
        #pragma omp for private(j,k)
    for(i=0; i < lin_c; i++){ 
      
      for(j=0; j < col_c; j++) {
        C[i*col_c+j]=0;
        
          for(k=0; k < col_a; k++) {
            //printf("Thread %d tratando iteração %d\n", omp_get_thread_num(), i);
            C[i*col_c+j] = C[i*col_c+j] + A[i*col_a+k] * B[k*col_b+j];
          }
        }
    }

  }

  clock_gettime(CLOCK_REALTIME, &end);
  seconds = end.tv_sec - begin.tv_sec;
  nanoseconds = end.tv_nsec - begin.tv_nsec;
  elapsed = seconds + nanoseconds*1e-9;
  printf("Tempo gasto: %.3f seconds.\n", elapsed);
  return(0);
}