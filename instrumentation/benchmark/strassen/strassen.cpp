#include "stdio.h"
#include "hclib_cpp.h"
#include "strassen.h"


static void OptimizedStrassenMultiply_par(double *C, double *A, double *B,
    unsigned MatrixSize, unsigned RowWidthC, unsigned RowWidthA,
    unsigned RowWidthB, unsigned int Depth, unsigned int cutoff_depth,
    unsigned cutoff_size)
{
#ifdef RACE_DETECTION
  ds_hclib_ready(true);
#endif
  
  unsigned QuadrantSize = MatrixSize >> 1; /* MatixSize / 2 */
  unsigned QuadrantSizeInBytes = sizeof(double) * QuadrantSize * QuadrantSize;
  unsigned Column, Row;


  /************************************************************************
   ** For each matrix A, B, and C, we'll want pointers to each quandrant
   ** in the matrix. These quandrants will be addressed as follows:
   **  --        --
   **  | A    A12 |
   **  |          |
   **  | A21  A22 |
   **  --        --
   ************************************************************************/
  double *A12, *B12, *C12,
         *A21, *B21, *C21, 
         *A22, *B22, *C22;

  double *S1,*S2,*S3,*S4,*S5,*S6,*S7,*S8,*M2,*M5,*T1sMULT;
  #define NumberOfVariables 11

  char *Heap;
  void *StartHeap;

  hclib::promise_t<void>  *p_a12, *p_b12, *p_c12, 
                          *p_a21, *p_b21, *p_c21,
                          *p_a22, *p_b22, *p_c22;

  hclib::promise_t<void>  *p_s1, *p_s2, *p_s3, *p_s4, *p_s5, *p_s6, *p_s7, *p_s8, *p_m2, *p_m5, *p_t1smult;

  hclib::promise_t<void>  *p_a, *p_b, *p_c;
  p_a = new hclib::promise_t<void>();
  p_b = new hclib::promise_t<void>();
  p_c = new hclib::promise_t<void>();

  if (MatrixSize <= cutoff_size) {
    MultiplyByDivideAndConquer(C, A, B, MatrixSize, RowWidthC, RowWidthA, RowWidthB, 0);
    return;
  }

  /* Initialize quandrant matrices */
  A12 = A + QuadrantSize;
    p_a12 = new hclib::promise_t<void>();

  B12 = B + QuadrantSize;
    p_b12 = new hclib::promise_t<void>();

  C12 = C + QuadrantSize;
    p_c12 = new hclib::promise_t<void>();

  A21 = A + (RowWidthA * QuadrantSize);
    p_a21 = new hclib::promise_t<void>();
    p_a21->put();

  B21 = B + (RowWidthB * QuadrantSize);
    p_b21 = new hclib::promise_t<void>();

  C21 = C + (RowWidthC * QuadrantSize);
    p_c21 = new hclib::promise_t<void>();

  A22 = A21 + QuadrantSize;
    p_a22 = new hclib::promise_t<void>();
    p_a22->put();

  B22 = B21 + QuadrantSize;
    p_b22 = new hclib::promise_t<void>();

  C22 = C21 + QuadrantSize;
    p_c22 = new hclib::promise_t<void>();

  /* Allocate Heap Space Here */
  StartHeap = Heap = (char*) malloc(QuadrantSizeInBytes * NumberOfVariables);

  /* Distribute the heap space over the variables */
  S1 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s1 = new hclib::promise_t<void>();

  S2 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s2 = new hclib::promise_t<void>();

  S3 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s3 = new hclib::promise_t<void>();

  S4 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s4 = new hclib::promise_t<void>();

  S5 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s5 = new hclib::promise_t<void>();

  S6 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s6 = new hclib::promise_t<void>();

  S7 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s7 = new hclib::promise_t<void>();

  S8 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_s8 = new hclib::promise_t<void>();

  M2 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_m2 = new hclib::promise_t<void>();

  M5 = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_m5 = new hclib::promise_t<void>();

  T1sMULT = (double*) Heap; Heap += QuadrantSizeInBytes;
    p_t1smult = new hclib::promise_t<void>();

  if (Depth < cutoff_depth)
  {
#ifdef RACE_DETECTION
      ds_hclib_ready(false);
#endif
      hclib::finish([&](){

        //#pragma omp task depend(in: A21, A22) depend(out: S1) private(Row, Column)
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
            ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
            ds_promise_task(true);
#endif

            for (Row = 0; Row < QuadrantSize; Row++)
                for (Column = 0; Column < QuadrantSize; Column++)
                S1[Row * QuadrantSize + Column] = A21[RowWidthA * Row + Column] + A22[RowWidthA * Row + Column];

            p_s1->put();
        });


        //#pragma omp task depend(in: S1, A) depend(out: S2) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          p_s1->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S2[Row * QuadrantSize + Column] = S1[Row * QuadrantSize + Column] - A[RowWidthA * Row + Column];

          p_s2->put();
        });

        //#pragma omp task depend(in: A12, S2) depend(out: S4) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          p_s2->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S4[Row * QuadrantSize + Column] = A12[Row * RowWidthA + Column] - S2[QuadrantSize * Row + Column];

          p_s4->put();
        });

        //#pragma omp task depend(in: B12, B) depend(out: S5) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S5[Row * QuadrantSize + Column] = B12[Row * RowWidthB + Column] - B[Row * RowWidthB + Column];

          p_s5->put();
        });

        //#pragma omp task depend(in: B22, S5) depend(out: S6) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          p_s5->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S6[Row * QuadrantSize + Column] = B22[Row * RowWidthB + Column] - S5[Row * QuadrantSize + Column];

          p_s6->put();
        });

        //#pragma omp task depend(in: S6, B21) depend(out: S8) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          p_s6->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S8[Row * QuadrantSize + Column] = S6[Row * QuadrantSize + Column] - B21[Row * RowWidthB + Column];

          p_s8->put();
        });

        //#pragma omp task depend(in: A, A21) depend(out: S3) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S3[Row * QuadrantSize + Column] = A[RowWidthA * Row + Column] - A21[RowWidthA * Row + Column];

          p_s3->put();
        });

        //#pragma omp task depend(in: B22, B12) depend(out: S7) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column++)
              S7[Row * QuadrantSize + Column] = B22[Row * RowWidthB + Column] - B12[Row * RowWidthB + Column];

          p_s7->put();
        });

        /* M2 = A x B */
        //#pragma omp task depend(in: A, B) depend(out: M2)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            OptimizedStrassenMultiply_par(M2, A, B, QuadrantSize, QuadrantSize, RowWidthA, RowWidthB, Depth+1, cutoff_depth, cutoff_size);
            p_m2->put();
        });

        /* M5 = S1 * S5 */
        //#pragma omp task untied depend(in: S1, S5) depend(out: M5)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            p_s1->get_future()->wait();
            p_s5->get_future()->wait();

            OptimizedStrassenMultiply_par(M5, S1, S5, QuadrantSize, QuadrantSize, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);

            p_m5->put();
        });

        /* Step 1 of T1 = S2 x S6 + M2 */
        //#pragma omp task untied depend(in: S2, S6) depend(out: T1sMULT)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            p_s2->get_future()->wait();
            p_s6->get_future()->wait();

            OptimizedStrassenMultiply_par(T1sMULT, S2, S6,  QuadrantSize, QuadrantSize, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);

            p_t1smult->put();
        });

        /* Step 1 of T2 = T1 + S3 x S7 */
        //#pragma omp task untied depend(in: S3, S7) depend(out: C22)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            p_s3->get_future()->wait();
            p_s7->get_future()->wait();

            OptimizedStrassenMultiply_par(C22, S3, S7, QuadrantSize, RowWidthC /*FIXME*/, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);

            p_c22->put();
        });

        /* Step 1 of C = M2 + A12 * B21 */
        //#pragma omp task untied depend(in: A12, B21) depend(out: C)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            OptimizedStrassenMultiply_par(C, A12, B21, QuadrantSize, RowWidthC, RowWidthA, RowWidthB, Depth+1, cutoff_depth, cutoff_size);

            p_c->put();
        });
            
        /* Step 1 of C12 = S4 x B22 + T1 + M5 */
        //#pragma omp task untied depend(in: S4, B22) depend(out: C12)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            p_s4->get_future()->wait();

            OptimizedStrassenMultiply_par(C12, S4, B22, QuadrantSize, RowWidthC, QuadrantSize, RowWidthB, Depth+1, cutoff_depth, cutoff_size);

            p_c12->put();
        });

        /* Step 1 of C21 = T2 - A22 * S8 */
        //#pragma omp task untied depend(in: A22, S8) depend(out: C21)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&](){
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

            p_s8->get_future()->wait();

            OptimizedStrassenMultiply_par(C21, A22, S8, QuadrantSize, RowWidthC, RowWidthA, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);

            p_c21->put();
        });

        //#pragma omp task depend(inout: C) depend(in: M2) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          // this is the last time C being used, no need to reset
          p_c->get_future()->wait();
          p_m2->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column += 1)
              C[RowWidthC * Row + Column] += M2[Row * QuadrantSize + Column];

        });

        //#pragma omp task depend(inout: C12) depend(in: M5, T1sMULT, M2) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          // this is the last time C12 being used, no need to reset
          p_c12->get_future()->wait();
          p_m5->get_future()->wait();
          p_t1smult->get_future()->wait();
          p_m2->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column += 1)
              C12[RowWidthC * Row + Column] += M5[Row * QuadrantSize + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];

        });

        //#pragma omp task depend(inout: C21) depend(in: C22, T1sMULT, M2) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          // this is the last time C21 being used, no need to reset
          p_c21->get_future()->wait();
          p_c22->get_future()->wait();
          p_t1smult->get_future()->wait();
          p_m2->get_future()->wait();

          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column += 1)
              C21[RowWidthC * Row + Column] = -C21[RowWidthC * Row + Column] + C22[RowWidthC * Row + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];
        });

        //#pragma omp task depend(inout: C22) depend(in: M5, T1sMULT, M2) private(Row, Column)
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::async([&,Row,Column]() mutable{
#ifdef RACE_DETECTION
          ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
          ds_promise_task(true);
#endif

          p_c22->get_future()->wait();
          p_m5->get_future()->wait();
          p_t1smult->get_future()->wait();
          p_m2->get_future()->wait();
          
          for (Row = 0; Row < QuadrantSize; Row++)
              for (Column = 0; Column < QuadrantSize; Column += 1)
              C22[RowWidthC * Row + Column] += M5[Row * QuadrantSize + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];
        });
        //#pragma omp taskwait
    });
  }
  else // the following are all sequential
  {
    for (Row = 0; Row < QuadrantSize; Row++)
      for (Column = 0; Column < QuadrantSize; Column++) {
        S1[Row * QuadrantSize + Column] = A21[RowWidthA * Row + Column] + A22[RowWidthA * Row + Column];
        S2[Row * QuadrantSize + Column] = S1[Row * QuadrantSize + Column] - A[RowWidthA * Row + Column];
        S4[Row * QuadrantSize + Column] = A12[Row * RowWidthA + Column] - S2[QuadrantSize * Row + Column];
        S5[Row * QuadrantSize + Column] = B12[Row * RowWidthB + Column] - B[Row * RowWidthB + Column];
        S6[Row * QuadrantSize + Column] = B22[Row * RowWidthB + Column] - S5[Row * QuadrantSize + Column];
        S8[Row * QuadrantSize + Column] = S6[Row * QuadrantSize + Column] - B21[Row * RowWidthB + Column];
        S3[Row * QuadrantSize + Column] = A[RowWidthA * Row + Column] - A21[RowWidthA * Row + Column];
        S7[Row * QuadrantSize + Column] = B22[Row * RowWidthB + Column] - B12[Row * RowWidthB + Column];
      }
    /* M2 = A x B */
    OptimizedStrassenMultiply_par(M2, A, B, QuadrantSize, QuadrantSize, RowWidthA, RowWidthB, Depth+1, cutoff_depth, cutoff_size);
    /* M5 = S1 * S5 */
    OptimizedStrassenMultiply_par(M5, S1, S5, QuadrantSize, QuadrantSize, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);
    /* Step 1 of T1 = S2 x S6 + M2 */
    OptimizedStrassenMultiply_par(T1sMULT, S2, S6,  QuadrantSize, QuadrantSize, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);
    /* Step 1 of T2 = T1 + S3 x S7 */
    OptimizedStrassenMultiply_par(C22, S3, S7, QuadrantSize, RowWidthC /*FIXME*/, QuadrantSize, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);
    /* Step 1 of C = M2 + A12 * B21 */
    OptimizedStrassenMultiply_par(C, A12, B21, QuadrantSize, RowWidthC, RowWidthA, RowWidthB, Depth+1, cutoff_depth, cutoff_size);
    /* Step 1 of C12 = S4 x B22 + T1 + M5 */
    OptimizedStrassenMultiply_par(C12, S4, B22, QuadrantSize, RowWidthC, QuadrantSize, RowWidthB, Depth+1, cutoff_depth, cutoff_size);
    /* Step 1 of C21 = T2 - A22 * S8 */
    OptimizedStrassenMultiply_par(C21, A22, S8, QuadrantSize, RowWidthC, RowWidthA, QuadrantSize, Depth+1, cutoff_depth, cutoff_size);

    for (Row = 0; Row < QuadrantSize; Row++) {
      for (Column = 0; Column < QuadrantSize; Column += 1) {
        C[RowWidthC * Row + Column] += M2[Row * QuadrantSize + Column];
        C12[RowWidthC * Row + Column] += M5[Row * QuadrantSize + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];
        C21[RowWidthC * Row + Column] = -C21[RowWidthC * Row + Column] + C22[RowWidthC * Row + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];
        C22[RowWidthC * Row + Column] += M5[Row * QuadrantSize + Column] + T1sMULT[Row * QuadrantSize + Column] + M2[Row * QuadrantSize + Column];
      }
    }
  }
  free(StartHeap);
}


