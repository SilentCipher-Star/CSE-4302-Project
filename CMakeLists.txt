cmake_minimum_required(VERSION 3.16)

project(Acadence VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

add_executable(Acadence WIN32 MACOSX_BUNDLE
    # Main
    main.cpp

    # UI Forms
    src/mainwindow.ui

    # Headers
    include/admin.hpp
    include/appmanager.hpp
    include/assetmanager.hpp
    include/attendance_simulator.hpp
    include/chart_widget.hpp
    include/command_history.hpp
    include/commands.hpp
    include/course.hpp
    include/csvdelegate.hpp
    include/csvhandler.hpp
    include/custom_tables.hpp
    include/databasemanager.hpp
    include/exceptions.hpp
    include/gpa_strategy.hpp
    include/habit.hpp
    include/icommand.hpp
    include/logindialog.hpp
    include/mainwindow.hpp
    include/manager_academics.hpp
    include/manager_auth.hpp
    include/manager_community.hpp
    include/manager_persons.hpp
    include/manager_productivity.hpp
    include/models.hpp
    include/notice_decorators.hpp
    include/notifications.hpp
    include/person.hpp
    include/personfactory.hpp
    include/report.hpp
    include/routine.hpp
    include/student.hpp
    include/student_iterator.hpp
    include/teacher.hpp
    include/theme.hpp
    include/timer.hpp
    include/ui_academics.hpp
    include/ui_admin.hpp
    include/ui_calendar.hpp
    include/ui_dashboard.hpp
    include/ui_habits.hpp
    include/ui_lostfound.hpp
    include/ui_personal_inbox.hpp
    include/ui_planner.hpp
    include/ui_queries.hpp
    include/ui_routine.hpp
    include/ui_tamagotchi.hpp
    include/ui_teacher_attendance.hpp
    include/ui_timers.hpp
    include/utils.hpp

    # Sources
    src/admin.cpp
    src/appmanager.cpp
    src/appmanager_analytics.cpp
    src/assetmanager.cpp
    src/command_history.cpp
    src/course.cpp
    src/csvdelegate.cpp
    src/csvhandler.cpp
    src/custom_tables.cpp
    src/databasemanager.cpp
    src/habit.cpp
    src/logindialog.cpp
    src/mainwindow.cpp
    src/manager_academics.cpp
    src/manager_auth.cpp
    src/manager_community.cpp
    src/manager_persons.cpp
    src/manager_productivity.cpp
    src/person.cpp
    src/personfactory.cpp
    src/routine.cpp
    src/student.cpp
    src/teacher.cpp
    src/theme.cpp
    src/timer.cpp
    src/ui_academics.cpp
    src/ui_admin.cpp
    src/ui_calendar.cpp
    src/ui_dashboard.cpp
    src/ui_habits.cpp
    src/ui_lostfound.cpp
    src/ui_personal_inbox.cpp
    src/ui_planner.cpp
    src/ui_queries.cpp
    src/ui_routine.cpp
    src/ui_tamagotchi.cpp
    src/ui_teacher_attendance.cpp
    src/ui_timers.cpp
    src/utils.cpp
)

target_link_libraries(Acadence PRIVATE Qt6::Widgets Qt6::Multimedia)
target_include_directories(Acadence PRIVATE include)