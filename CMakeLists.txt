cmake_minimum_required(VERSION 2.8.4)

find_package(Qt4)
include(${QT_USE_FILE})

include(${CMAKE_CURRENT_SOURCE_DIR}/ctkMacroGenerateMocs.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(common_moc_srcs
  ctkAbstractRestartManager.h
  ctkErrorLogFDMessageHandler_p.h
  ctkErrorLogModel.h
  )
  
set(common_srcs
  ${common_moc_srcs}
  ctkAbstractRestartManager.cpp
  ctkErrorLogFDMessageHandler.cpp
  ctkErrorLogFDMessageHandler.h
  ctkErrorLogModel.cpp
  ctkRestartWidget.h
  ctkRestartWidget.cpp
  ctkUtils.cpp
  ctkUtils.h
  )
QT4_GENERATE_MOCS(${common_moc_srcs})

# Case1
QT4_GENERATE_MOCS(RestartWithQProcessDetached.cpp)
add_executable(RestartWithQProcessDetached
  ${common_srcs}
  RestartWithQProcessDetached.cpp
  )
target_link_libraries(RestartWithQProcessDetached ${QT_LIBRARIES})

# Case2
QT4_GENERATE_MOCS(RestartWithWhileLoop.cpp)
add_executable(RestartWithWhileLoop
  ${common_srcs}
  RestartWithWhileLoop.cpp 
  )
target_link_libraries(RestartWithWhileLoop ${QT_LIBRARIES})

