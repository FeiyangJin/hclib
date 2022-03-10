#include <stdio.h>
#include <math.h>
#include <vector>
#include <unordered_map>
#include "hclib_cpp.h"

#define index2d(ny,i,j) (((i)*(ny))+(j))

float r8mat_rms(int nx, int ny, float *a_);
void rhs(int nx, int ny, float *f_, int block_size);
void timestamp(void);
float u_exact(float x, float y);
float uxxyy_exact(float x, float y);

void sweep_seq(int nx, int ny, float dx, float dy, float *f_, int itold, int itnew, float *u_, float *unew_)
{
    int i;
    int it;
    int j;
    float *f = f_;
    float *u = u_;
    float *unew = unew_;
    // float (*f)[nx][ny] = (float (*)[nx][ny])f_;
    // float (*u)[nx][ny] = (float (*)[nx][ny])u_;
    // float (*unew)[nx][ny] = (float (*)[nx][ny])unew_;

    for (it = itold + 1; it <= itnew; it++) {
        for (i = 0; i < nx; i++) {
            for (j = 0; j < ny; j++) {
                u[index2d(ny, i, j)] = unew[index2d(ny, i, j)];
                // (*u)[i][j] = (*unew)[i][j];
            }
        }
        for (i = 0; i < nx; i++) {
            for (j = 0; j < ny; j++) {
                if (i == 0 || j == 0 || i == nx - 1 || j == ny - 1) {
                    unew[index2d(ny, i, j)] = f[index2d(ny, i, j)];
                    // (*unew)[i][j] = (*f)[i][j];
                } else {
                    unew[index2d(ny, i, j)] = 0.25 * (u[index2d(ny, i-1, j)] + u[index2d(ny, i, j+1)] + u[index2d(ny, i, j-1)] + u[index2d(ny, i+1, j)]
                        + f[index2d(ny, i, j)] * dx * dy);
                    // (*unew)[i][j] = 0.25 * ((*u)[i-1][j] + (*u)[i][j+1]
                    //         + (*u)[i][j-1] + (*u)[i+1][j]
                    //         + (*f)[i][j] * dx * dy);
                }
            }
        }
    }
}


void sweep (int nx, int ny, float dx, float dy, float *f_, int itold, int itnew, float *u_, float *unew_, int block_size)
{
    #ifdef RACE_DETECTION
        ds_hclib_ready(true);
    #endif

    int i;
    int it;
    int j;
    float *f = f_;
    float *u = u_;
    float *unew = unew_;

    // TODO: allocate unew_p[], 1-D array of nx promises that are all initialized = ready
    #ifdef RACE_DETECTION
        ds_hclib_ready(false);
    #endif
    hclib::promise_t<void>* promise_unew[nx];
    for(int pi = 0; pi<nx; pi++){

        hclib::promise_t<void> *p = new hclib::promise_t<void>();

        p->put();
        promise_unew[pi] = p;
    }

    hclib::promise_t<void>* promise_u[nx];

    for (it = itold + 1; it <= itnew; it++) {
        #ifdef RACE_DETECTION
            ds_hclib_ready(false);
        #endif
        for(int pi=0; pi < nx; pi++){

            hclib::promise_t<void> *p = new hclib::promise_t<void>();

            promise_u[pi] = p;
        }
        #ifdef RACE_DETECTION
            ds_hclib_ready(true);
        #endif


        for (i = 0; i < nx; i++) {
            #ifdef RACE_DETECTION
                ds_hclib_ready(false);
            #endif
            hclib::async([i, nx, ny, &u, &unew, &promise_unew, &promise_u]() mutable{
                #ifdef RACE_DETECTION
                    ds_hclib_ready(true);
                #endif
                if(i > 0){
                    promise_unew[i-1]->get_future()->wait();
                }
                promise_unew[i]->get_future()->wait();
                if(i < nx - 1){
                    promise_unew[i+1]->get_future()->wait();
                }


                for (int ja = 0; ja < ny; ja++) {
                    u[index2d(ny,i,ja)] = unew[index2d(ny,i,ja)];
                }
                // #ifdef RACE_DETECTION
                //     ds_hclib_ready(false);
                // #endif

                promise_u[i]->put();
                // delete promise_unew[i];
                // promise_unew[i] = new hclib::promise_t<void>();
            }); // end of async
        }

        #ifdef RACE_DETECTION
            ds_hclib_ready(false);
        #endif
        for(i = 0; i < nx; i++){
            delete promise_unew[i];
            promise_unew[i] = new hclib::promise_t<void>();
        }

        for (i = 0; i < nx; i++) {
            #ifdef RACE_DETECTION
                ds_hclib_ready(false);
            #endif
            hclib::async([i, nx, ny, dx, dy, f, &u, &unew, &promise_u, &promise_unew]() mutable{
                #ifdef RACE_DETECTION
                    ds_hclib_ready(true);
                #endif
                if(i > 0){
                    promise_u[i-1]->get_future()->wait();
                }
                promise_u[i]->get_future()->wait();
                if(i < nx - 1){
                    promise_u[i+1]->get_future()->wait();
                }

                #ifdef RACE_DETECTION
                    ds_hclib_ready(false);

                    // try to reduce overhead
                    for (int k=0; k<ny; k++){
                        // 0. calculating index
                        int index = index2d(ny, i, k);
                        int* p = ((int*)&unew[index]);

                        // 1. access unew
                        ds_hclib_ready(true);
                        asap_check_write(p, 4);
                        ds_hclib_ready(false);

                        // 2. access f
                        int* p2 = ((int*)&f[index]);

                        ds_hclib_ready(true);
                        asap_check_read(p2, 4);
                        ds_hclib_ready(false);

                        
                        // 3. access u under some condition
                        if (i == 0 || k == 0 || i == nx - 1 || k == ny - 1) {
                            unew[index] = f[index];
                        }
                        else{
                            // access u
                            int index3 = index2d(ny, i-1, k);
                            int index4 = index2d(ny, i+1, k);
                            int* p3 = ((int*) &u[index3]);
                            int* p4 = ((int*) &u[index4]);

                            ds_hclib_ready(true);
                            asap_check_read(p3, 4);
                            asap_check_read(p4, 4);
                            ds_hclib_ready(false);

                            unew[index] = 0.25 * (u[index3] + u[index2d(ny, i, k+1)] + u[index2d(ny, i, k-1)] + u[index4] 
                                        + f[index] * dx * dy);
                        }
                    }

                    ds_hclib_ready(false);
                #else

                    for (int jb = 0; jb < ny; jb++) {
                        if (i == 0 || jb == 0 || i == nx - 1 || jb == ny - 1) {
                            unew[index2d(ny, i, jb)] = f[index2d(ny, i, jb)];
                        } else {
                            unew[index2d(ny, i, jb)] = 0.25 * (u[index2d(ny, i-1, jb)] + u[index2d(ny, i, jb+1)] + u[index2d(ny, i, jb-1)] + u[index2d(ny, i+1, jb)]
                                                    + f[index2d(ny, i, jb)] * dx * dy);
                        }
                    }

                #endif
 
                promise_unew[i]->put();

            }); // end of async
        }
    }
}

