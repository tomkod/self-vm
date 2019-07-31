cmake_minimum_required (VERSION 2.6)
project (self-vm)
set (CMAKE_CXX_STANDARD 14)
add_executable(vm vm.cpp vm.h)
add_executable(vm-gen vm-gen.cpp vm.h)

