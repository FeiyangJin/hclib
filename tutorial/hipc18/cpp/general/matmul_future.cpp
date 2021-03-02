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


    hclib::future_t<void>* f1 = hclib::async_future([=]() { 
      mat_mul_par(A1,B1,C1,n>>1);
      return;
    });


    hclib::future_t<void>* f2 = hclib::async_future([=]() { 
      mat_mul_par(A1,B2,C2,n>>1);
      return;
    });

    hclib::future_t<void>* f3 = hclib::async_future([=]() { 
      mat_mul_par(A3,B1,C3,n>>1);
      return;
    });

    hclib::future_t<void>* f4 = hclib::async_future([=]() { 
      mat_mul_par(A3,B2,C4,n>>1);
      return;
    });

    // mat_mul_par(A1,B1,C1,n>>1);
    // mat_mul_par(A1,B2,C2,n>>1);
    // mat_mul_par(A3,B1,C3,n>>1);
    // mat_mul_par(A3,B2,C4,n>>1);
    f1->wait();
    f2->wait();
    f3->wait();
    f4->wait();


    hclib::future_t<void>* f5 = hclib::async_future([=]() { 
      mat_mul_par(A2,B3,C1,n>>1);
      return;
    });

    hclib::future_t<void>* f6 = hclib::async_future([=]() { 
      mat_mul_par(A2,B4,C2,n>>1);
      return;
    });

    hclib::future_t<void>* f7 = hclib::async_future([=]() { 
      mat_mul_par(A4,B3,C3,n>>1);
      return;
    });

    mat_mul_par(A4,B4,C4,n>>1);

    f5->wait();
    f6->wait();
    f7->wait();
    // mat_mul_par(A2,B3,C1,n>>1);
    // mat_mul_par(A2,B4,C2,n>>1);
    // mat_mul_par(A4,B3,C3,n>>1);
    // mat_mul_par(A4,B4,C4,n>>1);
}


// //recursive parallel solution to matrix multiplication - row major order
// void mat_mul_par_rm(REAL *A, REAL *B, REAL *C, int n, int orig_n){
//     //BASE CASE: here computation is switched to itterative matrix multiplication
//     //At the base case A, B, and C point to row order matrices of n x n
//     if(n == BASE_CASE) {
//         int i, j, k;
//         for(i = 0; i < n; i++){
//             for(k = 0; k < n; k++){
//                 REAL c = 0.0;
//                 for(j = 0; j < n; j++){
//                     c += A[i * orig_n + j] * B[j * orig_n + k];
//                 }
//                 C[i * orig_n + k] += c;
//             }
//         }
//         return;
//     }

//     REAL *A1 = &A[0];
//     REAL *A2 = &A[n >> 1]; //bit shift to divide by 2
//     REAL *A3 = &A[(n * orig_n) >> 1];
//     REAL *A4 = &A[((n * orig_n) + n) >> 1];

//     REAL *B1 = &B[0];
//     REAL *B2 = &B[n >> 1];
//     REAL *B3 = &B[(n * orig_n) >> 1];
//     REAL *B4 = &B[((n * orig_n) + n) >> 1];
    
//     REAL *C1 = &C[0];
//     REAL *C2 = &C[n >> 1];
//     REAL *C3 = &C[(n * orig_n) >> 1];
//     REAL *C4 = &C[((n * orig_n) + n) >> 1];

//     //recursively call the sub-matrices for evaluation in parallel
//     /*cilk_spawn*/ mat_mul_par_rm(A1, B1, C1, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A1, B2, C2, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A3, B1, C3, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A3, B2, C4, n >> 1, orig_n);
//     /*cilk_sync*/; //wait here for first round to finish

//     /*cilk_spawn*/ mat_mul_par_rm(A2, B3, C1, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A2, B4, C2, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A4, B3, C3, n >> 1, orig_n);
//     /*cilk_spawn*/ mat_mul_par_rm(A4, B4, C4, n >> 1, orig_n);
//     /*cilk_sync*/; //wait here for all second round to finish

// }


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
  int n = 2048; //default n value
  POWER = 6; //default k value
  BASE_CASE = (int) pow(2.0, (double) POWER);


  REAL *A, *B, *C, *D;

  A = (REAL *) malloc(n * n * sizeof(REAL)); //source matrix 
  B = (REAL *) malloc(n * n * sizeof(REAL)); //source matrix
  C = (REAL *) malloc(n * n * sizeof(REAL)); //result matrix sequential
  D = (REAL *) malloc(n * n * sizeof(REAL)); //result matrix parallel

  char const *deps[] = { "system" };
  hclib::launch(deps, 1, [&]() {
    /* initialize */
    init_rm(A, n);
    init_rm(B, n);
    zero(C, n);


    // print_matrix(A,n);
    // print_matrix(B,n);
    // print_matrix(C,n);
    

    long start = hclib_current_time_ms();
    
    iter_matmul(A,B,C,n);
    
    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    
    
    //print_matrix(C,n);
    printf("Sequential Matrix Multiplication Time = %.3f\n", dur);


    // reset C do to mat mul in parallel
    //zero(C,n);

    start = hclib_current_time_ms();
    mat_mul_par(A,B,D,n);
    end = hclib_current_time_ms();
    dur = ((double)(end-start))/1000;

    //print_matrix(C,n);
    printf("parallel Matrix Multiplication Time = %.3f\n", dur);

    //compare two results
    compare_matrix(C,D,n);

    /* release memory */
    free(A);
    free(B);
    free(C);
  });

  return 0;
}