/* R8MAT_RMS returns the RMS norm of a vector stored as a matrix. */
float r8mat_rms(int nx, int ny, float *a_) {
    float *a = a_;
    // float (*a)[nx][ny] = (float (*)[nx][ny])a_;
    int i;
    int j;
    float v;

    v = 0.0;

    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            v = v + a[index2d(ny, i, j)] * a[index2d(ny, i, j)];
            // v = v + (*a)[i][j] * (*a)[i][j];
        }
    }
    v = sqrt(v / (float) (nx * ny));

    return v;
}

/* RHS initializes the right hand side "vector". */
void rhs(const int nx, const int ny, float *f_, int block_size)
{
    float *f = f_;
    // float (*f)[nx][ny] = (float (*)[nx][ny])f_;

    int i,ii;
    int j,jj;
    float x;
    float y;

    // The "boundary" entries of F store the boundary values of the solution.
    // The "interior" entries of F store the right hand sides of the Poisson equation.

// #pragma omp parallel
// #pragma omp master
    //for collapse(2)
    for (j = 0; j < ny; j+=block_size)
    {
        for (i = 0; i < nx; i+=block_size)
        {
// #pragma omp task firstprivate(block_size,i,j,nx,ny) private(ii,jj,x,y) shared(f)
            for (jj=j; jj<j+block_size; ++jj)
            {
                y = (float) (jj) / (float) (ny - 1);
                for (ii=i; ii<i+block_size; ++ii)
                {
                    x = (float) (ii) / (float) (nx - 1);
                    if (ii == 0 || ii == nx - 1 || jj == 0 || jj == ny - 1){
                        f[index2d(ny, ii, jj)] = u_exact(x,y);
                        // (*f)[ii][jj] = u_exact(x, y);
                    }
                    else{
                        f[index2d(ny, ii, jj)] = uxxyy_exact(x,y);
                        // (*f)[ii][jj] = - uxxyy_exact(x, y);
                    }
                }
            }
        }
   }
}

/* Evaluates the exact solution. */
float u_exact(float x, float y) {
    float pi = 3.141592653589793;
    float value;

    value = sin(pi * x * y);

    return value;
}

/* Evaluates (d/dx d/dx + d/dy d/dy) of the exact solution. */
float uxxyy_exact(float x, float y) {
    float pi = 3.141592653589793;
    float value;

    value = - pi * pi * (x * x + y * y) * sin(pi * x * y);

    return value;
}


