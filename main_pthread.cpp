// Maria Luíza Rodrigues da Silva - 120527
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#define N 50
#define MAX_TURNS 4
#define MAX_THREADS 2
using namespace std;

struct gridData{
    vector<vector<float>> grid;
    vector<vector<float>> newGrid;
    int t_index;
};

 void printGrid(vector<vector<float>> grid) {
    vector<float> aux;
    int alive = 0;

    for(int i=0; i<N; i++) {
        for(int j=0; j<N; j++) {
            if(grid[i][j] > 0.0000) {
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
                
                if(grid[lineIndex[k]][colIndex[l]] > 0.0000) // Acima de 0.0 já é considerado viva 
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
    
    if(((cState > 0.0000) && (numNeighbors == 2 || numNeighbors == 3)) || (cState == 0.0000 && numNeighbors == 3)){
        nState = mean;
    }

    return nState;
}

void * calcNewGrid(void *res) {
    gridData *data;
    data = (gridData *) res;
    int start = (*data).t_index * (N/MAX_THREADS);
    int end;

    if((*data).t_index == MAX_THREADS-1) {
        end = N;
    } else {
        end = start + (N/MAX_THREADS);
    }
    printf("Thread[%d]: start=%d end=%d\n", (*data).t_index, start, end);

    for(int i=start; i<end; i++) {
        for(int j=0; j<N; j++) {
            (*data).newGrid[i][j] = getNewCellState((*data).grid, i, j);
        }
    }
    //printf("\nThread[%d]\n", (*data).t_index);
    //printGrid((*data).newGrid);
    
    pthread_exit(NULL);
}

gridData runGeneration(gridData data) {
    pthread_t threads[MAX_THREADS];
    struct gridData newData;
    newData = data;

    for(int i=0; i<MAX_THREADS; i++) {
        newData.t_index = i;
        pthread_create(&threads[i], NULL, calcNewGrid, (void *) &newData);
        printf("Created Thread[%d]\n", i);
    }

    for(int i=0; i<MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    for(int i=0; i<N; i++)
        newData.grid[i] = newData.newGrid[i];

    return newData;
}

int main() {
    gridData data;
    vector<vector<float>> grid;
    initializeGrid(grid);
    
    data.grid = grid;
    data.newGrid = grid;

//    printf("--------Inicio--------\n");
//    printGrid(grid);

    for(int i=0; i<MAX_TURNS; i++) {
        data = runGeneration(data);
       // printGrid(data.grid);
    }

    printf("--------Final--------\n");
    printGrid(data.grid);
}