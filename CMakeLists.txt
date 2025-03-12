cmake_minimum_required(VERSION 3.10)
project(PostgresExample)

set(CMAKE_CXX_STANDARD 14)

# Указываем путь к библиотеке libpq
set(POSTGRESQL_DIR "C:/Program Files/PostgreSQL/17")
include_directories(${POSTGRESQL_DIR}/include)
link_directories(${POSTGRESQL_DIR}/lib)

add_executable(PostgresExample main.cpp)

# Подключаем библиотеку libpq
target_link_libraries(PostgresExample libpq)