#include "stdio.h"
#include "hclib_cpp.h"
#include "sparselu.h"
#include <vector>

void sparselu_par_call_dep(float **BENCH, int matrix_size, int submatrix_size)
{
    ds_promise_task(true);
    int ii, jj, kk;

    int array_size = matrix_size*matrix_size;
    hclib::promise_t<void>* promise_array[array_size];
    std::vector<hclib::promise_t<void>*> task_vector;

    for (kk=0; kk<matrix_size; kk++){

        // at the beginning of for loop, initialize promise array
        ds_hclib_ready(false);
        for(int index=0; index < array_size; index++){
            hclib::promise_t<void> *p = new hclib::promise_t<void>();
            promise_array[index] = p;
        }

        // #pragma omp task firstprivate(kk) shared(BENCH) depend(inout: BENCH[kk*matrix_size+kk:submatrix_size*submatrix_size])
        ds_hclib_ready(false);
        task_vector.push_back(new hclib::promise_t<void>());

        hclib::async([=, &BENCH, &promise_array, &task_vector]() {
            ds_hclib_ready(true);
            ds_promise_task(true);

            lu0(BENCH[kk*matrix_size+kk], submatrix_size);

            promise_array[kk*matrix_size+kk]->put();
            task_vector.at(0)->put();
        });
        
        for (jj=kk+1; jj<matrix_size; jj++)
            if (BENCH[kk*matrix_size+jj] != NULL)
            {
                // #pragma omp task firstprivate(kk, jj) shared(BENCH) \
                // depend(in: BENCH[kk*matrix_size+kk:submatrix_size*submatrix_size]) \
                // depend(inout: BENCH[kk*matrix_size+jj:submatrix_size*submatrix_size])

                ds_hclib_ready(false);
                task_vector.push_back(new hclib::promise_t<void>());
                int task_index = task_vector.size() - 1;
                hclib::async([=, &BENCH, &promise_array, &task_vector]() {
                    ds_hclib_ready(true);
                    ds_promise_task(true);
                    promise_array[kk*matrix_size+kk]->get_future()->wait();

                    fwd(BENCH[kk*matrix_size+kk], BENCH[kk*matrix_size+jj], submatrix_size);

                    promise_array[kk*matrix_size+jj]->put();
                    task_vector.at(task_index)->put();
                });
                
            }
        for (ii=kk+1; ii<matrix_size; ii++)
            if (BENCH[ii*matrix_size+kk] != NULL)
            {
                // #pragma omp task firstprivate(kk, ii) shared(BENCH) \
                // depend(in: BENCH[kk*matrix_size+kk:submatrix_size*submatrix_size]) \
                // depend(inout: BENCH[ii*matrix_size+kk:submatrix_size*submatrix_size])

                ds_hclib_ready(false);
                task_vector.push_back(new hclib::promise_t<void>());
                int task_index = task_vector.size() - 1;
                hclib::async([=, &BENCH, &promise_array]() {
                    ds_hclib_ready(true);
                    ds_promise_task(true);
                    promise_array[kk*matrix_size+kk]->get_future()->wait();

                    bdiv (BENCH[kk*matrix_size+kk], BENCH[ii*matrix_size+kk], submatrix_size);

                    promise_array[ii*matrix_size+kk]->put();
                    task_vector.at(task_index)->put();
                });
            }

        for (ii=kk+1; ii<matrix_size; ii++)
            if (BENCH[ii*matrix_size+kk] != NULL)
                for (jj=kk+1; jj<matrix_size; jj++)
                    if (BENCH[kk*matrix_size+jj] != NULL)
                    {
                        if (BENCH[ii*matrix_size+jj]==NULL) BENCH[ii*matrix_size+jj] = allocate_clean_block(submatrix_size);
                        // #pragma omp task firstprivate(kk, jj, ii) shared(BENCH) \
                        // depend(in: BENCH[ii*matrix_size+kk:submatrix_size*submatrix_size], BENCH[kk*matrix_size+jj:submatrix_size*submatrix_size]) \
                        // depend(inout: BENCH[ii*matrix_size+jj:submatrix_size*submatrix_size])

                        ds_hclib_ready(false);
                        task_vector.push_back(new hclib::promise_t<void>());
                        int task_index = task_vector.size() - 1;
                        hclib::async([=, &BENCH, &promise_array]() {
                            ds_hclib_ready(true);
                            ds_promise_task(true);
                            promise_array[ii*matrix_size+kk]->get_future()->wait();
                            promise_array[kk*matrix_size+jj]->get_future()->wait();

                            bmod(BENCH[ii*matrix_size+kk], BENCH[kk*matrix_size+jj], BENCH[ii*matrix_size+jj], submatrix_size);

                            promise_array[ii*matrix_size+jj]->put();
                            task_vector.at(task_index)->put();
                        });
                    }

        // at the end of for loop, reset all promise
        ds_hclib_ready(false);
        for(int index=0; index < array_size; index++){
            delete promise_array[index];
        }

        for(auto p = task_vector.begin(); p != task_vector.end(); p++){
            (*p)->get_future()->wait();
        }

        task_vector.clear();
    } // end of for loop
}


