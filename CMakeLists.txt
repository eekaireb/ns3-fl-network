cmake_minimum_required(VERSION 3.20)
project(lint)

file( GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src  *.cc *.h )
file( GLOB_RECURSE INCS ${CMAKE_SOURCE_DIR}/src  *.h )

file( GLOB_RECURSE PROJ ${CMAKE_SOURCE_DIR}/scratch/wifi_exp  *.cc *.h )

include_directories(${INCS}
        {CMAKE_SOURCE_DIR}/scratch/wifi_exp
        )



add_custom_target(build
        COMMAND make -f Makefile
        SOURCES ${SRCS} ${PROJ}
        )

