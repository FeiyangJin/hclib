# Set up include paths, library paths, and flags for the pass
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\
    -fdiagnostics-color=always")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

include(CheckCXXCompilerFlag)
# check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
# if (${SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG} EQUAL "1")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
# endif()

# Set the build directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

set(PASS "instrumentation")
set(PASS_SOURCE "instrumentation.cpp")
add_library(${PASS} SHARED ${PASS_SOURCE})
target_include_directories(${PASS} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set(DETECTOR "asap")
set(DETECTOR_SOURCE "check.cpp" "ds_dpst.cpp" "ds_to_hclib.cpp" "mem_access.cpp")
add_library(${DETECTOR} SHARED ${DETECTOR_SOURCE})
target_include_directories(${DETECTOR} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PASS} ${DETECTOR} DESTINATION ${CMAKE_INSTALL_PREFIX})
