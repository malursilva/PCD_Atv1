/* 
* Rainbow Game - Versão PThreads
* Maria Luíza Rodrigues da Silva - 120527
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#define N 2048
#define PRINT_SIZE 50
#define MAX_TURNS 5
#define MAX_THREADS 1

struct grid_data {
    float **grid;
    float **newGrid;
} typedef grid_data;

struct thread_data {
    grid_data *data;
    int t_index;
} typedef thread_data;

struct thread_data thread_data_array[MAX_THREADS];

void printGrid(float **grid) {
    int alive = 0;

    for(int i=0; i<PRINT_SIZE; i++) {
        for(int j=0; j<PRINT_SIZE; j++) {
            if(grid[i][j] > 0.000) {
                printf("o ");
                alive++;
            } else {
                printf("x ");
            }
            
        }
        printf("\n");
    }

    printf("Alive cells: %d\n", alive);
}

int calcMinIndex(int index) {
    if(index - 1 < 0) return N-1;
    else return index - 1;
}

void drawGlider(float **grid) {
    int lin = 1, col = 1;

    grid[lin  ][col+1] = 1.0;
    grid[lin+1][col+2] = 1.0;
    grid[lin+2][col  ] = 1.0;
    grid[lin+2][col+1] = 1.0;
    grid[lin+2][col+2] = 1.0;
}

void drawRPentomino(float **grid) {
    int lin = 10, col = 30;

    grid[lin  ][col+1] = 1.0;
    grid[lin  ][col+2] = 1.0;
    grid[lin+1][col  ] = 1.0;
    grid[lin+1][col+1] = 1.0;
    grid[lin+2][col+1] = 1.0;
}

void initializeGrid(float **grid) {
    for(int i=0; i<N; i++) {
        grid[i] = (float *) malloc(N * sizeof(float));
        for(int j=0; j<N; j++) {
            grid[i][j] = 0.0;
        }
    }
    drawGlider(grid);
    drawRPentomino(grid);
}

int getNeighbors(float **grid, int i, int j, float *mean) {
    float sum = 0.0;
    int li, ci, k, l, count = 0;
    int lineIndex[3] = {calcMinIndex(i), i, (i+1) % N};
    int colIndex[3] = {calcMinIndex(j), j, (j+1) % N};
    
    for(k=0; k<3; k++) {
        for(l=0; l<3; l++) {
            if(!(lineIndex[k]==i && colIndex[l]==j)) { 
                sum += grid[lineIndex[k]][colIndex[l]]; // Aproveita para achar a média dos vizinhos
                
                if(grid[lineIndex[k]][colIndex[l]] > 0.0) // Acima de 0.0 já é considerado viva 
                    count++;
                
            }
        }
    }
    (*mean) = sum / 8; 

    return count;
}

float getMean(float **grid, int i, int j) {
    int k, l;
    int minL = calcMinIndex(i);
    int minC = calcMinIndex(j);
    int maxL = (i+1) % N;
    int maxC = (j+1) % N;
    float sum = 0.0;

    for(k=minL; k=maxL; k++) {
        for(l=minC; l=minL; l++) {
            if(k!=i && l!=j) { 
                sum += grid[k][l];
            }
        }
    }

    return sum / 8;
}

float getNewCellState(float **grid, int i, int j) {
    float cState = grid[i][j];
    float nState = 0.0;
    float mean;
    int numNeighbors = getNeighbors(grid, i, j, &mean);
    
    // Os casos em que a célula fica viva: 1- Se já estiver viva e ter 2 ou 3 vizinhos vivos; 2- Se estiver morta e ter 3 vizinhos vivos
    if(((cState > 0.0) && (numNeighbors == 2 || numNeighbors == 3)) || (cState == 0.0 && numNeighbors == 3)){
        nState = mean;
    }

    return nState;
}

void * calcNewGrid(void *res) {
    thread_data *tData;
    grid_data *gridData;

    tData = (thread_data *) res;
    gridData = (*tData).data;

    int index = (*tData).t_index;
    int start = index * (N/MAX_THREADS);
    int end;

    if(index == MAX_THREADS-1) {
        end = N;
    } else {
        end = start + (N/MAX_THREADS);
    }
    
    for(int i=start; i<end; i++) {
        for(int j=0; j<N; j++) {
            gridData->newGrid[i][j] = getNewCellState((*gridData).grid, i, j);
        }
    }

    pthread_exit(NULL);
}

void runGeneration(grid_data *data) {
    pthread_t threads[MAX_THREADS];
    
    for(int i=0; i<MAX_THREADS; i++) {
        thread_data_array[i].data = data;
        thread_data_array[i].t_index = i;
        pthread_create(&threads[i], NULL, calcNewGrid, (void *) &thread_data_array[i]);
    }

    for(int i=0; i<MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    (*data).grid = (*data).newGrid;
}

int main() {
    grid_data data;
    float **grid, **newgrid;
    struct timeval inicio, final;
    int tmili;

    gettimeofday(&inicio, NULL);
    
    grid = (float **) malloc(N * sizeof(float *));      
    newgrid = (float **) malloc(N * sizeof(float *));      
    
    initializeGrid(grid);
    initializeGrid(newgrid);
    
    data.grid = grid;
    data.newGrid = newgrid;

    for(int i=0; i<MAX_TURNS; i++) {
        runGeneration(&data);
       // printf("------------Generation %d------------\n", i+1);
       // printGrid(grid);
    }
    gettimeofday(&final, NULL);
    tmili = (int) (1000 * (final.tv_sec - inicio.tv_sec) + (final.tv_usec - inicio.tv_usec) / 1000);

    printGrid(data.grid);
    printf("Tempo decorrido: %d milisegundos\n", tmili);
}