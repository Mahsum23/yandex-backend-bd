cmake_minimum_required(VERSION 3.2.0)
project(cppREST VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
find_package(PostgreSQL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/asio/asio/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Crow/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/json/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/json/include/nlohmann)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/taopq/include/pq)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/taopq/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libpq)
include_directories("C:\\Program Files\\PostgreSQL\\14\\include")
include_directories("C:\\Program Files\\PostgreSQL\\14\\include\\libpq")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/taopq)


add_executable(cppREST main.cpp file_json.h file_json.cpp)
target_link_libraries(cppREST taocpp::taopq)