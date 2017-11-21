# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

#设置变量SRC_DIR为lameresource的所在路径
set(SRC_DIR src/main/cpp/lameresourse)

#设定一个目录
aux_source_directory(src/main/cpp/lameresource SRC_LIST)

#将目录下所有文件都添加进去

add_library(lameresource SHARED src/main/cpp/LameSDK.cpp ${SRC_LIST})

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
target_link_libraries( # Specifies the target library.
                       lameresource

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )