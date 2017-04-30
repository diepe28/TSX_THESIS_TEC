cmake_minimum_required(VERSION 3.7)
project(tsxExample)

set(CMAKE_C_STANDARD 99)
set(GCC_TM_LINK_FLAG    '-fgnu-tm')
add_definitions(${GCC_TM_LINK_FLAG})


set(SOURCE_FILES main.c TestTSX.c UtilFunctionsTSX.c ThreadUtils.c UtilFunctionsTSX.h ThreadUtils.h TestTSX.h)
add_executable(txsExe ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(txsExe pthread)
