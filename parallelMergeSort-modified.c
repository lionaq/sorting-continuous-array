#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 20

void merge(int array[], int left, int middle, int right);
void mergesort(int array[], int left, int right);
void displayArr(int a[], int size);

int main(int argc, char* argv[]) {
    int i, a_size = NUM, local_size;
    int numtasks, rank, dest, source, rc, count, tag=1, j;
    int a[NUM];
    int global[NUM];
    int* comp;
    int buff[2] = {1, 0};
    int temp;
    
    MPI_Status Stat;
    MPI_Request req;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    local_size = NUM/numtasks;

    //Local array for every process
    int local[(local_size)];

    //Setup array with random numbers
    for(i=0; i<local_size; i++){
        local[i] = rand()%NUM;
    }
    printf("\n");

    while(buff[1] == 0){

        mergesort(local, 0, local_size-1); //will be replaced with an in place sorting algorithm

        if(rank == 2)
        printf("STAT: %d ", Stat.MPI_ERROR); printf("sorted array:"); displayArr(local, local_size); printf("buff[1]: %d \n ", buff[1]);
        //printf("rank: %d \n", rank);

        if(rank == 1)
            printf("ITS A ME RANK 1\n");
        
        if(rank == 3)
            printf("ITS A ME RANK 3\n");

        buff[0] = local[local_size-1];

        if(rank < 3){
            printf("SENDING! %d\n", rank);
            MPI_Send(buff, 2, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            printf("RECEIVING! %d\n", rank);
            MPI_Recv(buff, 2, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &Stat);
            if(buff[1] == 0){
                temp = local[local_size-1];
                local[local_size-1] = buff[0];
                buff[0] = temp;
                mergesort(local, 0, local_size-1);
                if(rank == 2)
                    printf("HIHIHI ");
            }
        }
        
        if(rank > 0){
            MPI_Recv(buff, 2, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &Stat);
            printf("RECEIVING WEWE! %d\n", rank);
            //printf("RECV: %d\n", buff[0]);
            //printf("LOCAL: %d\n", local[0]);
            if(buff[0] > local[0]){
                temp = local[0];
                local[0] = buff[0];
                buff[0] = temp;
                MPI_Send(buff, 2, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
                mergesort(local, 0, local_size-1);
            }else{
                buff[1] = 1;
                MPI_Send(buff, 2, MPI_INT, rank - 1, 1, MPI_COMM_WORLD);
            }
        }

        //printf("%d \n", buff[0]);
    }

    printf("RANK!!!!!!!!!!!!!:%d ", rank);printf("sorted array2:"); displayArr(local, local_size);

    MPI_Finalize();

}

// Function to perform sequential merge sort
void mergesort(int array[], int left, int right) {
    // Implementation of merge sort
    if(left < right){
        int middle = (left + right)/2;
        mergesort(array, left, middle);
        mergesort(array, middle+1, right);
        merge(array, left, middle, right);
    }
}

// Function to merge two sorted arrays
void merge(int array[], int left, int middle, int right) {
    // Implementation of merge operation
    int temp[NUM];
    int i = left, j = middle+1, k=0;

    while(i <= middle && j <= right){
        if(array[i] <= array[j]){
            temp[k++] = array[i++];
        }else{
            temp[k++] = array[j++];
        }
    }

    while(i <= middle){
         temp[k++] = array[i++];
    }

    while(j <= right){
        temp[k++] = array[j++];
    }

    k--;
    while(k >=0){
        array[left + k] = temp[k];
        k--;
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