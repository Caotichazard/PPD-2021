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

  printf("");   scanf("%d",&lin_a);
  printf(""); scanf("%d",&col_a);
  lin_b=col_a;
  printf("");  scanf("%d",&col_b);
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
    Apos testar a paralelização separadamente em cada loop, foi notável que a melhor opção para se 
    paralelizar foi o loop intermediário, seguido pelo loop externo. O loop interno mostrou ter um 
    aumento considerável no tempo de execução do codigo. Tendo isso, pode ser viável a paralelização
    de apenas os loops externos e intermediários, porém o ganho disso poderia ser mínimo
  */
  // Qual é o efeito de fazer um parallel for em cada um dos fors abaixo?
  /*
    Ao utilizar um parallel for nos fors abaixo, foi notável que a diferença de redução de tempo
    de execução entre eles era pequena, os resultados foram mais expressivos ao utilizar a declaração
    parallel e depois declarar o omp for
  */

  // É necessários sincronizar alguma operação, garantindo exclusão mútua?
  /*
    Não há necessidade de sincronizar operações já que, neste caso, o calculo de cada célula da matriz
    é realizado independente dos outros
  */
  /*
    Além disso, foram realizados testes utilizando a declaração omp parallel e seguida por 
    omp for em cada loop separadamente (resultados dos testes no fim do codigo).

  */
  clock_gettime(CLOCK_REALTIME, &begin);

  
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

/*

os testes foram realizados apenas com matrizes quadradas dos tamanhos
512x512 * 512x512
1024x1024 * 1024x1024
2048x2048 * 2048x2048


primeiro foi anotado os tempos de uma execução do codigo sem o uso de OpenMP

BASE 
512 - 0.961
1024 - 22.882
2048 - 318.178

então foi definido um arquivo para cada loop paralelizado
externo.o para o loop mais externo
intermediario.o para o loop intermediario
interno.o para o loop interno

segue a declaração de cada loop

externo.o
{
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
}

intermediario.o
{
  omp_set_num_threads(numThread);
    for(i=0; i < lin_c; i++){ 
      #pragma omp parallel shared(i)
        {
        #pragma omp for private(k)
      for(j=0; j < col_c; j++) {
        C[i*col_c+j]=0;
        
          for(k=0; k < col_a; k++) {
            //printf("Thread %d tratando iteração %d\n", omp_get_thread_num(), i);
            C[i*col_c+j] = C[i*col_c+j] + A[i*col_a+k] * B[k*col_b+j];
          }
        }
    }

  }
}

interno.o
{
  omp_set_num_threads(numThread);
    for(i=0; i < lin_c; i++){ 
      
      for(j=0; j < col_c; j++) {
        C[i*col_c+j]=0;
            #pragma omp parallel shared(i,j)
        {
        #pragma omp for 
          for(k=0; k < col_a; k++) {
            //printf("Thread %d tratando iteração %d\n", omp_get_thread_num(), i);
            C[i*col_c+j] = C[i*col_c+j] + A[i*col_a+k] * B[k*col_b+j];
          }
        }
    }

  }
}


foi utilizado o metodo
omp_set_num_threads(numThread);
para atribuir a quantidade de threads a serem usadas, para isso foi adicionado um valor de input 
para indicar quantas threads iriam ser usadas na execução

então foram executados os codigos respectivos a cada loop paralelizado, para as 3 matrizes diferentes
para os valores de threads de 1,2,4,8,16,32,64,128

com esses resultados foi possível notar que:
  - há uma pequena diferença entre a paralelização do loop externo e intermediário, sendo a melhor opção
  o loop intermediário, o qual mostrou o maior ganho de tempo

  - a paralelização do loop interior mostrou ser a pior opção a partir do uso de 8 ou mais threads
  onde houve um aumento no tempo de execução do código

  - os melhores tempos foram atingidos com a parelelização do loop intermediario e com 8 threads

  

ARQUIVO externo.o
    NUM_ THREADS = 1
    512 - 1.099 seconds.
    1024 - 29.394 seconds.
    2048 - 415.055 seconds.

    NUM_ THREADS = 2
    512 - 0.748 seconds.
    1024 - 11.411 seconds.
    2048 - 211.718 seconds.

    NUM_ THREADS = 4
    512 - 0.558 seconds.
    1024 - 7.666 seconds.
    2048 - 99.069 seconds.

    NUM_ THREADS = 8
    512 - 0.334 seconds.
    1024 - 5.244 seconds.
    2048 - 115.613 seconds.

    NUM_ THREADS = 16
    512 - 0.301 seconds.
    1024 - 6.600 seconds.
    2048 - 117.889 seconds.

    NUM_ THREADS = 32
    512 - 0.323 seconds.
    1024 - 6.081 seconds.
    2048 - 120.496 seconds.

    NUM_ THREADS = 64
    512 - 0.336 seconds.
    1024 - 5.082 seconds.
    2048 - 114.689 seconds.

    NUM_ THREADS = 128
    512 - 0.307 seconds.
    1024 - 5.747 seconds.
    2048 - 113.155 seconds.

ARQUIVO interno.o
    NUM_ THREADS = 1
    512 - 1.422 seconds.
    1024 - 31.675 seconds.
    2048 - 392.699 seconds.

    NUM_ THREADS = 2
    512 - 0.874 seconds.
    1024 - 17.315 seconds.
    2048 - 214.178 seconds.

    NUM_ THREADS = 4
    512 - 0.837 seconds.
    1024 - 7.421 seconds.
    2048 - 135.151 seconds.

    NUM_ THREADS = 8
    512 - 1.227 seconds.
    1024 - 10.210 seconds.
    2048 - 173.459 seconds.

    NUM_ THREADS = 16
    512 - 42.415 seconds.
    1024 - 179.550 seconds.
    2048 - 659.230 seconds.

    NUM_ THREADS = 32
    512 - 49.471 seconds.
    1024 - 200.451 seconds.
    2048 - 1481.362 seconds.

    NUM_ THREADS = 64
    512 -: 130.260 seconds.
    1024 - 449.726 seconds.
    2048 - 2287.444 seconds.

    NUM_ THREADS = 128
    512 - 185.830 seconds.
    1024 - 747.312 seconds.
    2048 - 3030.685 seconds.

ARQUIVO intermediario.o
    NUM_ THREADS = 1
    512 - 1.085 seconds.
    1024 - 13.793 seconds.
    2048 - 370.780 seconds.

    NUM_ THREADS = 2
    512 - 0.572 seconds.
    1024 - 8.090 seconds.
    2048 - 179.876 seconds.

    NUM_ THREADS = 4
    512 - 0.289 seconds.
    1024 - 3.438 seconds.
    2048 - 100.610 seconds.

    NUM_ THREADS = 8
    512 - 0.239 seconds.
    1024 - 3.036 seconds.
    2048 - 79.131 seconds.

    NUM_ THREADS = 16
    512 - 0.370 seconds.
    1024 - 4.099 seconds.
    2048 - 80.776 seconds.

    NUM_ THREADS = 32
    512 - 0.364 seconds.
    1024 - 3.862 seconds.
    2048 - 80.437 seconds.

    NUM_ THREADS = 64
    512 - 0.476 seconds.
    1024 - 3.586 seconds.
    2048 - 81.828 seconds.
    NUM_ THREADS = 128

    512 - 0.598 seconds.
    1024 - 4.532 seconds.
    2048 - 82.823 seconds.


*/