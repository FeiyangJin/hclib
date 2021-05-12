#include "hclib_cpp.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define REAL int
static int BASE_CASE; //the base case of the computation (2*POWER)
static int POWER; //the power of two the base case is based on


static const unsigned int Q[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
static const unsigned int S[] = {1, 2, 4, 8};

//provides a look up for the Morton Number of the z-order curve given the x and y coordinate
//every instance of an (x,y) lookup must use this function
unsigned int z_convert(int row, int col){

    unsigned int z; // z gets the resulting 32-bit Morton Number.  
    // x and y must initially be less than 65536.
    // The top and the left boundary 

    col = (col | (col << S[3])) & Q[3];
    col = (col | (col << S[2])) & Q[2];
    col = (col | (col << S[1])) & Q[1];
    col = (col | (col << S[0])) & Q[0];

    row = (row | (row << S[3])) & Q[3];
    row = (row | (row << S[2])) & Q[2];
    row = (row | (row << S[1])) & Q[1];
    row = (row | (row << S[0])) & Q[0];

    z = col | (row << 1);

    return z;
}


//converts (x,y) position in the array to the mixed z-order row major layout
int block_convert(int row, int col){
    int block_index = z_convert(row >> POWER, col >> POWER);
    return (block_index * BASE_CASE << POWER) 
        + ((row - ((row >> POWER) << POWER)) << POWER) 
        + (col - ((col >> POWER) << POWER));
}


//init the matric in order
void order(REAL *M, int n){
    int i,j;
    for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++){
            M[block_convert(i,j)] = i * n + j;   
        }
    }
}


//init the matrix to all ones 
void one(REAL *M, int n){
    int i;
    for(i = 0; i < n * n; i++) {
        M[i] = 1.0;
    }
}


//init the matrix to all zeros 
void zero(REAL *M, int n){
    int i;
    for(i = 0; i < n * n; i++) {
        M[i] = 0.0;
    }
}


//init the matrix to random numbers
void init_rm(REAL *M, int n){
    srand (time(NULL));
    int i;
    for(i = 0; i < n * n; i++) {
      // random int between 1 and 10
      M[i] = rand() % 10 + 1;
    }
}


//iterative solution for matrix multiplication
void iter_matmul(REAL *A, REAL *B, REAL *C, int n){
    int i, j, k;

    for(i = 0; i < n; i++){
        for(k = 0; k < n; k++){
            REAL c = 0.0;
            for(j = 0; j < n; j++){
                c += A[block_convert(i,j)] * B[block_convert(j,k)];
            }
            C[block_convert(i, k)] = c;
        }
    }
}


//iterative solution for matrix multiplication
void iter_matmul_rm(REAL *A, REAL *B, REAL *C, int n){
    int i, j, k;

    for(i = 0; i < n; i++){
        for(k = 0; k < n; k++){
            REAL c = 0.0;
            for(j = 0; j < n; j++){
                c += A[i * n + j] * B[j * n + k];
            }
            C[i * n + k] = c;
        }
    }
}


