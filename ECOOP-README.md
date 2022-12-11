## Overview: What does the artifact comprise?
This artifact is a data race detector for task-parallelism programs with promises. 

* Type: code
* Format: docker image
* Location: The docker image can be find [here](https://hub.docker.com/r/lecheny/drdp). 
* Badges claimed: Functional, Reusable, Available

## For authors claiming a functional or reusable badge: What are claims about the artifact’s functionality to be evaluated by the committee?

* Which data or conclusions from the paper are generated/supported by the artifact components?
  * Data in section 5: Table 1, Table 2, Table 3, Table 4 and Figure 11
* Which activities described in the paper have led to the creation of the artifact components?
  * Section 4: DRDP algorithm
  * Section 5: Evaluation

Please provide explicit references that link processes, data, or conclusions in the paper with the location of the supporting component in the artifact.  

To acquire evaluation results displayed in the paper, please launch the evaluation according to the **Evaluations** section in the end of this README.
* Table 1: Use commands in evaluation 1. The head of each benchmark (test*.cpp) has detailed comments. 
* Table 2 data except for the time and memory columns: Use commands in evaluation 2. 
* Table 2 's time and memory columns, and Figure 11: Same as evaluation 2. We use the data collected in the first run, which enables debug info.
* Table 3: Use commands in evaluation 3.
* Table 4: Use commands in evaluation 4.

## For authors claiming a reusable badge: What are the authors' claims about the artifact's reusability to be evaluated by the committee?

The algorithm we design can be extended for other race detectors on task-parallel programs with promise (or similar data structures). The algorithm is introduced in section 4 of the paper. Here we discuss how it can be extended. 
* Shadow memory: the shadow memory part can be directly used by other tools. It consists of three files in `/opt/hclib/race_detector/src`: `shadow_memory.h`, `mem_access.h`, and `mem_access.cpp`;
* Instrumentation: a standalone LLVM transform pass (instrumentation.h, instrumentation.cpp) that other tools can apply directly to capture memory accesses in a C/CPP program. 

## For authors claiming an available badge
* We plan to publish our artifact on Github and DARTS. These repositories will include all sources code. Links are temporarily removed for double-blind review.
* [Github retention policy](https://docs.github.com/en/organizations/managing-organization-settings/configuring-the-retention-period-for-github-actions-artifacts-and-logs-in-your-organization)
* [BSD 3-Clause "New" or "Revised" License](https://choosealicense.com/licenses/bsd-3-clause/)

## Artifact Requirements

Please list any specific hardware or software requirements for accessing your artifact

**The machine should have at least 32 GB memory (RAM)**

## Getting Started
### HClib repository
In the root directory of HClib's repository, there are two scripts to help install HClib.
* `install.sh`, which is used to install a HClib instance, and
* `install_artifact.sh`, which installs all required HClib instance for our evaluations.

The following HClib options are related to our evaluations. These options can be specified when invoking `install.sh`.
Please note that all these options are actually CMake options defined in `CMakeLists.txt`. So users need to add the prefix "-D"
for all these options when passing them to `install.sh` as input.
 
| Option                       | Effect                                                          | Default  |
| ---                          | ---                                                             | ---      |
| HCLIB_ENABLE_DRDP            | turn on/off DRDP                                                | OFF      |
| HCLIB_DRDP_DEBUG_INFO        | collect and report debug info when DRDP conducts race detection | ON       |
| HCLIB_DRDP_QUERY_ALWAYS_TRUE | naively return true for reachability query                      | OFF      |
| HCLIB_DRDP_BFS_QUERY         | apply BFS/DFS when traversing the computational graph           | ON (bfs) |
| HCLIB_DRDP_REVERSE_NT_ORDER  | traverse a node's non-tree join edge list in reverse order      | ON       |

### Docker Image for DRDP
A docker image with DRDP installed is available at [Docker Hub](https://hub.docker.com/r/lecheny/drdp).
Please use the following command to download and launch the docker image.
    
    docker pull lecheny/drdp
    docker run -i -t lecheny/drdp

The docker image uses llvm-14.0 as the default compiler, which is placed in `/opt/llvm14`.

DRDP is a dynamic race detector for HClib parallel programming model. We have integrated DRDP into HClib as an optional module.
When installing HClib with DRDP enabled, an instrumentation script (`inst.sh`) will be generated in the `bin` folder of the installation directory. At the same time, DRDP's dynamic analysis library will be placed in the `lib` folder.

To use DRDP, programmers need to instrument an HClib program using the instrumentation script, then invoke the program using a DRDP-enabled HClib instance.
The underlying runtime of HClib will automatically communicate with DRDP's dynamic analysis library to carry out race detection.

The root directory of HClib in the docker image is `/opt/hclib`.
For convenience, we have pre-installed two HClib instances in `/opt/hclib` using the script `install_artifact.sh`.
* `/opt/hclib/hclib-install-orig`: an original HClib instance which will be used to measure each benchmark's baseline performance;
* `/opt/hclib/hclib-install`: a DRDP-enabled HClib instance which will be used to measure DRDP's impact to the program execution.

Please use the following commands if reviewers want to manually install HClib and DRDP in other directories. Please notice that all evaluation scripts introduced in the following sections will use the pre-installed HClib instances.

Install original HClib:
  * `cd /opt/hclib`
  * `INSTALL_PREFIX=[path to installation directory] ./install.sh -DHCLIB_ENABLE_PRODUCTION=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`

Install HClib with DRDP enabled:
  * `cd /opt/hclib`
  * `INSTALL_PREFIX=[path to installation directory] ./install.sh -DHCLIB_ENABLE_PRODUCTION=ON -DHCLIB_ENABLE_DRDP=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`


In total, we mentioned seven performance benchmarks in the paper. All seven benchmarks reside in `/opt/hclib/race_detector_benchmark/bench`, including
* health 
* knapsack 
* matmul
* sort 
* poisson 
* sparselu 
* strassen

Five benchmarks in `/opt/hclib/race_detector_benchmark/bench` have been transformed using a polyhedral-analysis-based optimizer to remove redundant checks of memory accesses.
We also put the original version of these benchmarks in the folder `/opt/hclib/race_detector_benchmark/noskip` to evaluate DRDP's performance impact without the optimization.

We also mention correctness benchmarks in the paper. All correctness benchmarks reside in `/opt/hclib/race_detector_benchmark/correctness`, including
* 1 - 9 are benchmarks that written by ourselves to test the infrastructure correctness, and
* 10 - 23 are benchmarks presented in Table 1 of our paper. 

All evaluations are conducted using benchmarks in `/opt/hclib/race_detector_benchmark`. It contains the following subdirectories:
| Folder       | Usage                                                                 |
| ---          | ---                                                                   |
| bin          | scripts to help launch evaluations                                    |
| common       | global Makefile settings                                              |
| race_example | a simple HClib program with a data race                               |
| correctness  | benchmarks to validate DRDP's implementation                          |
| bench        | performance benchmarks with polyhedral-analysis-based optimization    |
| noskip       | performance benchmarks without polyhedral-analysis-based optimization |

### Basic usage of provided scripts
To reproduce the evaluation results described in the paper, we provide bash scripts to help set up the environment and carry out evaluations. The provided bash scripts will automatically conduct all necessary work, including setting up HClib's environment, instrumenting benchmarks, and launching the evaluation.

**/opt/hclib/race_detection_benchmark/bin/evaluation.sh**

This bash script is used to conduct performance evaluations for benchmark sets. Users can choose the benchmark set through the option `--bench`. By default, it will launch the evaluations
on all seven benchmarks in `bench`.
For each benchmark, it first executes the original version for certain times as the baseline, then invokes DRDP to conduct the race detection.
By default, both the baseline and the race detection will be conducted five times. The number of times can be changed through the environment variable `TIMES`. For example, `TIMES=1 evaluation.sh` sets the times to 1. During the execution, `evaluation.sh` will collect the execution time and memory usage into a csv file ("eval-[timestamp].csv"), and draw four bar charts to illustrate the time and memory overhead (time.pdf, memory.pdf, time_overhead.pdf, memory_overhead.pdf).

Launch the performance evaluation on all benchmarks in `bench` (i.e., with polyhedral-analysis-based optimization)

    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh

Launch the performance evaluation on all benchmarks in `noskip` (i.e., without polyhedral-analysis-based optimization)

    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh --bench noskip
    
To review these pdf files, please first copy them from the docker container to the host machine. When is copy is finished, these pdf files
can be displayed using a preferred pdf viewer.

    # search for the container ID
    docker ps
    
    # copy file from the docker container to the host
    docker cp <containerId>:<path_in_the_container> <destionation_in_the_host>

    # Open the pdf file with any preferred pdf viewer
    
    
**run.sh in each benchmark's folder**

For each benchmark, we provide a MAKEFILE for compilation and a bash script, `run.sh`, to launch the benchmark. 
To run each benchmark individually, please follow these steps.
  * `cd /opt/hclib/race_detection_benchmark/[benchmark name]`(replace [benchmark name] with a valid benchmark name)
  * `./run.sh --orig` (this will run the baseline version without any race detection)
  * `./run.sh --rd` (this will launch race detection for the benchmark)
  
To rebuild the benchmark before execution, add the option `--rebuild` to the commandline.


## Evaluations
### Evaluation 1: Correctness evaluation
  * Benchmark locations: /opt/hclib/race_detection_benchmark/correctness

Commands to launch evaluation: 

    cd /opt/hclib/race_detector_benchmark/correctness
    ./run.sh
    
    
### Evaluation 2: default performance evaluation (with polyhedral optimization)
  * Benchmark locations: `/opt/hclib/race_detection_benchmark/bench`
  * Setup: no additional setup needed, using `evaluation.sh`
  * Skip duplicate checks: Yes
  * Graph traversal order: BFS
  * Non-tree join traversal order: Back-to-Front

Commands to launch evaluation:  
    
    # Using HClib installation with debug info
    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh
    
    # Update HClib installation to turn off debug info
    cd /opt/hclib/
    ./install.sh -DHCLIB_ENABLE_DRDP=ON -DHCLIB_DRDP_DEBUG_INFO=OFF -DHCLIB_ENABLE_PRODUCTION=ON \
                 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    
    # Re-evaluate all benchmarks without collecting debug info
    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh
    
    # Revert changes to the HClib installation
    cd /opt/hclib/
    ./install.sh -DHCLIB_ENABLE_DRDP=ON -DHCLIB_ENABLE_PRODUCTION=ON    \
                 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++    
 
### Evaluation 3: performance evaluation using different graph and non-tree join traversal orders
  *  Benchmark locations: `/opt/hclib/race_detection_benchmark/bench`

Commands to launch evaluation:    

    # Update HClib installation according to specified graph traversal order and non-tree join traversal order
    cd /opt/hclib/
    ./install.sh -DHCLIB_ENABLE_DRDP=ON -DHCLIB_ENABLE_PRODUCTION=ON \
                 -DHCLIB_DRDP_BFS_QUERY=ON/OFF -DHCLIB_DRDP_REVERSE_NT_ORDER=ON/OFF                    \
                 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    
    # Re-evaluate all benchmarks
    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh
    
    # Revert changes to the HClib installation
    cd /opt/hclib/
    ./install.sh -DHCLIB_ENABLE_DRDP=ON -DHCLIB_ENABLE_PRODUCTION=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    
### Evaluation 4: run all benchmarks in `bench` and `noskip` to compare the effect of polyhedral optimization
  * Benchmark locations: `/opt/hclib/race_detection_benchmark/bench` and `/opt/hclib/race_detection_benchmark/bench`
  * Setup: no additional setup needed, using `evaluation.sh`
  * Skip duplicate checks: Yes
  * Graph traversal order: BFS
  * Non-tree join traversal order: Back-to-Front
  
Make sure your machine has at least **400GB** memory (RAM) to run benchmarks in `noskip`. 
Commands to launch evaluation:  
    
    cd /opt/hclib/race_detection_benchmark
    ./bin/evaluation.sh
    ./bin/evaluation.sh --bench noskip


