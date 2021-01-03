cmake_minimum_required(VERSION 2.8)
project( ${EXEC_BINARY} )

message( "[info.] Executable binary code for '${CPP_SOURCE}' will be stored in '${EXEC_BINARY}'" )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( ${EXEC_BINARY} ${CPP_SOURCE} )
target_link_libraries( ${EXEC_BINARY} ${OpenCV_LIBS} )