void sparselu_par_call(float **BENCH, int matrix_size, int submatrix_size)
{
    ds_promise_task(true);
    int ii, jj, kk;
    std::vector<hclib::promise_t<void>*> promise_vector;

    for (kk=0; kk<matrix_size; kk++)
    {
        lu0(BENCH[kk*matrix_size+kk], submatrix_size);
        for (jj=kk+1; jj<matrix_size; jj++)
            if (BENCH[kk*matrix_size+jj] != NULL)
            {
                ds_hclib_ready(false);
                promise_vector.push_back(new hclib::promise_t<void>());
                ds_hclib_ready(true);

                int index = promise_vector.size() - 1;

                ds_hclib_ready(false);
                hclib::async([=, &BENCH, &promise_vector]() {
                    ds_hclib_ready(true);
                    ds_promise_task(true);
                    fwd(BENCH[kk*matrix_size+kk], BENCH[kk*matrix_size+jj], submatrix_size);
                    promise_vector.at(index)->put();
                });
            }
            
        for (ii=kk+1; ii<matrix_size; ii++)
            if (BENCH[ii*matrix_size+kk] != NULL)
            {
                ds_hclib_ready(false);
                promise_vector.push_back(new hclib::promise_t<void>());
                ds_hclib_ready(true);

                int index = promise_vector.size() - 1;

                ds_hclib_ready(false);
                hclib::async([=, &BENCH, &promise_vector]() {
                    ds_hclib_ready(true);
                    ds_promise_task(true);
                    bdiv (BENCH[kk*matrix_size+kk], BENCH[ii*matrix_size+kk], submatrix_size);
                    promise_vector.at(index)->put();
                });
            }

        for(auto pp = promise_vector.begin(); pp != promise_vector.end(); pp++){
            (*pp)->get_future()->wait();
        }
        promise_vector.empty();


        for (ii=kk+1; ii<matrix_size; ii++)
            if (BENCH[ii*matrix_size+kk] != NULL)
                for (jj=kk+1; jj<matrix_size; jj++)
                    if (BENCH[kk*matrix_size+jj] != NULL)
                    {
                        ds_hclib_ready(false);
                        promise_vector.push_back(new hclib::promise_t<void>());
                        ds_hclib_ready(true);

                        int index = promise_vector.size() - 1;

                        ds_hclib_ready(false);
                        hclib::async([=, &BENCH, &promise_vector](){
                            ds_hclib_ready(true);
                            ds_promise_task(true);
                            if (BENCH[ii*matrix_size+jj]==NULL) BENCH[ii*matrix_size+jj] = allocate_clean_block(submatrix_size);
                            bmod(BENCH[ii*matrix_size+kk], BENCH[kk*matrix_size+jj], BENCH[ii*matrix_size+jj], submatrix_size);
                            promise_vector.at(index)->put();
                        });
                    }

        for(auto pp = promise_vector.begin(); pp != promise_vector.end(); pp++){
            (*pp)->get_future()->wait();
        }
        promise_vector.empty();
    }
}


static void genmat (float *M[], int matrix_size, int submatrix_size)
{
    int null_entry, init_val, i, j, ii, jj;

    init_val = 1325;

    /* generating the structure */
    for (ii=0; ii < matrix_size; ii++)
    {
        for (jj=0; jj < matrix_size; jj++)
        {
// #pragma omp task shared(M)
// {
            float *p;
            /* computing null entries */
            null_entry=0;
            if ((ii<jj) && (ii%3 !=0)) null_entry = 1;
            if ((ii>jj) && (jj%3 !=0)) null_entry = 1;
            if (ii%2==1) null_entry = 1;
            if (jj%2==1) null_entry = 1;
            if (ii==jj) null_entry = 0;
            if (ii==jj-1) null_entry = 0;
            if (ii-1 == jj) null_entry = 0;
            /* allocating matrix */
            if (null_entry == 0){
                M[ii*matrix_size+jj] = (float *) malloc(submatrix_size*submatrix_size*sizeof(float));
                if (M[ii*matrix_size+jj] == NULL)
                    exit(101);
                /* initializing matrix */
                p = M[ii*matrix_size+jj];
                for (i = 0; i < submatrix_size; i++)
                {
                    for (j = 0; j < submatrix_size; j++)
                    {
                        init_val = (3125 * init_val) % 65536;
                        (*p) = (float)((init_val - 32768.0) / 16384.0);
                        p++;
                    }
                }
            }
            else
            {
                M[ii*matrix_size+jj] = NULL;
            }
// }
        }
    }
// #pragma omp taskwait
}


float * allocate_clean_block(int submatrix_size)
{
    int i,j;
    float *p, *q;

    p = (float *) malloc(submatrix_size*submatrix_size*sizeof(float));
    q=p;
    if (p!=NULL){
        for (i = 0; i < submatrix_size; i++)
            for (j = 0; j < submatrix_size; j++){(*p)=0.0; p++;}

    }
    else
        exit (101);
    return (q);
}


void lu0(float *diag, int submatrix_size)
{
    int i, j, k;

    for (k=0; k<submatrix_size; k++)
        for (i=k+1; i<submatrix_size; i++)
        {
            diag[i*submatrix_size+k] = diag[i*submatrix_size+k] / diag[k*submatrix_size+k];
            for (j=k+1; j<submatrix_size; j++)
                diag[i*submatrix_size+j] = diag[i*submatrix_size+j] - diag[i*submatrix_size+k] * diag[k*submatrix_size+j];
        }
}


void bdiv(float *diag, float *row, int submatrix_size)
{
    int i, j, k;
    for (i=0; i<submatrix_size; i++)
        for (k=0; k<submatrix_size; k++)
        {
            row[i*submatrix_size+k] = row[i*submatrix_size+k] / diag[k*submatrix_size+k];
            for (j=k+1; j<submatrix_size; j++)
                row[i*submatrix_size+j] = row[i*submatrix_size+j] - row[i*submatrix_size+k]*diag[k*submatrix_size+j];
        }
}


void bmod(float *row, float *col, float *inner, int submatrix_size)
{
    int i, j, k;
    for (i=0; i<submatrix_size; i++)
        for (j=0; j<submatrix_size; j++)
            for (k=0; k<submatrix_size; k++)
                inner[i*submatrix_size+j] = inner[i*submatrix_size+j] - row[i*submatrix_size+k]*col[k*submatrix_size+j];
}


void fwd(float *diag, float *col, int submatrix_size)
{
    int i, j, k;
    for (j=0; j<submatrix_size; j++)
        for (k=0; k<submatrix_size; k++)
            for (i=k+1; i<submatrix_size; i++)
                col[i*submatrix_size+j] = col[i*submatrix_size+j] - diag[i*submatrix_size+k]*col[k*submatrix_size+j];
}


static void sparselu_init (float ***pBENCH, int matrix_size, int submatrix_size)
{
    *pBENCH = (float **) malloc(matrix_size*matrix_size*sizeof(float *));
    genmat(*pBENCH, matrix_size, submatrix_size);
}


static int checkmat (float *M, float *N, int submatrix_size)
{
    int i, j;
    float r_err;

    for (i = 0; i < submatrix_size; i++)
    {
        for (j = 0; j < submatrix_size; j++)
        {
            r_err = M[i*submatrix_size+j] - N[i*submatrix_size+j];
            if ( r_err == 0.0 ) continue;

            if (r_err < 0.0 ) r_err = -r_err;

            if ( M[i*submatrix_size+j] == 0 )
                return 0;
            r_err = r_err / M[i*submatrix_size+j];
            if(r_err > EPSILON)
                return 0;
        }
    }
    return 1;
}


static int sparselu_check(float **BENCH_SEQ, float **BENCH, int matrix_size, int submatrix_size)
{
    int ii,jj,ok=1;

    for (ii=0; ((ii<matrix_size) && ok); ii++)
    {
        for (jj=0; ((jj<matrix_size) && ok); jj++)
        {
            if ((BENCH_SEQ[ii*matrix_size+jj] == NULL) && (BENCH[ii*matrix_size+jj] != NULL)) ok = 0;
            if ((BENCH_SEQ[ii*matrix_size+jj] != NULL) && (BENCH[ii*matrix_size+jj] == NULL)) ok = 0;
            if ((BENCH_SEQ[ii*matrix_size+jj] != NULL) && (BENCH[ii*matrix_size+jj] != NULL))
                ok = checkmat(BENCH_SEQ[ii*matrix_size+jj], BENCH[ii*matrix_size+jj], submatrix_size);
        }
    }
    return ok;
}


void run(int ms, int ss)
{
    float **BENCH;
    int matrix_size = ms;
    if (matrix_size <= 0) {
        matrix_size = 64;
    }
    int submatrix_size = ss;
    if (submatrix_size <= 0) {
        submatrix_size = 64;
    }

    
    sparselu_init(&BENCH, matrix_size, submatrix_size);
    

    /// KERNEL INTENSIVE COMPUTATION
    long start = hclib_current_time_ms();

    ds_hclib_ready(true);
    // sparselu_par_call(BENCH, matrix_size, submatrix_size);
    sparselu_par_call_dep(BENCH, matrix_size, submatrix_size);
    ds_hclib_ready(false);

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    printf("sparselu time %f \n", dur);

    #ifdef CHECK
        float **BENCH_SEQ;
        sparselu_init(&BENCH_SEQ, matrix_size, submatrix_size);
        sparselu_seq_call(BENCH_SEQ, matrix_size, submatrix_size);
    #endif
    
    return;
}

int main (int argc, char ** argv) {
    printf("sparselu benchmark from KASTORS\n");
    int matrix_size = 128;
    int submatrix_size = 32;

    if(argc == 3){
        matrix_size = atoi(argv[1]);
        submatrix_size = atoi(argv[2]);
    }
    
    printf("matrix size is: %d, submatrix size is: %d \n", matrix_size, submatrix_size);

    char const *deps[] = { "system" }; 
    hclib::launch(deps, 1, [&]() {
        long start = hclib_current_time_ms();
        run(matrix_size,submatrix_size);
        long end = hclib_current_time_ms();
        double dur = ((double)(end-start))/1000;
        printf("Run Time = %f\n",dur);
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
        ds_print_check_read_count();
        ds_print_check_write_count();
    });

    return 0;
}