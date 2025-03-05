cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (lab1)

# option(IS_FLOAT "True if you want to compile with float version of array" False)

# # set(IS_FLOAT 0)

# # configure_file (config.h.in config.h @ONLY)
# # add_executable(lab1 lab1.cpp)
# # include_directories ($ {CMAKE_CURRENT_BINARY_DIR})

# if (IS_FLOAT)
#     add_executable(lab1 lab1_float.cpp)
# else()
#     add_executable(lab1 lab1_double.cpp)
# endif()

###############################################

# if (IS_FLOAT)
#     # include(FetchContent)
#     # FetchContent_Declare(googletest
#     # URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#     # )
#     # For Windows: Prevent overriding the parent project's compiler/linker settings
#     # set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#     # FetchContent_MakeAvailable(googletest)

#     # Добавляем библиотеку bitarray
#     # add_library(time_lib time2.cpp)

#     # Добавляем тестовое приложение
#     add_executable(lab1 lab1.cpp)

#     # configure_file (config.h.in config.h @ONLY)
#     # Связываем тестовое приложение с библиотекой и GoogleTest
#     # target_link_libraries(time_test PRIVATE time_lib GTest::gtest_main)

#     # Находим тесты
#     # include(GoogleTest)
#     # gtest_discover_tests(time_test)

# else()
#     # add_library(time_lib time2.cpp)

#     add_executable(lab1 lab1.cpp)

#     # target_link_libraries(time_app PRIVATE time_lib)
# endif()

#######################################

option(IS_FLOAT "True if you want to compile with float version of array" False)

include_directories(build)

if (IS_FLOAT)
    set(IS_FLOATT true)
else()
    set(IS_FLOATT false)
endif()

# configure_file (config.h.in config.h @ONLY)
add_executable (lab1 lab1.cpp)

# if (IS_FLOAT)
#     target_compile_definitions(lab1 PRIVATE "IS_FLOAT")
# endif()
add_definitions(-DIS_FLOAT=${IS_FLOATT})

#-6.76916e-10      -0.213894
