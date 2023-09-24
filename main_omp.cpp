/* 
* Rainbow Game - Versão OpenMP
* Maria Luíza Rodrigues da Silva - 120527
*/
#include <stdio.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <time.h>
#define N 2048
#define PRINT_SIZE 50
#define MAX_TURNS 5
#define MAX_THREADS 1
using namespace std;

void printGrid(vector<vector<float>> grid) {
    vector<float> aux;
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

void drawGlider(vector<vector<float>> &grid) {
    int lin = 1, col = 1;

    grid[lin  ][col+1] = 1.0;
    grid[lin+1][col+2] = 1.0;
    grid[lin+2][col  ] = 1.0;
    grid[lin+2][col+1] = 1.0;
    grid[lin+2][col+2] = 1.0;
}

void drawRPentomino(vector<vector<float>> &grid) {
    int lin = 10, col = 30;

    grid[lin  ][col+1] = 1.0;
    grid[lin  ][col+2] = 1.0;
    grid[lin+1][col  ] = 1.0;
    grid[lin+1][col+1] = 1.0;
    grid[lin+2][col+1] = 1.0;
}

void initializeGrid(vector<vector<float>> &grid) {
    vector<float> aux;

    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            aux.push_back(0);
        }
        grid.push_back(aux);
        aux.clear();
    }
    drawGlider(grid);
    drawRPentomino(grid);
}

int getNeighbors(vector<vector<float>> grid, int i, int j, float *mean) {
    float sum = 0.0;
    int li, ci, k, l, count = 0;
    vector<int> lineIndex = {calcMinIndex(i), i, (i+1) % N};
    vector<int> colIndex = {calcMinIndex(j), j, (j+1) % N};
    
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

float getMean(vector<vector<float>> grid, int i, int j) {
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

float getNewCellState(vector<vector<float>> grid, int i, int j) {
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

void runGeneration(vector<vector<float>> &grid) {
    int i, j;
    vector<vector<float>> newgrid;
    initializeGrid(newgrid);

    omp_set_num_threads(MAX_THREADS);
    
    #pragma omp parallel default(none) private(j) shared(i, newgrid, grid) 
    {
        #pragma omp for
        for(i=0; i<N; i++) {
            for(j=0; j<N; j++) {
                newgrid[i][j] = getNewCellState(grid, i, j);
            }
        }
    }
    
    for(int i=0; i<N; i++)
        grid[i] = newgrid[i];
}

int main() {
    vector<vector<float>> grid;
    double start, end;
        
    initializeGrid(grid);

    start = omp_get_wtime();    
    for(int i=0; i<MAX_TURNS; i++) {
        runGeneration(grid);
    }
    end = omp_get_wtime();

    printGrid(grid);
    printf("\nTempo decorrido: %f segundos.\n", end-start);
}