void strassen_main_par(double *A, double *B, double *C, int n, unsigned int cutoff_size, unsigned int cutoff_depth)
{
  OptimizedStrassenMultiply_par(C, A, B, n, n, n, n, 1, cutoff_depth, cutoff_size);
}



static void FastNaiveMatrixMultiply(REAL *C, REAL *A, REAL *B, unsigned MatrixSize,
     unsigned RowWidthC, unsigned RowWidthA, unsigned RowWidthB)
{
  /* Assumes size of real is 8 bytes */
  PTR RowWidthBInBytes = RowWidthB  << 3;
  PTR RowWidthAInBytes = RowWidthA << 3;
  PTR MatrixWidthInBytes = MatrixSize << 3;
  PTR RowIncrementC = ( RowWidthC - MatrixSize) << 3;
  unsigned Horizontal, Vertical;

  REAL *ARowStart = A;
  for (Vertical = 0; Vertical < MatrixSize; Vertical++) {
    for (Horizontal = 0; Horizontal < MatrixSize; Horizontal += 8) {
      REAL *BColumnStart = B + Horizontal;
      REAL FirstARowValue = *ARowStart++;

      REAL Sum0 = FirstARowValue * (*BColumnStart);
      REAL Sum1 = FirstARowValue * (*(BColumnStart+1));
      REAL Sum2 = FirstARowValue * (*(BColumnStart+2));
      REAL Sum3 = FirstARowValue * (*(BColumnStart+3));
      REAL Sum4 = FirstARowValue * (*(BColumnStart+4));
      REAL Sum5 = FirstARowValue * (*(BColumnStart+5));
      REAL Sum6 = FirstARowValue * (*(BColumnStart+6));
      REAL Sum7 = FirstARowValue * (*(BColumnStart+7));

      unsigned Products;
      for (Products = 1; Products < MatrixSize; Products++) {
        REAL ARowValue = *ARowStart++;
        BColumnStart = (REAL*) (((PTR) BColumnStart) + RowWidthBInBytes);
        Sum0 += ARowValue * (*BColumnStart);
        Sum1 += ARowValue * (*(BColumnStart+1));
        Sum2 += ARowValue * (*(BColumnStart+2));
        Sum3 += ARowValue * (*(BColumnStart+3));
        Sum4 += ARowValue * (*(BColumnStart+4));
        Sum5 += ARowValue * (*(BColumnStart+5));
        Sum6 += ARowValue * (*(BColumnStart+6));
        Sum7 += ARowValue * (*(BColumnStart+7));
      }
      ARowStart = (REAL*) ( ((PTR) ARowStart) - MatrixWidthInBytes);

      *(C) = Sum0;
      *(C+1) = Sum1;
      *(C+2) = Sum2;
      *(C+3) = Sum3;
      *(C+4) = Sum4;
      *(C+5) = Sum5;
      *(C+6) = Sum6;
      *(C+7) = Sum7;
      C+=8;
    }
    ARowStart = (REAL*) ( ((PTR) ARowStart) + RowWidthAInBytes );
    C = (REAL*) ( ((PTR) C) + RowIncrementC );
  }
}


