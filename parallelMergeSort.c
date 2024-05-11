#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 1000

void merge(int array[], int left, int middle, int right);
void mergesort(int array[], int left, int right);
int* mergeArrays(int a[], int b[], int n, int m);
void p2a(int a[], int* b, int size);
void l2g(int a[], int b[], int size);
void displayArr(int a[], int size);

int main(int argc, char* argv[]) {
    int i, a_size = NUM, local_size;
    int numtasks, rank, dest, source, rc, count, tag=1, j;
    int a[NUM];
    int global[NUM];
    int* comp;
    
    MPI_Status Stat;
    MPI_Request req;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    local_size = NUM/numtasks;

    //Local array for every process
    int local[(local_size)];

    srand(time(NULL));

    //Setup array with random numbers
    if(rank == 0){
        for(i=0; i<NUM; i++){
            a[i] = rand()%NUM;
        }
    }

    MPI_Scatter(a, local_size, MPI_INT, local, local_size, MPI_INT, 0, MPI_COMM_WORLD);


    //displayArr(local, local_size);
    
    if(rank==0){//Parent Process

        //displayArr(a, NUM);

        clock_t begin, end;
        double time_spent;
        begin = clock();

        //sequential mergesort
        mergesort(local, 0, local_size-1);

        //Push sorted local array to global array
        l2g(global, local, local_size);

        int j, recv_size = local_size;
        int buff[recv_size];

        for(j=0; j<numtasks-1; j++){
            //Receive sorted array from child process
            MPI_Recv(buff, recv_size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
            //Merge received array and global array together
            comp = mergeArrays(global, buff, local_size, recv_size);
            local_size += recv_size;
            //Pointer to Array
            p2a(global, comp, local_size);
        }
        end = clock();
        time_spent=(double)(end-begin)/CLOCKS_PER_SEC;

        int k;

        // displayArr(global, NUM);

        printf("Time spent (Parallel): %f\n", time_spent);

        begin = clock();

        //Time sequential merge sort on same set of numbers
        mergesort(a, 0, NUM-1);
        end = clock();
        time_spent=(double)(end-begin)/CLOCKS_PER_SEC;
        printf("Time spent (Non-Parallel): %f\n", time_spent);

    }
    else{//child process
        //sequential mergesort the given array from scatter
        mergesort(local, 0, local_size-1);

        //send the sorted array to the parent process
        MPI_Send(local, local_size, MPI_INT, 0, 0, MPI_COMM_WORLD);

    }
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

int* mergeArrays(int a[], int b[], int n, int m) {
    int* c;
    int size = n+m;
    c = malloc(size*sizeof(int));

    int i = 0, j = 0, k = 0;
    while(i <= n-1 && j <= m-1){

    if(a[i] <= b[j]){

        c[k++] = a[i++];
    }else{

        c[k++] = b[j++];
    }
    }

    while(i <= n-1){
        c[k++] = a[i++];
    }

    while(j <= m-1){
        c[k++] = b[j++];
    }

    return c;
}

// Function to convert pointer to array
void p2a(int a[], int* b, int size) {
    // Implementation to copy data from pointer to array
    int i;
    for(i=0; i < size; i++){
        a[i] = b[i];
    }
}

// Function to convert local array to global array
void l2g(int a[], int b[], int size) {
    // Implementation to copy data from local array to global array
    int i;
    for(i=0; i < size; i++){
        a[i] = b[i];
    }
}

void displayArr(int a[], int size){
    printf("a = {");
    for(int i=0; i<size; i++){
        printf("%d",a[i]);
        if(i != size-1)
            printf(",");
    }
    printf("}\n");
}