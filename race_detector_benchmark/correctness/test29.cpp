#include "hclib_cpp.h"
#include <stdio.h>

#define N 100

int a[N][N];

int main(int argc, char **argv) {
    char const *deps[] = { "system" };

    hclib::launch(deps, 1, [&]() {
        ds_hclib_ready(true);

        // Create a 2D array of promises to enforce dependencies
        hclib::promise_t<void> *promises[N][N];

        // Initialize promises
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                promises[i][j] = new hclib::promise_t<void>();
            }
        }

        hclib::finish([&]() {
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    hclib::async([=]() {
                        a[i][j] = a[i][j] + 1;

                        // Wait for dependencies
                        if (i > 0) promises[i - 1][j]->get_future()->wait();
                        if (j > 0) promises[i][j - 1]->get_future()->wait();

                        // Print statement after dependencies are resolved
                        printf("test i=%d j=%d\n", i, j);

                        // Mark this task as complete
                        promises[i][j]->put();
                    });
                }
            }
        });

        // Cleanup dynamically allocated promises
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                delete promises[i][j];
            }
        }
    });

    return 0;
}
