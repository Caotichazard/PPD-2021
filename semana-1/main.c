/* Produto Interno Escalar de dois vetores */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h> 
#include <time.h>

#define N 1<<19

double A[N];
double B[N];
// double C[N][ 64/sizeof(double) ];
// double C[N][1024];
// double C[N][64];
double C[N][2];
	
int
main()
{
  long int i;
  double sum;

  struct timespec begin, end; 
  long seconds, nanoseconds;
  double elapsed;

  srand(time(NULL));

  // iniciacao dos vetores com valores aleatorios
  for(i=0; i < N; i++) {
    A[i]=(double)rand() / (double)RAND_MAX;
    B[i]=(double)rand() / (double)RAND_MAX;
    C[i][0]=B[i];
  }

  // Inicia medida de tempo
  clock_gettime(CLOCK_REALTIME, &begin);
    
  // calculo do produto vetorial
  sum = 0.0;

  // Caso 1: trabalha com 2 vetores de elementos salvos sequencialmente na memória
  for (i=0; i < N; i++)
    sum += A[i] * B[i];

  // Termina medida de tempo
  clock_gettime(CLOCK_REALTIME, &end);
  seconds = end.tv_sec - begin.tv_sec;
  nanoseconds = end.tv_nsec - begin.tv_nsec;
  elapsed = seconds + nanoseconds*1e-9;
    
  printf("Produto interno: %f\n",sum);
  printf("Tempo gasto: %.3f seconds.\n", elapsed);

  // Inicia medida de tempo
  clock_gettime(CLOCK_REALTIME, &begin);
    
  // calculo do produto vetorial
  sum = 0.0;
	
  // Caso 2: trabalha com os elementos de um vetor e com os elementos da 1a coluna de uma matriz
  // Dados da 1a coluna vão estar em posições não contíguas na memória
  for (i=0; i < N; i++)
    sum += A[i] * C[i][0];

  // Termina medida de tempo
  clock_gettime(CLOCK_REALTIME, &end);
  seconds = end.tv_sec - begin.tv_sec;
  nanoseconds = end.tv_nsec - begin.tv_nsec;
  elapsed = seconds + nanoseconds*1e-9;
    
  printf("Produto interno: %f\n",sum);
  printf("Tempo gasto: %.3f seconds.\n", elapsed);

 return 0;
}