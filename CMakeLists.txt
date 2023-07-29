cmake_minimum_required(VERSION 3.26)
project(jss_cp)

set(CMAKE_CXX_STANDARD 20)


################
##  OR_TOOLS  ##
################

# Fetch OR-tools library and create the alias ortools::ortools.
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
set(BUILD_DEPS ON)
set(BUILD_SAMPLES OFF)
set(BUILD_EXAMPLES OFF)
set(ABSL_PROPAGATE_CXX_STD ON)

FetchContent_Declare(
        or-tools
        GIT_REPOSITORY https://github.com/google/or-tools.git
        GIT_TAG master

)
FetchContent_MakeAvailable(or-tools)

################
##   CPLEX    ##
################

find_package(Threads)

set(CPLEX_STUDIO_DIR /home/tatdatnguyen/ibm/ILOG/CPLEX_Studio2211)
#find_path(CPLEX_STUDIO_DIR cpoptimizer/include/ilcp/cp.h REQUIRED)
find_path(CONCERT_DIR include/ilconcert/iloenv.h HINTS ${CPLEX_STUDIO_DIR}/concert REQUIRED)
find_path(CPOPTIMIZER_DIR include/ilcp/cp.h REQUIRED HINTS ${CPLEX_STUDIO_DIR}/cpoptimizer REQUIRED)
find_path(CPLEX_DIR include/ilcplex/cplex.h HINTS ${CPLEX_STUDIO_DIR}/cplex REQUIRED)
include_directories(${CONCERT_DIR}/include)
include_directories(${CPOPTIMIZER_DIR}/include)
find_library(CONCERT_LIB concert PATHS ${CONCERT_DIR}/lib REQUIRED PATH_SUFFIXES x86-64_linux/static_pic x86-64_osx/static_pic ppc64le_linux/static_pic s390x_linux/static_pic x64_windows_vs2017/stat_mda x64_windows_msvc14/stat_mda x64_windows_vs2015/stat_mda x64_windows_vs2013/stat_mda)
find_library(CPOPTIMIZER_LIB cp PATHS ${CPOPTIMIZER_DIR}/lib REQUIRED PATH_SUFFIXES x86-64_linux/static_pic x86-64_osx/static_pic ppc64le_linux/static_pic s390x_linux/static_pic x64_windows_vs2017/stat_mda x64_windows_msvc14/stat_mda x64_windows_vs2015/stat_mda x64_windows_vs2013/stat_mda)
find_library(CPLEX_LIB NAMES cplex cplex2211 PATHS ${CPLEX_DIR}/lib REQUIRED PATH_SUFFIXES x86-64_linux/static_pic x86-64_osx/static_pic ppc64le_linux/static_pic s390x_linux/static_pic x64_windows_vs2017/stat_mda x64_windows_msvc14/stat_mda x64_windows_vs2015/stat_mda x64_windows_vs2013/stat_mda)

add_executable(jss_cp main.cpp
        data_manager.cpp
        data_manager.h
        solution_sequence.cpp
        solution_sequence.h)

target_link_libraries(jss_cp ortools::ortools)
target_link_libraries(jss_cp ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(jss_cp ${CPOPTIMIZER_LIB})
target_link_libraries(jss_cp ${CPLEX_LIB})
target_link_libraries(jss_cp ${CONCERT_LIB})
