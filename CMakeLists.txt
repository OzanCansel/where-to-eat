cmake_minimum_required( VERSION 3.10 )
project( wte )

add_executable( wte main.cpp placer.h placer.cpp )

target_compile_features( wte PRIVATE cxx_std_17 )

FILE( COPY dataset/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )