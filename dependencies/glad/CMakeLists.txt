cmake_minimum_required(VERSION 3.7)

project(glad)

include_directories(include)
add_library(glad STATIC glad.c)
