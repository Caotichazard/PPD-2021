//ALUNO: GUILHERME LOCCA SALOMÂO
//RA: 758569

/*
	A estratégia de paralelização foi a de computação embaraçosamente paralela,
	onde foi criado um unico processo o qual é executado multiplas vezes em cima de conjuntos de dados
	diferentes (SPMD)

	A implementação adotada então foi, separar a imagem em multiplos blocos diferentes e aplicar o 
	filtro de smoothing em cada bloco em threads separadas, é possível ainda dar quantidades de blocos
	diferentes para cada eixo, possibilitando um controle maior na quantidade total de blocos

	Todos os blocos são iniciados em conjunto e então após iniciados todos, é necessário esperar que todos
	terminem de executar (sincronizem), antes de poder escrever no arquivo as informações

	Potenciais speedups seriam:
		- A implementação para tratar de divisões com resto acaba dando mais trabalho para os blocos
		na ultima linha e coluna, essa carga poderia ser dividida de forma mais uniforme
		- Encontrar uma função ainda mais básica, já que a função do filtro utiliza 4 loops for algo
		que com o aumento do filtro e do bloco pode escalonar rapidamente
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


// #define WIDTH  7680
#define WIDTH  512
// #define HEIGHT 4320
#define HEIGHT 512

//definem o tamanho do bloco a ser aplicado o filtro em cada pixel
/*
	A ideia é separar a imagem em blocos grandes e nesses blocos aplicar o filtro de smoothing
*/
#define NUM_BLOCKS_W 4
#define NUM_BLOCKS_H 4
#define NUM_THREADS NUM_BLOCKS_W*NUM_BLOCKS_H

//define o tamanho do filtro quadrado a ser aplicado
#define FILTER_SIZE 3

int mr [WIDTH][HEIGHT];
int mg [WIDTH][HEIGHT];
int mb [WIDTH][HEIGHT];
int ma [WIDTH][HEIGHT];
int mr2 [WIDTH][HEIGHT];
int mg2 [WIDTH][HEIGHT];
int mb2 [WIDTH][HEIGHT];
int ma2 [WIDTH][HEIGHT];

int mrPadded [WIDTH+2][HEIGHT+2];
int mgPadded [WIDTH+2][HEIGHT+2];
int mbPadded [WIDTH+2][HEIGHT+2];
int maPadded [WIDTH+2][HEIGHT+2];

//struct responsável por armazernar as informações a serem usadas do filtro em cada função
struct filter_info {
	//indica o inicio do bloco a ser aplicado no filtro
    int begin_w;
    int begin_h;

	//indica o fim do bloco a ser aplicado o filtro
	int end_w;
	int end_h;

	//indica o tamanho do filtro (o filtro é quadrado, logo só é necessário uma unica medida)
	
	int filter_size;
	/*
	caso fosse interessante criar um filtro retangular, basta usar,
	int filter_size_w;
	int filter_size_h;
	*/
};

//struct responsável por armazernar os tamanhos dos blocos, responsável tambem para gerenciar os blocos de tamanhos auxiliares
struct block{
	int block_size_w;
	int block_size_h;
};

//struct contendo a informação da thread
struct thread_info
{
	pthread_t threadId;// id da thread
	int threadNum;// numero da thread
	struct filter_info finfo;// informações do filtro a serem usadas no processo gerenciado pela thread
};

//função que aplica o filtro smooth no bloco determinado
void* filtroSmooth(void *arg){
	struct thread_info *tinfo = arg;// recebe o ponteiro e trata como a struct thread info
	int tmpr,tmpg,tmpb,tmpa;//varaiveis temporarias para realizar a somas dos pixies em cada faixa(r,g,b,a)
	
	//realiza o loop para cada pixel dentro do bloco determinado
	for(int h = tinfo->finfo.begin_h ;h<tinfo->finfo.end_h;h++) {
		for(int w=tinfo->finfo.begin_w;w<tinfo->finfo.end_w;w++) {
			//zera as somas dos pixeis envolta do pixel em questão
			tmpr = 0;
			tmpg = 0;
			tmpb = 0;
			tmpa = 0;
			//pega os valores dos pixeis envolta do pixel em questão e soma todos a ele
			for (int i = 0; i < tinfo->finfo.filter_size; i++){
				for (int j = 0; j < tinfo->finfo.filter_size; j++){
					tmpr += mrPadded[h+i][w+j];
					tmpg += mgPadded[h+i][w+j];
					tmpb += mbPadded[h+i][w+j];
					tmpa += maPadded[h+i][w+j];
				}
			}
			
			//realiza a divisão e atribui os valores a nova matriz com o filtro aplicado
			mr2[h][w] = tmpr/9;
			mg2[h][w] = tmpg/9;
			mb2[h][w] = tmpb/9;
			ma2[h][w] = tmpa/9;
			
		}
	}

	
	
			
}


int 
main(int argc, char **argv)
{
	int i,j;
	int fdi, fdo;
	int nlin = 0;
	int ncol = 0;
	char name[128];

	//usado para medir tempo
	/*
	struct timespec begin, end;
	long seconds, nanoseconds;
  	double elapsed; 
	*/
	if(argc<2) {
		printf("Uso: %s nome_arquivo\n",argv[0]);
		exit(0);
	}
	if((fdi=open(argv[1],O_RDONLY))==-1) {
		printf("Erro na abertura do arquivo %s\n",argv[1]);
		exit(0);
	}

	// lê arquivo de imagem
	// formato: 
	//		ncolunas (2bytes) - largura
	//		nlinhas (2bytes) - altura
	// dados: rgba (4bytes)

/*
 * Supondo imagem 512x512, do exemplo, não mantemos tamanho no arquivo
 *
	if(read(fdi,&ncol,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	if(read(fdi,&nlin,2)==-1) {
		printf("Erro na leitura do arquivo\n");		
		exit(0);
	}
	printf("Tamanho da imagem: %d x %d\n",ncol,nlin);
*/
	nlin=ncol=512;
	

	//define os tamanhos dos blocos
	struct block block_info;//vai guardar a informmação do bloco
	struct block block_aux;//caso não seja uma divisão exata, vai ser necessário que o tamanho do ultimo bloco seja maior
	/*
		Uma futura possível otimização seria dividir o restante da divisão entre multiplos blocos,
		distribuindo a carga e evitando casos onde o valor do resto seja quase outro bloco, dobrando
		o trabalho do ultimo bloco
	*/
	int has_block_aux = 0;//flag para indicar a existencia do bloco de tamanho adicional
	if((nlin % NUM_BLOCKS_H) == 0){
		block_info.block_size_h = (nlin / NUM_BLOCKS_H);
		block_aux.block_size_h = 0;
	}else{
		block_info.block_size_h = (nlin / NUM_BLOCKS_H);
		has_block_aux = 1;
		block_aux.block_size_h = (nlin % NUM_BLOCKS_H);
	}

	if((ncol % NUM_BLOCKS_W) == 0){
		block_info.block_size_w = (ncol / NUM_BLOCKS_W);
		block_aux.block_size_w = 0;
	}else{
		block_info.block_size_w = (ncol / NUM_BLOCKS_W);
		has_block_aux = 1;
		block_aux.block_size_w = (ncol % NUM_BLOCKS_W);
	}

	

	// zerar as matrizes (4 bytes, mas usaremos 1 por pixel)
	// void *memset(void *s, int c, size_t n);
	memset(mr,0,nlin*ncol*sizeof(int));
	memset(mg,0,nlin*ncol*sizeof(int));
	memset(mb,0,nlin*ncol*sizeof(int));
	memset(ma,0,nlin*ncol*sizeof(int));
	memset(mr2,0,nlin*ncol*sizeof(int));
	memset(mg2,0,nlin*ncol*sizeof(int));
	memset(mb2,0,nlin*ncol*sizeof(int));
	memset(ma2,0,nlin*ncol*sizeof(int));

	//Criada as matrizes com padding 0, isso causará que as bordas da imagem final sejam mais escuras
	//Isso será necessário para poder gerar os calculos na borda da imagem
	//Outra forma seria repetir os valores da borda para tentar "preservar" a cor
	memset(mrPadded,0,(nlin+2)*(ncol+2)*sizeof(int));
	memset(mgPadded,0,(nlin+2)*(ncol+2)*sizeof(int));
	memset(mbPadded,0,(nlin+2)*(ncol+2)*sizeof(int));
	memset(maPadded,0,(nlin+2)*(ncol+2)*sizeof(int));
	
	
	
	// (ao menos) 2 abordagens: 
	// - ler pixels byte a byte, colocando-os em matrizes separadas
	//	- ler pixels (32bits) e depois usar máscaras e rotações de bits para o processamento.

	// ordem de leitura dos bytes (componentes do pixel) depende se o formato
	// é little ou big endian
	// Assumindo little endian
	for(i=0;i<nlin;i++) {
		for(j=0;j<ncol;j++) {
			read(fdi,&mr[i][j],1);
			read(fdi,&mg[i][j],1);
			read(fdi,&mb[i][j],1);
			read(fdi,&ma[i][j],1);
		}
	}
	close(fdi);
	

	//popula o interior da matriz Padded com os valores da imagem
	for(i=0;i<nlin;i++) {
		for(j=0;j<ncol;j++) {
			mrPadded[i+1][j+1] = mr[i][j];
			mgPadded[i+1][j+1] = mg[i][j];
			mbPadded[i+1][j+1] = mb[i][j];
			maPadded[i+1][j+1] = ma[i][j];
		}
	}

	//inicia a marcação de tempo
	//clock_gettime(CLOCK_REALTIME, &begin);
	
	
	//gera a alocação de memoria para as informações relacionadas a thread
	//havera uma alocação para cada thread, e serão um total de NUM_BLOCKS_H * NUM_BLOCKS_W de threads
	struct thread_info *tinfo = calloc(NUM_THREADS, sizeof(*tinfo));
	int count = 0; //contador responsável por garantir acesso ao elemento certo do vetor de threads
	//realiza o loop para o numero de blocos totais
	for(i=0;i<NUM_BLOCKS_H;i++) {
		for(j=0;j<NUM_BLOCKS_W;j++) {
			tinfo[count].threadNum = count;//informa o numero da thread
			struct filter_info finfo;//informações que o filtro vai usar

			/*
				Aqui são preenchidas as informações padrões, caso não haja bloco auxiliar
				as informações aqui são o pixel de inicio e fim de cada bloco em ambas as dimensões
			*/
			finfo.begin_h = i*block_info.block_size_h;
			finfo.end_h = finfo.begin_h + block_info.block_size_h;
				
			finfo.begin_w = j*block_info.block_size_w;
			finfo.end_w = finfo.begin_w + block_info.block_size_w;

			//caso haja bloco auxiliar,ele sobreescreverá as informações anteriores para tratar cada caso
			if(i == nlin-1){//aqui é tratado se há necessidade de um blocos maiores na ultima linha
				if(has_block_aux == 0){
					finfo.end_h += block_aux.block_size_h;
					
				}
			}
			if(j == ncol-1){//aqui é tratado se há necessidade de blocos maiores na ultima coluna
				if(has_block_aux == 0){	
					finfo.end_w += block_aux.block_size_w;
					
				}
			}
			
			finfo.filter_size = FILTER_SIZE;//atribui então o tamanho do filtro
			/*
				já que o filtro é quadrado, só é necessária uma única dimensão dele, caso fosse de interesse
				ter um filtro retangular seria necessária alterar para acomodar isso
			*/
			tinfo[count].finfo = finfo;//atribui então as informações do filtro a thread

			pthread_create(&tinfo[count].threadId,NULL,&filtroSmooth,&tinfo[count]);//cria a thread
			count++;//adiciona ao contador para gerar as proximas threads

		}
	}
	
	// Join em todas as threads, isso é necessário para garantir que todos os pixeis foram gerados antes de escrever para o arquivo
	for (int i = 0; i < NUM_THREADS; i++){
		pthread_join(tinfo[i].threadId,NULL);
	}

	//finaliza a marcação de tempo
	/*
	clock_gettime(CLOCK_REALTIME, &end);
	seconds = end.tv_sec - begin.tv_sec;
	nanoseconds = end.tv_nsec - begin.tv_nsec;
	elapsed = seconds + nanoseconds*1e-9;
	printf("Tempo gasto: %.3f seconds.\n", elapsed);
	*/
	
	// gravar imagem resultante
	sprintf(name,"%s.new",argv[1]);	
	fdo=open(name,O_WRONLY|O_CREAT);

/*
	write(fdo,&ncol,2);
	write(fdo,&nlin,2);
*/

	for(i=0;i<nlin;i++) {
		for(j=0;j<ncol;j++) {
			write(fdo,&mr2[i][j],1);
			write(fdo,&mg2[i][j],1);
			write(fdo,&mb2[i][j],1);
			write(fdo,&ma2[i][j],1);
		}
	}
	close(fdo);
	
	return 0;
}




