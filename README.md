# Acadence

## 🎓 Introduction

**Acadence** is a robust, GUI-based Academic Management System designed to bridge the gap between students, teachers, and administrators. Built with **C++** and the **Qt 6 framework**, it serves as a personal academic assistant that streamlines daily educational tasks.

Whether you are a student trying to keep up with a busy schedule and maintain healthy study habits, a teacher managing grades and attendance for multiple courses, or an administrator overseeing the entire institution's data, Acadence provides a unified, role-based interface to handle it all.

The project emphasizes clean **Object-Oriented Programming (OOP)** principles, utilizing inheritance, polymorphism, and encapsulation to manage complex data relationships backed by a persistent CSV storage system.

## 🚀 Key Features

### 👤 For Students
*   **Dashboard:** View real-time stats, profile details, notices, and the next upcoming class.
*   **Study Planner:** A To-Do list manager integrated with a **Focus Timer** (Pomodoro style) to boost productivity.
*   **Habit Tracker:** Track daily routines (like prayers) and custom habits. Supports both **Duration-based** (e.g., "Workout for 30 mins") and **Count-based** (e.g., "Drink 8 glasses of water") habits with visual streak tracking.
*   **Dynamic Routine:** View the weekly class schedule with real-time status updates (e.g., "Starting Soon", "In Progress").
*   **Academics:** Monitor attendance percentages, view assessment results, and track CGPA.
*   **Q&A:** Ask questions directly to teachers.

### 👨‍🏫 For Teachers
*   **Routine Management:** Schedule classes and manage room allocations.
*   **Assessment Creation:** Create quizzes, assignments, and exams for specific courses.
*   **Grading System:** Enter and save marks for students; the system automatically calculates averages and statistics.
*   **Attendance Manager:** Add new class dates and mark student attendance with a simple checkbox interface.
*   **Student Interaction:** Reply to student queries via the Q&A panel.

### 🛡️ For Administrators
*   **Database Control:** Full CRUD (Create, Read, Update, Delete) access to all system tables (Students, Teachers, Courses, etc.).
*   **Data Integrity:** Built-in validation for unique IDs, usernames, and password strength.
*   **Search & Filter:** Powerful filtering capabilities to find specific records quickly.

### 🎨 General Features
*   **Theming Engine:** Switch between modern themes like *Cyberpunk*, *Nord*, *Gruvbox*, *Solarized*, and *Monokai*.
*   **Security:** Role-based authentication and password management.
*   **Persistence:** All data is saved locally in CSV format, ensuring data remains available between sessions.

---

## 🛠️ Technical Architecture

### File Structure
The project is organized to separate interface, implementation, and data management:

*   **`src/`**: Contains all `.cpp` implementation files.
*   **`include/`**: Contains all `.hpp` header files.
*   **`data/`**: Stores the CSV databases (generated automatically at runtime).
*   **`fonts/`**: Custom fonts loaded by the application.
*   **`CMakeLists.txt`**: Build configuration file.

### Class Hierarchy & OOP Concepts

#### 1. Core Logic (`AcadenceManager`)
The central controller class that mediates between the UI and the data layer. It handles login logic, data retrieval, and updates.

#### 2. User Hierarchy (Inheritance)
*   **`Person` (Abstract Base)**: Defines common attributes (ID, Name, Email, Credentials).
*   **`Student`**: Extends `Person` with academic data (Department, Batch, Semester, GPA).
*   **`Teacher`**: Extends `Person` with faculty data (Department, Designation, Salary).
*   **`Admin`**: Extends `Person` for system management roles.

#### 3. Habit System (Polymorphism)
*   **`Habit` (Abstract Base)**: Defines the interface for tracking streaks and completion.
*   **`DurationHabit`**: Implements logic for time-based goals (uses `Timer`).
*   **`CountHabit`**: Implements logic for quantity-based goals.
*   *Polymorphism is used to store and process different habit types in a single list.*

#### 4. Data Handling
*   **`CsvHandler`**: A static utility class that handles low-level file I/O, parsing CSV lines, and escaping special characters.
*   **`CsvDelegate`**: A custom Qt delegate used in the Admin panel to provide specific editors (SpinBoxes, DateEdits) based on the column type being edited.

---

## 💻 Installation & Build

### Prerequisites
Ensure you have the following installed:
*   **C++ Compiler**: GCC, Clang, or MSVC supporting C++17.
*   **CMake**: Version 3.16 or higher.
*   **Qt 6**: Core, Gui, and Widgets modules.

### Building the Project

1.  **Clone or Extract** the project repository.
2.  **Create a build directory**:
    ```bash
    mkdir build
    cd build
    ```
3.  **Configure with CMake**:
    ```bash
    cmake ..
    ```
4.  **Compile**:
    ```bash
    make  # On Linux/macOS
    # OR
    nmake # On Windows (MSVC)
    ```
5.  **Run**:
    ```bash
    ./Acadence # On Linux/macOS
    # OR
    Acadence.exe # On Windows
    ```

---

## 📖 Usage Guide

### Initial Login
When you run the application for the first time, it will generate the necessary data files in a `data/` folder relative to the executable.

**Default Admin Credentials:**
*   **Username:** `admin`
*   **Password:** `admin`

*Note: You can create Student and Teacher accounts via the Admin Panel after logging in as an admin.*

### Navigation
The application uses a Tabbed Interface. However, tabs are dynamically hidden or shown based on your role:

1.  **Dashboard:** The landing page for all users.
2.  **Study Planner (Student):** Add tasks and use the Focus Timer.
3.  **Habit Tracker (Student):** Manage daily goals.
4.  **Routine:** View (Student) or Manage (Teacher) weekly schedules.
5.  **Academics/Grading:** View grades (Student) or Enter grades (Teacher).
6.  **Admin Panel (Admin):** Select a table from the dropdown to view and edit raw system data.

---

## 📂 Data Management

Data is stored in CSV files to ensure simplicity and portability.

| File | Description |
| :--- | :--- |
| `admins.csv` | System administrator credentials. |
| `students.csv` | Student profiles, including GPA and semester info. |
| `teachers.csv` | Teacher profiles, departments, and salaries. |
| `courses.csv` | Academic courses linked to teachers and semesters. |
| `routine.csv` | Class schedules (Day, Time, Room, Course). |
| `grades.csv` | Student marks for specific assessments. |
| `attendance.csv` | Attendance records per course and date. |
| `habits.csv` | User-defined habits and tracking data. |
| `tasks.csv` | To-do list items. |
| `notices.csv` | Public announcements posted by teachers/admins. |
| `queries.csv` | Q&A threads between students and teachers. |
| `assessments.csv` | Definitions of quizzes/exams created by teachers. |
| `prayers.csv` | Daily prayer tracking logs. |

---

## 🤝 Contributing

1.  Fork the repository.
2.  Create a feature branch (`git checkout -b feature/NewFeature`).
3.  Commit your changes (`git commit -m 'Add some NewFeature'`).
4.  Push to the branch (`git push origin feature/NewFeature`).
5.  Open a Pull Request.

---

## 📜 License

This project is open-source and available for educational purposes.