//recursive parallel solution to matrix multiplication
void mat_mul_par(const REAL *const A, const REAL *const B, REAL *C, int n){
    ds_hclib_ready(true);
    //BASE CASE: here computation is switched to itterative matrix multiplication
    //At the base case A, B, and C point to row order matrices of n x n
    if(n == BASE_CASE) {
        int i, j, k;
        for(i = 0; i < n; i++){
            for(k = 0; k < n; k++){
                REAL c = 0.0;
                for(j = 0; j < n; j++){
                    c += A[i * n + j] * B[j* n + k];
                }
                C[i * n + k] += c;
            }
        }

        return;
    }

    //partition each matrix into 4 sub matrices
    //each sub-matrix points to the start of the z pattern
    const REAL *const A1 = &A[block_convert(0,0)];
    const REAL *const A2 = &A[block_convert(0, n >> 1)]; //bit shift to divide by 2
    const REAL *const A3 = &A[block_convert(n >> 1,0)];
    const REAL *const A4 = &A[block_convert(n >> 1, n >> 1)];

    const REAL *const B1 = &B[block_convert(0,0)];
    const REAL *const B2 = &B[block_convert(0, n >> 1)];
    const REAL *const B3 = &B[block_convert(n >> 1, 0)];
    const REAL *const B4 = &B[block_convert(n >> 1, n >> 1)];
    
    REAL *C1 = &C[block_convert(0,0)];
    REAL *C2 = &C[block_convert(0, n >> 1)];
    REAL *C3 = &C[block_convert(n >> 1,0)];
    REAL *C4 = &C[block_convert(n >> 1, n >> 1)];

    ds_hclib_ready(false);
    hclib::promise_t<void> *p1 = new hclib::promise_t<void>();
    // hclib::promise_t<void> *p2 = new hclib::promise_t<void>();
    // hclib::promise_t<void> *p3 = new hclib::promise_t<void>();
    hclib::async([&](){
        mat_mul_par(A1,B1,C1,n>>1);
        ds_hclib_ready(true);
        p1->put();
        ds_hclib_ready(false);
    });
    
    hclib::finish([&](){    
        hclib::async([&](){
            mat_mul_par(A1,B2,C2,n>>1);
            ds_hclib_ready(false);
        });

        hclib::async([&](){
            mat_mul_par(A3,B1,C3,n>>1);
            ds_hclib_ready(false);
        });

            mat_mul_par(A3,B2,C4,n>>1);
            ds_hclib_ready(false);
    });
    ds_hclib_ready(true);
    p1->get_future()->wait();
    // p2->get_future()->wait();
    // p3->get_future()->wait();

    
    // hclib::promise_t<int> *p4 = new hclib::promise_t<int>();
    // hclib::promise_t<int> *p5 = new hclib::promise_t<int>();
    // hclib::promise_t<int> *p6 = new hclib::promise_t<int>();

    ds_hclib_ready(false);
    hclib::finish([&](){
        hclib::async([&](){
            mat_mul_par(A2,B3,C1,n>>1);
            ds_hclib_ready(false);
        });

        hclib::async([&](){
            mat_mul_par(A2,B4,C2,n>>1);
            ds_hclib_ready(false);
        });

        hclib::async([&](){
            mat_mul_par(A4,B3,C3,n>>1);
            ds_hclib_ready(false);
        });

            mat_mul_par(A4,B4,C4,n>>1);
            ds_hclib_ready(false);
    });

    // hclib::async([&](){
    //     mat_mul_par(A2,B3,C1,n>>1);
    //     p4->put(4);
    //     ds_hclib_ready(false);
    // });

    // hclib::async([&](){
    //     mat_mul_par(A2,B4,C2,n>>1);
    //     p5->put(5);
    //     ds_hclib_ready(false);
    // });

    // hclib::async([&](){
    //     mat_mul_par(A4,B3,C3,n>>1);
    //     p6->put(6);
    //     ds_hclib_ready(false);
    // });

    // ds_hclib_ready(false);
    // mat_mul_par(A4,B4,C4,n>>1);

    // ds_hclib_ready(true);
    // p4->get_future()->wait();
    // p5->get_future()->wait();
    // p6->get_future()->wait();
}


//prints the matrix
void print_matrix(REAL *M, int n){
    int i,j;
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            printf("%d ", M[block_convert(i,j)]);
        }
        printf("\n");
    }
    printf("\n");
}


void compare_matrix(REAL *C, REAL *D, int n){
    int i;
    for(int i=0; i < n*n; i++){
        if(C[i] != D[i]){
            printf("the two matrix differ at index %d \n",i);
            return;
        }
    }
    printf("parallel result is the same as sequential result \n");
}


int main(int argc, char *argv[]){
    // 2048 takes time up to 206.479 seconds
  int n = argc>1?atoi(argv[1]) : 2048; // default n value is 2048
  printf("multiplying two matrices of size %d * %d \n", n, n);

  POWER = 8; //default k value
  BASE_CASE = (int) pow(2.0, (double) POWER);


  REAL *A, *B, *C, *D;

  A = (REAL *) malloc(n * n * sizeof(REAL)); //source matrix 
  B = (REAL *) malloc(n * n * sizeof(REAL)); //source matrix
  C = (REAL *) malloc(n * n * sizeof(REAL)); //result matrix sequential
  D = (REAL *) malloc(n * n * sizeof(REAL)); //result matrix parallel

    /* initialize */
    init_rm(A, n);
    init_rm(B, n);
    zero(C, n);

  char const *deps[] = { "system" };
  hclib::launch(deps, 1, [&]() {
    ds_hclib_ready(true);
    
    // sequential calculation
    long start = hclib_current_time_ms();
    
    //iter_matmul(A,B,C,n);
    
    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    
    //printf("Sequential Matrix Multiplication Time = %.3f\n", dur);

    start = hclib_current_time_ms();
    mat_mul_par(A,B,D,n);
    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;

    printf("parallel Matrix Multiplication Time = %.3f\n", dur);
    
    //compare two results
    //compare_matrix(C,D,n);
    ds_hclib_ready(false);
    printf("cache size is %d \n",ds_get_cache_size());
    printf("number of task is %d \n",get_task_id_unique());
    printf("number of nt join %d \n", get_nt_count());
  });

    /* release memory */
    free(A);
    free(B);
    free(C);

  return 0;
}