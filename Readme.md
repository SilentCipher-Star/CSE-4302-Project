# Acadence - Personal Academic Assistant

Acadence is a C++ console-based application designed to manage academic activities for Students, Teachers, and Administrators. It features a role-based login system and data persistence.

## Features

### Roles
1. **Admin**
   - Create new users (Student, Teacher).
   - Delete existing users.
   - Manage system access.

2. **Student**
   - View daily schedule.
   - Check class notices.
   - View grades.
   - Manage a study planner.

3. **Teacher**
   - Post class notices.
   - Assign grades to students.

## Getting Started

### Prerequisites
- C++ Compiler (C++17 standard)
- CMake

### Default Login
If no users exist, the system creates a default admin account:
- **Username:** `admin`
- **Password:** `admin`

## Data Storage
The application uses text files for data persistence:
- `users.txt`: Stores user credentials and profile info.
- `notices.txt`: Stores notices posted by teachers.
- `grades.txt`: Stores grades assigned to students.

## Project Structure
- `src/`: Source code organized by modules (App, Student, Teacher, Admin, Core).