void run(int ms, int bs, int nit)
{
    #ifdef RACE_DETECTION
        ds_hclib_ready(false);
    #endif
    int matrix_size = ms;
    int block_size = bs;
    int niter = nit;
    float dx;
    float dy;
    float error;
    int ii,i;
    int jj,j;
    int nx = matrix_size;
    int ny = matrix_size;

    float *f_ = (float*) malloc(nx * nx * sizeof(float));
    if (f_ == 0){
        printf("malloc error \n");
    }
    float *f = f_;
    float *u_ = (float*) malloc(nx * nx * sizeof(float));
    float *unew_ = (float*) malloc(nx * ny * sizeof(float));
    float *unew = unew_;

    if( (nx % block_size) || (ny % block_size) )
    {
        printf("*****ERROR: blocsize must divide NX and NY \n");
        return;
    }


    /// INITIALISATION
    dx = 1.0 / (float) (nx - 1);
    dy = 1.0 / (float) (ny - 1);

    // Set the right hand side array F.
    rhs(nx, ny, f_, block_size);

    //for collapse(2)
    

    for (j = 0; j < ny; j+= block_size)
        for (i = 0; i < nx; i+= block_size)
        {
            {
            for (jj=j; jj<j+block_size; ++jj)
                for (ii=i; ii<i+block_size; ++ii)
                {
                    if (ii == 0 || ii == nx - 1 || jj == 0 || jj == ny - 1) {
                        unew[index2d(ny, ii, jj)] = f[index2d(ny, ii, jj)];
                        // (*unew)[ii][jj] = (*f)[ii][jj];
                    } else {
                        unew[index2d(ny, ii, jj)] = 0.0;
                        // (*unew)[ii][jj] = 0.0;
                    }
                }
            }
        }

    /// KERNEL INTENSIVE COMPUTATION
    // long start = hclib_current_time_ms();
    
    #ifdef RACE_DETECTION
        ds_hclib_ready(true);
    #endif
        sweep(nx, ny, dx, dy, f_, 0, niter, u_, unew_, block_size);
    #ifdef RACE_DETECTION
        ds_hclib_ready(false);
    #endif

    // long end = hclib_current_time_ms();
    // float dur = ((float)(end-start))/1000;
    // printf("Sweep time = %f\n",dur);

    #ifdef CHECK
        float x;
        float y;
        float *udiff_ = (float*) malloc(nx * ny * sizeof(float));
        float *udiff = udiff_;
        //float (*udiff)[nx][ny] = (float (*)[nx][ny])udiff_;
        /// CHECK OUTPUT
        // Check for convergence.
        for (j = 0; j < ny; j++) {
            y = (float) (j) / (float) (ny - 1);
            for (i = 0; i < nx; i++) {
                x = (float) (i) / (float) (nx - 1);
                udiff[index2d(ny, i, j)] = unew[index2d(ny, i, j)] - u_exact(x,y);
                // (*udiff)[i][j] = (*unew)[i][j] - u_exact(x, y);
            }
        }
        error = r8mat_rms(nx, ny, udiff_);

        float error1;
        // Set the right hand side array F.
        rhs(nx, ny, f_, block_size);

        
        // Set the initial solution estimate UNEW.
        // We are "allowed" to pick up the boundary conditions exactly.
        
        for (j = 0; j < ny; j++) {
            for (i = 0; i < nx; i++) {
                if (i == 0 || i == nx - 1 || j == 0 || j == ny - 1) {
                    unew[index2d(ny, i, j)] = f[index2d(ny, i, j)];
                    // (*unew)[i][j] = (*f)[i][j];
                } else {
                    unew[index2d(ny, i, j)] = 0.0;
                    // (*unew)[i][j] = 0.0;
                }
            }
        }

        sweep_seq(nx, ny, dx, dy, f_, 0, niter, u_, unew_);

        // Check for convergence.
        for (j = 0; j < ny; j++) {
            y = (float) (j) / (float) (ny - 1);
            for (i = 0; i < nx; i++) {
                x = (float) (i) / (float) (nx - 1);
                udiff[index2d(ny, i, j)] = unew[index2d(ny, i, j)] - u_exact(x, y);
                // (*udiff)[i][j] = (*unew)[i][j] - u_exact(x, y);
            }
        }
        error1 = r8mat_rms(nx, ny, udiff_);

        printf("error is %f \n", fabs(error - error1));
        free(udiff_);
    #endif

    free(f_);
    free(u_);
    free(unew_);
}


int main (int argc, char ** argv) {
    assert(index2d(5,1,2) == 7);
    printf("jacobi benchmark \n");
    int matrix_size = 512;
    int block_size = 128;
    int niter = 4;

    if(argc == 4){
        matrix_size = atoi(argv[1]);
        block_size = atoi(argv[2]);
        niter = atoi(argv[3]);
    }
    printf("matrix size: %d     block_size: %d      number of iteration: %d \n",matrix_size,block_size,niter);

    char const *deps[] = { "system" }; 
    hclib::launch(deps, 1, [&]() {
        long start = hclib_current_time_ms();
        
        run(matrix_size, block_size, niter);

        long end = hclib_current_time_ms();
        float dur = ((float)(end-start))/1000;
        printf("Run Time = %f \n \n",dur);

    #ifdef RACE_DETECTION
        printf("DPST height is: %d \n", get_dpst_height());
        printf("cache size is %d \n",ds_get_cache_size());
        printf("number of task is %d \n",get_task_id_unique());
        printf("number of nt join %d \n", get_nt_count());
        printf("number of tree joins %d \n", ds_get_tree_join_count());
        ds_print_check_write_count();
        ds_print_check_read_count();
    #endif
    });

    return 0;
}
