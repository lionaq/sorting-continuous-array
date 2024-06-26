#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 40 //total size of the arrays

/* 
    To better understand this algorithm, think about a chasing light pattern (https://www.youtube.com/watch?v=oDlwtnct8ME)
    The first 2 seconds in the video is exactly how the algorithm would work, the first set of turned on lights would pair
    with one another and sort, then after they're done sorting it transitions over to the second set of turned on lights 
    which would pair and sort once again. 

    This is done so large numbers from p0 would "bubble" up to pn. It would do all this until the numbers are sorted 
    in increasing order from p0 to pn.

 */

void sendRight(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat);
void recvLeft(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat);

void swap(int* a, int* b);
void heapify(int arr[], int N, int i);
void heapSort(int arr[], int N);
void displayArr(int a[], int size);

int main(int argc, char* argv[]) {
    int local_size;
    int numtasks, rank;
    int buff[3] = {0, 0, 0}; // array element, nOffstCnd, offstCnd
    int temp;
    
    MPI_Status Stat;
    MPI_Request req;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(numtasks % 2 != 0 && numtasks != 1){
        printf("Even Number of Processors Only! (or 1 processor if odd)\n");
        return 1;
    }

    local_size = NUM/numtasks;

    //Local array for every process
    int local[(local_size)];
    srand(time(NULL) + rank);
    //Setup array with random numbers
    for(int i=0; i<local_size; i++){
        local[i] = rand()%NUM;
    }

    clock_t begin, end;
    double time_spent;
    begin = clock();

    heapSort(local, local_size);
    printf("Rank %d ARRAY (LOCAL):", rank);displayArr(local,local_size); //unsorted pn array
    fflush(stdin);

    for(int i = 0; i < numtasks/2; i++){

        buff[1] = 0; //condition for first step
        buff[2] = 0; //condition for second step

        //compare and swap first set until sorted
        while(buff[1] == 0){                      // p1*, p2, p3*, p4 ; pair and sort
            if(rank % 2 == 0){ // even
                sendRight(buff, local, local_size, rank, 1, Stat);
            }
            if(rank % 2 != 0){ // odd
                recvLeft(buff, local, local_size, rank, 1, Stat);
            }
        }

        if(rank != 0 && rank != numtasks - 1){  // p1, p2*, p3, p4* ; pair and sort
            while(buff[2] == 0){ //compare and swap second set until sorted
                if(rank % 2 != 0){//Odd
                    sendRight(buff, local, local_size, rank, 2, Stat);
                }
                if(rank % 2 == 0){ //Even
                    recvLeft(buff, local, local_size, rank, 2, Stat);
                }
            }
        }

    }
    end = clock();
    time_spent=(double)(end-begin)/CLOCKS_PER_SEC;
    printf("Rank %d Time spent (Parallel): %f\n", rank, time_spent);displayArr(local,local_size); //sorted from p0 to pn
    fflush(stdin);

    MPI_Finalize();
}

void sendRight(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat){
    int temp;
    buff[0] = local[local_size-1];
    MPI_Send(buff, 3, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    MPI_Recv(buff, 3, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &Stat);
    if(buff[bOffset] == 0){
        local[local_size-1] = buff[0];
        heapSort(local, local_size);
    }
}

void recvLeft(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat){
    int temp;
    buff[0] = local[local_size-1];
    MPI_Recv(buff, 3, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &Stat);
    if(buff[0] > local[0]){ //swap
        temp = local[0];
        local[0] = buff[0];
        buff[0] = temp;
        MPI_Send(buff, 3, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
        heapSort(local, local_size);
    }else{ //array in processors sorted in increasing order from pn-1 to pn
        buff[bOffset] = 1;
        MPI_Send(buff, 3, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
    }
}


//for inplace sorting algorithm

void swap(int* a, int* b)
{

    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int arr[], int N, int i)
{
    // Find largest among root,
    // left child and right child

    // Initialize largest as root
    int largest = i;

    // left = 2*i + 1
    int left = 2 * i + 1;

    // right = 2*i + 2
    int right = 2 * i + 2;

    // If left child is larger than root
    if (left < N && arr[left] > arr[largest])

        largest = left;

    // If right child is larger than largest
    // so far
    if (right < N && arr[right] > arr[largest])

        largest = right;

    // Swap and continue heapifying
    // if root is not largest
    // If largest is not root
    if (largest != i) {

        swap(&arr[i], &arr[largest]);

        // Recursively heapify the affected
        // sub-tree
        heapify(arr, N, largest);
    }
}

void heapSort(int arr[], int N)
{

    // Build max heap
    for (int i = N / 2 - 1; i >= 0; i--)

        heapify(arr, N, i);

    // Heap sort
    for (int i = N - 1; i >= 0; i--) {

        swap(&arr[0], &arr[i]);

        // Heapify root element
        // to get highest element at
        // root again
        heapify(arr, i, 0);
    }
}

void displayArr(int a[], int size){
    printf("{");
    for(int i=0; i<size; i++){
        printf("%d",a[i]);
        if(i != size-1)
            printf(",");
    }
    printf("}\n");
}