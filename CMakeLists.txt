
# Set the name and the supported language of the project
project(tsxProject C)
# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 3.7)

set( CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_BUILD_TYPE Release) # optimizations, but debuging is not allowed
#set(CMAKE_BUILD_TYPE Debug) # optimizations, but debuging is not allowed

# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

include(FindGTK2)
if (GTK2_FOUND)
    include_directories(${GTK2_INCLUDE_DIRS})
    link_directories(${GTK2_LIBRARY_DIRS})
    link_libraries(${GTK2_LIBRARIES})
endif (GTK2_FOUND)

set(CMAKE_C_STANDARD 99)
set(GCC_TM_LINK_FLAG -fgnu-tm
        #-O3
        #-fauto-inc-dec -fbranch-count-reg -fcombine-stack-adjustments -fcompare-elim -fcprop-registers -fdce -fdefer-pop -fdelayed-branch -fdse -fforward-propagate -fguess-branch-probability -fif-conversion2 -fif-conversion -finline-functions-called-once -fipa-pure-const -fipa-profile -fipa-reference -fmerge-constants
        #-fmove-loop-invariants -fshrink-wrap -fsplit-wide-types -ftree-bit-ccp -ftree-ccp -fssa-phiopt -ftree-ch -ftree-copy-prop -ftree-copyrename -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre -ftree-phiprop -ftree-sink -ftree-slsr -ftree-sra -ftree-pta -ftree-ter -funit-at-a-time

        #-fthread-jumps -falign-functions  -falign-jumps -falign-loops  -falign-labels -fcaller-saves -fcrossjumping -fcse-follow-jumps  -fcse-skip-blocks -fdelete-null-pointer-checks -fdevirtualize -fdevirtualize-speculatively -fexpensive-optimizations -fgcse  -fgcse-lm  -fhoist-adjacent-loads -finline-small-functions -findirect-inlining -fipa-cp -fipa-cp-alignment -fipa-sra -fipa-icf -fisolate-erroneous-paths-dereference
        #-flra-remat -foptimize-sibling-calls -foptimize-strlen -fpartial-inlining -fpeephole2 -freorder-blocks -freorder-blocks-and-partition -freorder-functions -frerun-cse-after-loop  -fsched-interblock  -fsched-spec -fschedule-insns  -fschedule-insns2 -fstrict-aliasing -fstrict-overflow -ftree-builtin-call-dce -ftree-switch-conversion -ftree-tail-merge -ftree-pre -ftree-vrp -fipa-ra

        #-finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-loop-vectorize -ftree-loop-distribute-patterns -ftree-slp-vectorize -fvect-cost-model -ftree-partial-pre -fipa-cp-clone
        )

add_definitions(${GCC_TM_LINK_FLAG})

set(SOURCE_FILES main.c TestTSX.c UtilFunctionsTSX.c ThreadUtils.c UtilFunctionsTSX.h ThreadUtils.h TestTSX.h BenchmarkSupport.c BenchmarkSupport.h TestAtomicOps.c TestAtomicOps.h TestCoreAffinity.c TestCoreAffinity.h Test_HyperThread.c Test_HyperThread.h Queues.c Queues.h lynxq.h)
add_executable(tsxExe ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(tsxExe pthread)
TARGET_LINK_LIBRARIES(tsxExe capstone)

TARGET_LINK_LIBRARIES(tsxExe m)