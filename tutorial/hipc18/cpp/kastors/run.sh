LD_LIBRARY_PATH="/home/fjin/gitRepo/hclib/instrumentation:${LD_LIBRARY_PATH}" HCLIB_WORKERS=1 ./poisson-task.exe 8192 128 4

LD_LIBRARY_PATH="/home/fjin/gitRepo/hclib/instrumentation:${LD_LIBRARY_PATH}" HCLIB_WORKERS=1 ./sparselu.exe 128 32

# LD_LIBRARY_PATH="/home/fjin/gitRepo/hclib/instrumentation:${LD_LIBRARY_PATH}" HCLIB_WORKERS=1 ./strassen.exe 2048 128 4