static void FastAdditiveNaiveMatrixMultiply(REAL *C, REAL *A, REAL *B, unsigned MatrixSize,
     unsigned RowWidthC, unsigned RowWidthA, unsigned RowWidthB)
{
  /* Assumes size of real is 8 bytes */
  PTR RowWidthBInBytes = RowWidthB  << 3;
  PTR RowWidthAInBytes = RowWidthA << 3;
  PTR MatrixWidthInBytes = MatrixSize << 3;
  PTR RowIncrementC = ( RowWidthC - MatrixSize) << 3;
  unsigned Horizontal, Vertical;

  REAL *ARowStart = A;
  for (Vertical = 0; Vertical < MatrixSize; Vertical++) {
    for (Horizontal = 0; Horizontal < MatrixSize; Horizontal += 8) {
      REAL *BColumnStart = B + Horizontal;

      REAL Sum0 = *C;
      REAL Sum1 = *(C+1);
      REAL Sum2 = *(C+2);
      REAL Sum3 = *(C+3);
      REAL Sum4 = *(C+4);
      REAL Sum5 = *(C+5);
      REAL Sum6 = *(C+6);
      REAL Sum7 = *(C+7);

      unsigned Products;
      for (Products = 0; Products < MatrixSize; Products++) {
        REAL ARowValue = *ARowStart++;

        Sum0 += ARowValue * (*BColumnStart);
        Sum1 += ARowValue * (*(BColumnStart+1));
        Sum2 += ARowValue * (*(BColumnStart+2));
        Sum3 += ARowValue * (*(BColumnStart+3));
        Sum4 += ARowValue * (*(BColumnStart+4));
        Sum5 += ARowValue * (*(BColumnStart+5));
        Sum6 += ARowValue * (*(BColumnStart+6));
        Sum7 += ARowValue * (*(BColumnStart+7));

	    BColumnStart = (REAL*) (((PTR) BColumnStart) + RowWidthBInBytes);

      }
      ARowStart = (REAL*) ( ((PTR) ARowStart) - MatrixWidthInBytes);

      *(C) = Sum0;
      *(C+1) = Sum1;
      *(C+2) = Sum2;
      *(C+3) = Sum3;
      *(C+4) = Sum4;
      *(C+5) = Sum5;
      *(C+6) = Sum6;
      *(C+7) = Sum7;
      C+=8;
    }

    ARowStart = (REAL*) ( ((PTR) ARowStart) + RowWidthAInBytes );
    C = (REAL*) ( ((PTR) C) + RowIncrementC );
  }
}


