project(mtlib)
cmake_minimum_required(VERSION 2.8)

############compiler settings############
set(CMAKE_C_COMPILER "g++")
add_definitions(-g -W -Wall -std=c++11 )
#add_definitions(-DDEBUG)
set(TARGET mtndn)


############source files############
aux_source_directory(. SRC_LIST)
#add_subdirectory(subsrc)
#include_directories(include)    #head file path

############link in############
#link extern library directories, like -L in gcc
link_directories(/usr/local/lib)

#set extern libraries
set(LIBBOOST boost_system boost_thread boost_chrono boost_regex)
set(LIBNDNCPP ndn-cpp)
set(LIBTHIREPARTY pthread protobuf sqlite3 crypto)
set(LIBFFMPEG avcodec avutil avformat avfilter avdevice postproc swscale)
set(LIBGLOG glog dl)

############generate binary files############
#add_executable(${PROJECT_NAME} ${SRC_LIST})
add_library(${TARGET} SHARED ${SRC_LIST} )

#link libraries, like -l in gcc
target_link_libraries(${TARGET} ${LIBBOOST} ${LIBTHIREPARTY} ${LIBNDNCPP} ${LIBFFMPEG} ${LIBGLOG} )

set(INSTALL_INCLUDES glogger.h  interfaces.h  mtndn-defines.h  mtndn-library.h  name-components.h  params.h  simple-log.h  statistics.h) 
set(CMAKE_INSTALL_PREFIX /usr/local)
install(FILES ${INSTALL_INCLUDES} DESTINATION include)
install(TARGETS mtndn LIBRARY DESTINATION lib)
