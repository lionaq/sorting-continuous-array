#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 100 //total size of the arrays

/* 
    To better understand this algorithm, think about a chasing light pattern (https://www.youtube.com/watch?v=oDlwtnct8ME)
    The first 2 seconds in the video is exactly how the algorithm would work, the first set of turned on lights would pair
    with one another and sort, then after they're done sorting it transitions over to the second set of turned on lights 
    which would pair and sort once again. 

    This is done so large numbers from p0 would "bubble" up to pn. It would do all this until the numbers are sorted 
    in increasing order from p0 to pn.

 */

void swap(int* a, int* b);
void heapify(int arr[], int N, int i);
void heapSort(int arr[], int N);
void displayArr(int a[], int size);
void sendForward(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat);
void recvBackward(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat);

int main(int argc, char* argv[]) {
    int local_size;
    int numtasks, rank;
    int buff[3] = {0, 0, 0}; // array element, ownCond, fCond, bCond
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

    heapSort(local, local_size);

    for(int i = 0; i < numtasks/2; i++){

        buff[1] = 0;
        buff[2] = 0;

        while(buff[1] == 0){                      // p1*, p2, p3*, p4 ; pair and sort
            if(rank % 2 == 0){ // even
                sendForward(buff, local, local_size, rank, 1, Stat);
            }
            if(rank % 2 != 0){ // odd
                recvBackward(buff, local, local_size, rank, 1, Stat);
            }
        }

        if(rank != 0 && rank != numtasks - 1){  // p1, p2*, p3, p4* ; pair and sort
            while(buff[2] == 0){
                if(rank % 2 != 0){//Odd
                    sendForward(buff, local, local_size, rank, 2, Stat);
                }
                if(rank % 2 == 0){ //Even
                    recvBackward(buff, local, local_size, rank, 2, Stat);
                }
            }
        }

    }
    printf("RANK %d Completed ", rank);displayArr(local,local_size);
    MPI_Finalize();
}

void sendForward(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat){
    int temp;
    buff[0] = local[local_size-1];
    MPI_Send(buff, 3, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    MPI_Recv(buff, 3, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &Stat);
    if(buff[bOffset] == 0){
        temp = local[local_size-1];
        local[local_size-1] = buff[0];
        buff[0] = temp;
        heapSort(local, local_size);
    }
}

void recvBackward(int buff[], int* local, int local_size, int rank, int bOffset, MPI_Status Stat){
    int temp;
    buff[0] = local[local_size-1];
    MPI_Recv(buff, 3, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &Stat);
    if(buff[0] > local[0]){
        temp = local[0];
        local[0] = buff[0];
        buff[0] = temp;
        MPI_Send(buff, 3, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
        heapSort(local, local_size);
    }else{
        buff[bOffset] = 1;
        MPI_Send(buff, 3, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
    }
}

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