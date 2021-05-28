#include <stdio.h>
#include <math.h>
#include <vector>
#include "hclib_cpp.h"

#define index2d(ny,i,j) (((i)*(ny))+(j))

double r8mat_rms(int nx, int ny, double *a_);
void rhs(int nx, int ny, double *f_, int block_size);
void timestamp(void);
double u_exact(double x, double y);
double uxxyy_exact(double x, double y);

void sweep_seq(int nx, int ny, double dx, double dy, double *f_, int itold, int itnew, double *u_, double *unew_)
{
    int i;
    int it;
    int j;
    double *f = f_;
    double *u = u_;
    double *unew = unew_;
    // double (*f)[nx][ny] = (double (*)[nx][ny])f_;
    // double (*u)[nx][ny] = (double (*)[nx][ny])u_;
    // double (*unew)[nx][ny] = (double (*)[nx][ny])unew_;

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


void sweep (int nx, int ny, double dx, double dy, double *f_, int itold, int itnew, double *u_, double *unew_, int block_size)
{
    int i;
    int it;
    int j;
    double *f = f_;
    double *u = u_;
    double *unew = unew_;

    // TODO: allocate unew_p[], 1-D array of nx promises that are all initialized = ready
    hclib::promise_t<void>* promise_unew[nx];
    for(int pi = 0; pi<nx; pi++){
#ifdef RACE_DETECTION
        ds_hclib_ready(false);
#endif
        hclib::promise_t<void> *p = new hclib::promise_t<void>();
#ifdef RACE_DETECTION
        ds_hclib_ready(true);
#endif

        p->put();
        promise_unew[pi] = p;
    }
#ifdef RACE_DETECTION
    ds_hclib_ready(true);
#endif
    hclib::promise_t<void>* promise_u[nx];

    for (it = itold + 1; it <= itnew; it++) {
        for(int pi=0; pi < nx; pi++){
#ifdef RACE_DETECTION
            ds_hclib_ready(false);
#endif
            hclib::promise_t<void> *p = new hclib::promise_t<void>();
#ifdef RACE_DETECTION
            ds_hclib_ready(true);
#endif
            promise_u[pi] = p;
        }
#ifdef RACE_DETECTION
        ds_hclib_ready(true);
#endif


        for (i = 0; i < nx; i++) {
#ifdef RACE_DETECTION
            ds_hclib_ready(false);
#endif
            hclib::async([=, &u, &unew, &promise_unew, &promise_u]() mutable{
#ifdef RACE_DETECTION
                ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
                ds_promise_task(true);
#endif
                promise_unew[i]->get_future()->wait();

                for (j = 0; j < ny; j++) {
                    u[index2d(ny,i,j)] = unew[index2d(ny,i,j)];
                }
 
                promise_u[i]->put();
                delete promise_unew[i];
                promise_unew[i] = new hclib::promise_t<void>();
            }); // end of async
        }

        for (i = 0; i < nx; i++) {
#ifdef RACE_DETECTION
            ds_hclib_ready(false);
#endif
            hclib::async([=, &u, &unew, &promise_u, &promise_unew]() mutable{
#ifdef RACE_DETECTION
                ds_hclib_ready(true);
#endif
#ifdef RACE_DETECTION
                ds_promise_task(true);
#endif
                if(i > 0){
                    promise_u[i-1]->get_future()->wait();
                }
                promise_u[i]->get_future()->wait();
                if(i < nx - 1){
                    promise_u[i+1]->get_future()->wait();
                }
                
                
                for (j = 0; j < ny; j++) {
                    if (i == 0 || j == 0 || i == nx - 1 || j == ny - 1) {
                        unew[index2d(ny, i, j)] = f[index2d(ny, i, j)];
                    } else {
                        unew[index2d(ny, i, j)] = 0.25 * (u[index2d(ny, i-1, j)] + u[index2d(ny, i, j+1)] + u[index2d(ny, i, j-1)] + u[index2d(ny, i+1, j)]
                                                + f[index2d(ny, i, j)] * dx * dy);
                    }
                }
                promise_unew[i]->put();

                // TODO: figure out how to reset or reallocate u_p[*]
            }); // end of async
        }
    }
}

/* R8MAT_RMS returns the RMS norm of a vector stored as a matrix. */
double r8mat_rms(int nx, int ny, double *a_) {
    double *a = a_;
    // double (*a)[nx][ny] = (double (*)[nx][ny])a_;
    int i;
    int j;
    double v;

    v = 0.0;

    for (j = 0; j < ny; j++) {
        for (i = 0; i < nx; i++) {
            v = v + a[index2d(ny, i, j)] * a[index2d(ny, i, j)];
            // v = v + (*a)[i][j] * (*a)[i][j];
        }
    }
    v = sqrt(v / (double) (nx * ny));

    return v;
}

/* RHS initializes the right hand side "vector". */
void rhs(const int nx, const int ny, double *f_, int block_size)
{
    double *f = f_;
    // double (*f)[nx][ny] = (double (*)[nx][ny])f_;

    int i,ii;
    int j,jj;
    double x;
    double y;

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
                y = (double) (jj) / (double) (ny - 1);
                for (ii=i; ii<i+block_size; ++ii)
                {
                    x = (double) (ii) / (double) (nx - 1);
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
double u_exact(double x, double y) {
    double pi = 3.141592653589793;
    double value;

    value = sin(pi * x * y);

    return value;
}

/* Evaluates (d/dx d/dx + d/dy d/dy) of the exact solution. */
double uxxyy_exact(double x, double y) {
    double pi = 3.141592653589793;
    double value;

    value = - pi * pi * (x * x + y * y) * sin(pi * x * y);

    return value;
}


void run(int ms, int bs, int nit)
{
    int matrix_size = ms;
    int block_size = bs;
    int niter = nit;
    double dx;
    double dy;
    double error;
    int ii,i;
    int jj,j;
    int nx = matrix_size;
    int ny = matrix_size;

    double *f_ = (double*) malloc(nx * nx * sizeof(double));
    if (f_ == 0){
        printf("malloc error \n");
    }
    double *f = f_;
    double *u_ = (double*) malloc(nx * nx * sizeof(double));
    double *unew_ = (double*) malloc(nx * ny * sizeof(double));
    double *unew = unew_;

    if( (nx % block_size) || (ny % block_size) )
    {
        printf("*****ERROR: blocsize must divide NX and NY \n");
        return;
    }


    /// INITIALISATION
    dx = 1.0 / (double) (nx - 1);
    dy = 1.0 / (double) (ny - 1);

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
#ifdef RACE_DETECTION
    ds_hclib_ready(true);
#endif
    long start = hclib_current_time_ms();
    
    sweep(nx, ny, dx, dy, f_, 0, niter, u_, unew_, block_size);
#ifdef RACE_DETECTION
    ds_hclib_ready(false);
#endif

    long end = hclib_current_time_ms();
    double dur = ((double)(end-start))/1000;
    printf("Sweep time = %f\n",dur);

    #ifdef CHECK
        double x;
        double y;
        double *udiff_ = (double*) malloc(nx * ny * sizeof(double));
        double *udiff = udiff_;
        //double (*udiff)[nx][ny] = (double (*)[nx][ny])udiff_;
        /// CHECK OUTPUT
        // Check for convergence.
        for (j = 0; j < ny; j++) {
            y = (double) (j) / (double) (ny - 1);
            for (i = 0; i < nx; i++) {
                x = (double) (i) / (double) (nx - 1);
                udiff[index2d(ny, i, j)] = unew[index2d(ny, i, j)] - u_exact(x,y);
                // (*udiff)[i][j] = (*unew)[i][j] - u_exact(x, y);
            }
        }
        error = r8mat_rms(nx, ny, udiff_);

        double error1;
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
            y = (double) (j) / (double) (ny - 1);
            for (i = 0; i < nx; i++) {
                x = (double) (i) / (double) (nx - 1);
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
    int matrix_size = 8192;
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
        double dur = ((double)(end-start))/1000;
        printf("Run Time = %f\n",dur);
    });

    return 0;
}
