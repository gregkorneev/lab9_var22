cmake_minimum_required(VERSION 3.10)

# Название проекта
project(sql_query_optimizer LANGUAGES CXX)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Режим сборки по умолчанию
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Подключаем папку с заголовками
include_directories(${CMAKE_SOURCE_DIR}/include)

# Исполняемый файл и исходники
add_executable(sql_query_optimizer
    src/main.cpp
    src/query_model.cpp
    src/algorithms.cpp
)
