cmake_minimum_required(VERSION 3.16)

set(IDF_TARGET "esp32c5" CACHE STRING "ESP-IDF target")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(rcj_comm_module)
