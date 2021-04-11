project(instrumentation)
cmake_minimum_required(VERSION 3.12)
enable_language(CXX)
set(CMAKE_VERBOSE_MAKEFILE on)

# Set up paths to the include and library directries of the installed LLVM
if (NOT SYS_LLVM_INSTALL_DIR)
    message(FATAL_ERROR "Path to the installed llvm (SYS_LLVM_INSTALL_DIR) is not specified")
endif()

set(SYS_LLVM_INSTALL_DIR "" CACHE PATH "LLVM Installation Directory")
set(SYS_LLVM_INCLUDE_DIR "${SYS_LLVM_INSTALL_DIR}/include/llvm")
if(NOT EXISTS "${SYS_LLVM_INCLUDE_DIR}")
    message(FATAL_ERROR "${SYS_LLVM_INCLUDE_DIR} does not exist")
endif()

# Set up path to the config file of the installed LLVM
set(SYS_LLVM_LIB_DIR_LIST "${SYS_LLVM_INSTALL_DIR}/lib")
list(APPEND SYS_LLVM_LIB_DIR_LIST "${SYS_LLVM_INSTALL_DIR}/lib64")
set(SYS_LLVM_CMAKE_FILE_FOUND "0")
foreach(LIB_DIR ${SYS_LLVM_LIB_DIR_LIST})
    set(SYS_LLVM_CMAKE_FILE "${LIB_DIR}/cmake/llvm/LLVMConfig.cmake")
    if(EXISTS ${SYS_LLVM_CMAKE_FILE})
        set(SYS_LLVM_CMAKE_DIR "${LIB_DIR}/cmake/llvm")
        set(SYS_LLVM_CMAKE_FILE_FOUND "1")
        break()
    endif()
endforeach()

if(NOT SYS_LLVM_CMAKE_FILE_FOUND)
    message(FATAL_ERROR "Fail to locate LLVM config file, search paths: ${SYS_LLVM_LIB_DIR_LIST}")
endif()

# Load LLVM's config file
list(APPEND CMAKE_PREFIX_PATH "${SYS_LLVM_CMAKE_DIR}")
find_package(LLVM REQUIRED CONFIG)
set(SYS_LLVM_MINIMUM_VERSION "11")
if (${LLVM_VERSION_MAJOR} VERSION_LESS ${SYS_LLVM_MINIMUM_VERSION})
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR} < ${SYS_LLVM_MINIMUM_VERSION}")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

add_subdirectory(src)
