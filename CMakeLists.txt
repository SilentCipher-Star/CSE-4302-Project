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
    src/csvdelegate.cpp
    include/csvdelegate.hpp
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
    include/appmanager.hpp
    src/appmanager.cpp
    src/appmanager_analytics.cpp
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
    include/databasemanager.hpp
    src/databasemanager.cpp
    include/logindialog.hpp
    src/logindialog.cpp
    include/theme.hpp
    src/theme.cpp
    include/manager_auth.hpp
    src/manager_auth.cpp
    include/manager_notices.hpp
    src/manager_notices.cpp
    include/manager_persons.hpp
    src/manager_persons.cpp
    include/personfactory.hpp
    src/personfactory.cpp
    include/manager_prayers.hpp
    src/manager_prayers.cpp
    include/manager_routine.hpp
    src/manager_routine.cpp
    include/manager_academics.hpp
    src/manager_academics.cpp
    include/manager_queries.hpp
    src/manager_queries.cpp
    include/manager_tasks.hpp
    src/manager_tasks.cpp
    include/manager_habits.hpp
    src/manager_habits.cpp
    include/notifications.hpp
    include/notice_decorators.hpp
    include/chart_widget.hpp
    include/ui_dashboard.hpp
    src/ui_dashboard.cpp
    include/ui_planner.hpp
    src/ui_planner.cpp
    include/ui_habits.hpp
    src/ui_habits.cpp
    include/ui_timers.hpp
    src/ui_timers.cpp
    include/ui_routine.hpp
    src/ui_routine.cpp
    include/ui_academics.hpp
    src/ui_academics.cpp
    include/gpa_strategy.hpp
    include/report.hpp
    include/student_iterator.hpp
    include/attendance_simulator.hpp
    include/ui_tamagotchi.hpp
    src/ui_tamagotchi.cpp
    include/ui_queries.hpp
    src/ui_queries.cpp
    include/ui_admin.hpp
    src/ui_admin.cpp
    include/ui_teacher_attendance.hpp
    src/ui_teacher_attendance.cpp
    include/manager_messages.hpp
    src/manager_messages.cpp
    include/ui_personal_inbox.hpp
    src/ui_personal_inbox.cpp
    include/ui_calendar.hpp
    src/ui_calendar.cpp
    include/theme_toggle.hpp
)

target_link_libraries(Acadence PRIVATE Qt6::Widgets)
target_include_directories(Acadence PRIVATE include)