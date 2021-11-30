project(clip2gltf2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("thirdparty/json/include")

add_executable(clip2gltf2 src/main.cpp)