void MultiplyByDivideAndConquer(REAL *C, REAL *A, REAL *B,
				     unsigned MatrixSize,
				     unsigned RowWidthC,
				     unsigned RowWidthA,
				     unsigned RowWidthB,
				     int AdditiveMode
				    )
{
  REAL  *A01, *A10, *A11, *B01, *B10, *B11, *C01, *C10, *C11;
  unsigned QuadrantSize = MatrixSize >> 1;

  /* partition the matrix */
  A01 = A + QuadrantSize;
  A10 = A + RowWidthA * QuadrantSize;
  A11 = A10 + QuadrantSize;

  B01 = B + QuadrantSize;
  B10 = B + RowWidthB * QuadrantSize;
  B11 = B10 + QuadrantSize;

  C01 = C + QuadrantSize;
  C10 = C + RowWidthC * QuadrantSize;
  C11 = C10 + QuadrantSize;

  if (QuadrantSize > SizeAtWhichNaiveAlgorithmIsMoreEfficient) {

    MultiplyByDivideAndConquer(C, A, B, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     AdditiveMode);

    MultiplyByDivideAndConquer(C01, A, B01, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     AdditiveMode);

    MultiplyByDivideAndConquer(C11, A10, B01, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     AdditiveMode);

    MultiplyByDivideAndConquer(C10, A10, B, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     AdditiveMode);

    MultiplyByDivideAndConquer(C, A01, B10, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     1);

    MultiplyByDivideAndConquer(C01, A01, B11, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     1);

    MultiplyByDivideAndConquer(C11, A11, B11, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     1);

    MultiplyByDivideAndConquer(C10, A11, B10, QuadrantSize,
				     RowWidthC, RowWidthA, RowWidthB,
				     1);

  } else {

    if (AdditiveMode) {
      FastAdditiveNaiveMatrixMultiply(C, A, B, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastAdditiveNaiveMatrixMultiply(C01, A, B01, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastAdditiveNaiveMatrixMultiply(C11, A10, B01, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastAdditiveNaiveMatrixMultiply(C10, A10, B, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

    } else {

      FastNaiveMatrixMultiply(C, A, B, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastNaiveMatrixMultiply(C01, A, B01, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastNaiveMatrixMultiply(C11, A10, B01, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);

      FastNaiveMatrixMultiply(C10, A10, B, QuadrantSize,
			      RowWidthC, RowWidthA, RowWidthB);
    }

    FastAdditiveNaiveMatrixMultiply(C, A01, B10, QuadrantSize,
				    RowWidthC, RowWidthA, RowWidthB);

    FastAdditiveNaiveMatrixMultiply(C01, A01, B11, QuadrantSize,
				    RowWidthC, RowWidthA, RowWidthB);

    FastAdditiveNaiveMatrixMultiply(C11, A11, B11, QuadrantSize,
				    RowWidthC, RowWidthA, RowWidthB);

    FastAdditiveNaiveMatrixMultiply(C10, A11, B10, QuadrantSize,
				    RowWidthC, RowWidthA, RowWidthB);
  }
  return;
}


static void init_matrix(int n, REAL *A, int an, unsigned int bs)
{
     int i, j;

     for (i = 0; i < n; i+=bs)
        for (j = 0; j < n; j+=bs)
        {
          {
            unsigned int seed = rand();
            int ii, jj;
            for (ii = i; ii < i+bs; ++ii)
                 for (jj = 0; jj < j+bs; ++jj)
                      ELEM(A, an, ii, jj) = ((double) rand_r(&seed) / RAND_MAX);
          }
        }
}


static int compare_matrix(int n, REAL *A, int an, REAL *B, int bn)
{
    int i, j;
    REAL c;

    for (i = 0; i < n; ++i)
        for (j = 0; j < n; ++j) {
            /* compute the relative error c */
            c = ELEM(A, an, i, j) - ELEM(B, bn, i, j);
            if (c < 0.0)
                c = -c;
            c = c / ELEM(A, an, i, j);
            if (c > EPSILON)
                return 0;
        }

    return 1;
}


void matrix_multiply(double* A, double* B, double* C, int matrix_size)
{
int i, j, k;
for(i=0; i<matrix_size; i++)
    for(j=0; j<matrix_size; j++)
    {
	double res = 0;
        for(k=0; k<matrix_size; k++)
	    res += A[i * matrix_size + k] * B[k * matrix_size + j];
        C[i * matrix_size + j] = res;
    }
}


void run(int ms, int cs, int cd)
{
    double *A, *B, *C;
    int matrix_size = ms;
    int cutoff_size = cs;
    int cutoff_depth = cd;
    if (matrix_size <= 0) {
        matrix_size = 256;
    }
    if (cutoff_size <= 0) {
        cutoff_size = 64;
    }
    if (cutoff_depth <= 0) {
        cutoff_depth = 4;
    }

    A = (double *) malloc (matrix_size * matrix_size * sizeof(double));
    B = (double *) malloc (matrix_size * matrix_size * sizeof(double));
    C = (double *) malloc (matrix_size * matrix_size * sizeof(double));

    init_matrix(matrix_size,A,matrix_size, matrix_size/8);
    init_matrix(matrix_size,B,matrix_size, matrix_size/8);

    /// KERNEL INTENSIVE COMPUTATION
    long start = hclib_current_time_ms();

#ifdef RACE_DETECTION
    ds_hclib_ready(true);
#endif
    strassen_main_par(C, A, B, matrix_size, cutoff_size, cutoff_depth);
#ifdef RACE_DETECTION
    ds_hclib_ready(false);
#endif

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    printf("strassen time = %f\n",dur);

    #ifdef CHECK 
        double *D = (double *) malloc (matrix_size * matrix_size * sizeof(double));
        matrix_multiply(A, B, D, matrix_size);
        params->succeed = compare_matrix(matrix_size, C, matrix_size, D, matrix_size);
        free(D);
    #endif

    free(A);
    free(B);
    free(C);
    return;
}

int main (int argc, char ** argv) {
    printf("strassen benchmark \n");
    int matrix_size = 4096;
    int cutoff_size = 64;
    int cutoff_depth = 5;

    if(argc == 4){
        matrix_size = atoi(argv[1]);
        cutoff_size = atoi(argv[2]);
        cutoff_depth = atoi(argv[3]);
    }

    printf("matrix size: %d     cutoff_size: %d      cutoff_depth: %d \n",matrix_size,cutoff_size,cutoff_depth);

    char const *deps[] = { "system" }; 
    hclib::launch(deps, 1, [&]() {
        long start = hclib_current_time_ms();
        
        run(matrix_size, cutoff_size, cutoff_depth);

        long end = hclib_current_time_ms();
        double dur = ((double)(end-start))/1000;
        printf("Run Time = %f\n",dur);
    });

    return 0;
}
