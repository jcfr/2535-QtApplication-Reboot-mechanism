cmake_minimum_required(VERSION 2.8.4)

find_package(Qt4)
include(${QT_USE_FILE})

include(${CMAKE_CURRENT_SOURCE_DIR}/ctkMacroGenerateMocs.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

QT4_GENERATE_MOCS(RestartWithQProcessDetached.cpp)
add_executable(RestartWithQProcessDetached RestartWithQProcessDetached.cpp)
target_link_libraries(RestartWithQProcessDetached ${QT_LIBRARIES})

QT4_GENERATE_MOCS(RestartWithWhileLoop.cpp)
add_executable(RestartWithWhileLoop RestartWithWhileLoop.cpp)
target_link_libraries(RestartWithWhileLoop ${QT_LIBRARIES})

