# Acadence

## 🎓 Introduction

**Acadence** is a role-based desktop Academic Management System built with **C++17** and **Qt 6 Widgets**. It provides a unified interface for students, teachers, and administrators, while persisting data in CSV files.

The codebase is intentionally OOP-heavy and demonstrates multiple design patterns across real feature modules (authentication, routine planning, notices, academics, reports, and admin operations).

## 🚀 Key Features

### 👤 Student Features
- Dashboard with GPA, attendance, task stats, notices, and upcoming class context.
- Study planner with task tracking and completion state.
- Focus/workout timers integrated with habit workflows.
- Habit tracking with streak logic for `DurationHabit`, `CountHabit`, and `WorkoutHabit`.
- Daily prayer checklist persistence.
- Effective routine view that respects cancellations and reschedules.
- Academics panel with per-course attendance/grade status and risk coloring.
- GPA calculator with strategy selection and what-if simulation for remaining courses.
- Report export in both CSV and plain-text formats.
- Q&A flow to ask teachers directly.

### 👨‍🏫 Teacher Features
- Routine management, including class cancellation/rescheduling and exchange options.
- Assessment creation and grade entry with validation.
- Attendance date creation and per-student attendance marking.
- Automated attendance-warning notice generation for low-attendance students.
- Query response workflow for student questions.

### 🛡️ Admin Features
- CSV-backed table management via editable grid (CRUD-style operations).
- Built-in table filtering/search through proxy model.
- Specialized editors via delegate for safer data entry.
- Control over core institutional data tables from one panel.

### 🎨 Cross-Cutting Features
- Role-based authentication (`Admin`, `Student`, `Teacher`).
- Password change flow.
- Theme support plus light/dark toggle behavior in the main window.
- Notice rendering with badges/highlight rules (urgent, pinned, expiring).
- Attendance chart visualization for student dashboard.
- CSV backup support through `DatabaseManager::backupCsvData`.

## 🛠️ Technical Architecture

### Project Layout
- `src/`: Implementation (`.cpp`) files, including UI modules and managers.
- `include/`: Public headers (`.hpp`) and core abstractions.
- `assets/data/`: CSV storage files.
- `assets/fonts/`: Runtime font assets loaded by utility helpers.
- `.Project/`: Internal project docs and Mermaid diagrams (`*.mmd`, guides).
- `CMakeLists.txt`: Build target definition.

### Core Orchestration
- `AcadenceManager` in `include/appmanager.hpp` is the facade between UI modules and domain managers.
- Manager modules (`manager_auth`, `manager_academics`, `manager_routine`, etc.) isolate domain-specific operations.
- `MainWindow` wires role-dependent tabs and module lifecycles.

### OOP and Pattern Usage
- Inheritance: `Person` -> `Student`, `Teacher`, `Admin`.
- Polymorphism: `Habit` hierarchy with specialized progress serialization.
- Observer Pattern: `IDataObserver` and `DataType` notifications from `AcadenceManager`.
- Strategy Pattern: `IGPAStrategy` (`PercentageGPAStrategy`, `LetterGradeGPAStrategy`).
- Decorator Pattern: Notice decorators (`UrgentNotice`, `PinnedNotice`, `ExpiringNotice`).
- Template Method Pattern: `IReport` with `CSVReport` and `TextReport` implementations.
- Iterator Pattern: `StudentIterator`/`StudentCollection` used in grading flow.
- Factory-style creation helpers: `PersonFactory` and decorated notice builder helpers.

## 💻 Installation and Build

### Prerequisites
- C++17 compiler (GCC/Clang/MSVC).
- CMake `>= 3.16`.
- Qt 6 with **Widgets** module.

### Build Steps

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

### Run

From the build directory (target name from `CMakeLists.txt`):

```bash
./Acadence
```

On Windows, run `Acadence.exe`.

## 📖 Usage Notes

### First Run Behavior
- On startup, the app initializes the CSV store under the executable-relative data path (`../assets/data`).
- It seeds default admin credentials if missing:
  - Username: `admin`
  - Password: `admin`

### Role-Based Navigation
- Student: dashboard, planner, habits/timers/prayers, routine view, academics, queries.
- Teacher: dashboard, routine management, assessment/grading/attendance tools, queries.
- Admin: dashboard notice tools plus admin data-management panel.

## 📂 Data Files

The project persists data in CSV format. Current files present in the repository include:

| File | Purpose |
| :--- | :--- |
| `admins.csv` | Admin credentials and profile basics. |
| `students.csv` | Student identity, login, department, semester, CGPA. |
| `teachers.csv` | Teacher identity, login, department, designation, salary. |
| `courses.csv` | Course metadata, teacher mapping, semester, credits. |
| `enrollments.csv` | Student-course enrollment mappings. |
| `routine.csv` | Baseline weekly class routine. |
| `routine_adjustments.csv` | Routine overrides (`CANCEL`/`RESCHEDULE`). |
| `attendance.csv` | Attendance entries by student/course/date. |
| `assessments.csv` | Assessment definitions (course/title/type/date/max marks). |
| `grades.csv` | Marks per student-assessment pair. |
| `tasks.csv` | Student planner tasks. |
| `habits.csv` | Habit definitions and serialized progress. |
| `prayers.csv` | Daily prayer completion records. |
| `queries.csv` | Student-teacher Q&A records. |
| `notices.csv` | Posted notice records consumed by dashboard feed. |
| `themes.csv` | Theme-related persisted data. |

Additional generated/report artifact in project root:

| File | Purpose |
| :--- | :--- |
| `academic_report.csv` | Example exported academic report output. |

## 🧩 Developer Notes

- Some legacy/backup source files are present in `src/` (for example `.bak`, `.old`, `.backup` variants).
- Working implementation in CMake currently points to `src/logindialog_new.cpp` for login dialog logic.

## 🤝 Contributing

1. Fork the repository.
2. Create a branch: `git checkout -b feature/your-change`.
3. Commit: `git commit -m "Describe your change"`.
4. Push your branch.
5. Open a pull request.

## 📜 License

Open-source project intended for academic and educational use.