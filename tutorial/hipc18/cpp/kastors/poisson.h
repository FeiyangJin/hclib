#ifndef KASTORS_POISSON_H
#define KASTORS_POISSON_H
#include <assert.h>

void sweep(int nx, int ny, double dx, double dy, double *f,
	   int itold, int itnew, double *u, double *unew, int block_size);
void sweep_seq(int nx, int ny, double dx, double dy, double *f,
	   int itold, int itnew, double *u, double *unew);


static inline void copy_block(int nx, int ny, int block_x, int block_y, double *u_, double *unew_, int block_size) {
    int i, j, start_i, start_j;
    double (*u)[nx][ny] = (double (*)[nx][ny])u_;
    double (*unew)[nx][ny] = (double (*)[nx][ny])unew_;
    start_i = block_x * block_size;
    start_j = block_y * block_size;
    for (i = start_i; i < start_i + block_size; i++) {
        for (j = start_j; j < start_j + block_size; j++) {
            assert((i < nx) && (j < ny));
            (*u)[i][j] = (*unew)[i][j];
        }
    }
}

static inline void compute_estimate(int block_x, int block_y, double *u_,
                                    double *unew_, double *f_, double dx,
                                    double dy, int nx, int ny, int block_size) {
    int i, j, start_i, start_j;
    double (*f)[nx][ny] = (double (*)[nx][ny])f_;
    double (*u)[nx][ny] = (double (*)[nx][ny])u_;
    double (*unew)[nx][ny] = (double (*)[nx][ny])unew_;
    start_i = block_x * block_size;
    start_j = block_y * block_size;
    for (i = start_i; i < start_i + block_size; i++) {
        for (j = start_j; j < start_j + block_size; j++) {
            if (i == 0 || j == 0 || i == nx - 1 || j == ny - 1) {
                (*unew)[i][j] = (*f)[i][j];
            } else {
                (*unew)[i][j] = 0.25 * ((*u)[i-1][j] + (*u)[i][j+1]
                                      + (*u)[i][j-1] + (*u)[i+1][j]
                                      + (*f)[i][j] * dx * dy);
            }
        }
    }
}
#endif
