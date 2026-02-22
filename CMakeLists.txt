cmake_minimum_required(VERSION 3.16)

project(Acadence VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Acadence WIN32 MACOSX_BUNDLE
    main.cpp
    src/mainwindow.cpp
    include/mainwindow.hpp
    src/mainwindow.ui
    include/person.hpp
    src/person.cpp
    include/admin.hpp
    src/admin.cpp
    include/student.hpp
    src/student.cpp
    include/teacher.hpp
    src/teacher.cpp
    include/course.hpp
    src/course.cpp
    include/academicmanager.hpp
    src/academicmanager.cpp
    include/timer.hpp
    src/timer.cpp
    include/habit.hpp
    src/habit.cpp
    include/routine.hpp
    src/routine.cpp
    include/exceptions.hpp
    include/utils.hpp
    src/utils.cpp
    include/csvhandler.hpp
    src/csvhandler.cpp
    include/logindialog.hpp
    src/logindialog.cpp
    include/theme.hpp
    src/theme.cpp
)

target_link_libraries(Acadence PRIVATE Qt6::Widgets)
target_include_directories(Acadence PRIVATE include)