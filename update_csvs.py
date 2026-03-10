import csv
import sys
import os

sys.stdout.reconfigure(encoding='utf-8')

DATA_DIR = os.path.join(os.path.dirname(__file__), 'data')

# Teacher ID -> Full Name (from teachers.csv)
teacher_names = {
    101: 'Prof. Dr. Md. Hasanul Kabir',
    102: 'Prof. Dr. Muhammad Mahbub Alam',
    103: 'Prof. Dr. Abu Raihan Mostofa Kamal',
    104: 'Prof. Dr. Md. Kamrul Hasan',
    105: 'Prof. Dr. Hasan Mahmud',
    106: 'Prof. Dr. Md. Sakhawat Hossen',
    107: 'Dr. Md. Azam Hossain',
    108: 'Tareque Mohmud Chowdhury',
    109: 'Shohel Ahmed',
    110: 'Dr. Md Moniruzzaman',
    111: 'Lutfun Nahar Lota',
    112: 'Ashraful Alam Khan',
    113: 'Faisal Hussain',
    114: 'Sabbir Ahmed',
    115: 'Md. Bakhtiar Hasan',
    116: 'Shahriar Ivan',
    117: 'Njayou Youssef',
    118: 'Tanjila Alam Sathi',
    119: 'S. M. Sabit Bananee',
    120: 'Mohammad Ishrak Abedin',
    121: 'Jibon Naher',
    122: 'Md. Rafid Haque',
    123: 'Mueeze Al Mushabbir',
    124: 'Asaduzzaman Herok',
    125: 'Ishmam Tashdeed',
    126: 'Farzana Tabassum',
    127: 'Sabrina Islam',
    128: 'MD Atiqur Rahman',
    129: 'Syed Rifat Raiyan',
    130: 'Aashnan Rahman',
    131: 'Reaz Hassan Joarder',
    132: 'Ajwad Abrar Mostofa',
    133: 'Md. Tariquzzaman',
    134: 'Md. Tanvir Hossain Saikat',
}

# Course code -> teacher_id mapping from the PDF schedule
# Based on BSc CSE class schedule from IUT
course_teacher_map = {
    # Semester 1
    'Hum 4145': 110,   # Islamiat
    'Hum 4147': 117,   # Technology, Environment and Society
    'Math 4141': 125,  # Geometry and Differential Calculus
    'Phy 4141': 104,   # Physics I
    'CSE 4105': 104,   # Computing for Engineers - KH
    'CSE 4107': 110,   # Structured Programming I - MM
    'Hum 4142': 110,   # Arabic I
    'Hum 4144': 129,   # English I
    'Phy 4142': 107,   # Physics I Lab
    'CSE 4104': 132,   # Engineering Drawing Lab - AA
    'CSE 4108': 110,   # Structured Programming I Lab - MM/SRR

    # Semester 2
    'Hum 4241': 103,   # Islamic History Science and Culture
    'Math 4241': 125,  # Integral Calculus and Differential Equations
    'Phy 4241': 104,   # Physics II
    'Chem 4241': 107,  # Chemistry
    'CSE 4203': 119,   # Discrete Mathematics
    'CSE 4205': 120,   # Digital Logic Design
    'Hum 4242': 110,   # Arabic II
    'Hum 4244': 129,   # English II
    'Phy 4242': 107,   # Physics II Lab
    'Chem 4242': 107,  # Chemistry Lab
    'CSE 4202': 131,   # Structured Programming II Lab
    'CSE 4206': 120,   # Digital Logic Design Lab

    # Semester 3 (from PDF - CSE 3rd Section 1 & 2)
    'Math 4341': 125,  # Linear Algebra - IT (Ishmam Tashdeed)
    'EEE 4383': 108,   # Electronic Devices and Circuits - HR(PT), using TMC
    'CSE 4301': 113,   # Object Oriented Programming - FH (Faisal Hussain)
    'CSE 4303': 114,   # Data Structures - SA (Sabbir Ahmed)
    'CSE 4305': 120,   # Computer Organization and Architecture - MIA
    'CSE 4307': 103,   # Database Management Systems - ARMK
    'EEE 4384': 108,   # Electronic Devices and Circuits Lab
    'CSE 4302': 113,   # Object Oriented Programming Lab - FH/FT
    'CSE 4304': 130,   # Data Structures Lab - AAR
    'CSE 4308': 133,   # Database Management Systems Lab - SN/TZF

    # Semester 4
    'Hum 4441': 111,   # Engineering Ethics
    'Math 4441': 125,  # Probability and Statistics
    'EEE 4483': 108,   # Digital Electronics and Pulse Techniques
    'CSE 4403': 114,   # Algorithms - SA
    'CSE 4405': 134,   # Data and Telecommunications
    'CSE 4407': 127,   # System Analysis and Design
    'EEE 4484': 108,   # Digital Electronics and Pulse Techniques Lab
    'CSE 4402': 126,   # Visual Programming Lab
    'CSE 4404': 114,   # Algorithms Lab
    'CSE 4408': 127,   # System Analysis and Design Lab

    # Semester 5 (from PDF - CSE 5th Section 1 & 2)
    'CSE 4501': 121,   # Operating Systems - JN (Jibon Naher)
    'CSE 4503': 134,   # Microprocessor and Assembly Language - OR(PT), using THS
    'CSE 4511': 134,   # Computer Networks - THS
    'CSE 4513': 132,   # Software Eng & OOD - AA (Ajwad Abrar Mostofa)
    'CSE 4502': 121,   # Operating Systems Lab - JN
    'CSE 4504': 130,   # Microprocessor Lab - AAR
    'CSE 4508': 127,   # RDBMS Programming Lab - SBI/MAMR
    'CSE 4510': 107,   # Software Development - AZM/SAH
    'CSE 4512': 134,   # Computer Networks Lab - THS
    'Math 4541': 130,  # Multivariable Calculus - AKA(NSc), using AAR
    'CSE 4531': 112,   # E-Commerce and Web Security - AAK
    'CSE 4537': 112,   # Decision Support Systems - AAK
    'CSE 4547': 128,   # Parallel and Distributed Systems
    'CSE 4539': 126,   # Web Programming - FT
    'CSE 4543': 128,   # Geographical Information Systems
    'CSE 4549': 128,   # Simulation and Modeling - ARN
    'CSE 4551': 105,   # Computer Graphics - HM
    'CSE 4540': 126,   # Web Programming Lab - FT
    'CSE 4544': 128,   # GIS Lab
    'CSE 4550': 128,   # Simulation Lab - ARN
    'CSE 4552': 105,   # Computer Graphics Lab

    # Semester 6
    'Hum 4641': 103,   # Accounting
    'CSE 4615': 134,   # Wireless Networks
    'CSE 4619': 120,   # Peripherals and Interfacing
    'CSE 4621': 105,   # Machine Learning - HM
    'CSE 4610': 132,   # Design Project
    'CSE 4614': 111,   # Technical Report Writing
    'CSE 4616': 134,   # Wireless Networks Lab
    'CSE 4620': 120,   # Peripherals and Interfacing Lab
    'CSE 4622': 105,   # Machine Learning Lab
    'Math 4641': 125,  # Numerical Methods
    'CSE 4641': 119,   # Distributed Operating Systems
    'CSE 4643': 120,   # Mobile Application Development
    'CSE 4647': 133,   # Distributed Database Systems
    'Math 4642': 125,  # Numerical Methods Lab
    'CSE 4642': 119,   # Distributed OS Lab
    'CSE 4644': 120,   # Mobile App Dev Lab
    'CSE 4648': 133,   # Distributed Database Lab
    'CSE 4631': 128,   # Digital Signal Processing
    'CSE 4635': 126,   # Web Architecture
    'CSE 4649': 124,   # Systems Programming
    'CSE 4651': 117,   # Unix Programming
    'CSE 4632': 128,   # DSP Lab
    'CSE 4636': 126,   # Web Architecture Lab
    'CSE 4650': 124,   # Systems Programming Lab
    'CSE 4652': 117,   # Unix Programming Lab

    # Semester 7 (from PDF - CSE 7th Section 1 & 2)
    'Hum 4741': 111,   # Business Communication and Law
    'Hum 4743': 103,   # Engineering Economics
    'Hum 4745': 103,   # International Relationship
    'Math 4741': 102,  # Mathematical Analysis - MMA
    'CSE 4703': 107,   # Theory of Computing - AZM
    'CSE 4711': 105,   # Artificial Intelligence - FHS(PT), using HM
    'CSE 4712': 126,   # AI Lab - AFZ
    'CSE 4700': 101,   # Project/Thesis - HK
    'CSE 4790': 101,   # Industrial Training
    'CSE 4739': 123,   # Data Mining - MAMR
    'CSE 4743': 117,   # Cryptography and Network Security - NY
    'CSE 4745': 120,   # Embedded Systems Design
    'CSE 4747': 108,   # Computational Biology
    'CSE 4733': 101,   # Digital Image Processing - HK
    'CSE 4735': 123,   # Digital Systems Design
    'CSE 4749': 117,   # Introduction to Cloud Computing - NY
    'CSE 4751': 108,   # Network Programming
    'CSE 4753': 108,   # Bioinformatics - TMC
    'CSE 4734': 101,   # DIP Lab - HK
    'CSE 4736': 123,   # Digital Systems Design Lab
    'CSE 4750': 117,   # Cloud Computing Lab - NY
    'CSE 4752': 108,   # Network Programming Lab
    'CSE 4754': 108,   # Bioinformatics Lab - TMC

    # Semester 8
    'CSE 4801': 106,   # Compiler Design
    'CSE 4803': 107,   # Graph Theory
    'CSE 4807': 112,   # IT Organization and Management
    'CSE 4809': 114,   # Algorithm Engineering
    'CSE 4802': 106,   # Compiler Design Lab
    'CSE 4810': 114,   # Algorithm Engineering Lab
    'CSE 4800': 101,   # Project/Thesis
    'CSE 4841': 125,   # Introduction to Optimization
    'CSE 4845': 123,   # Introduction to Information Retrieval
    'CSE 4847': 127,   # Information and OS Security
    'CSE 4849': 115,   # Human Computer Interaction
    'CSE 4851': 113,   # Design Pattern
    'CSE 4833': 120,   # VLSI Design and Testing
    'CSE 4835': 101,   # Pattern Recognition
    'CSE 4839': 134,   # Internetworking Protocols
    'CSE 4834': 120,   # VLSI Lab
    'CSE 4836': 101,   # Pattern Recognition Lab
    'CSE 4840': 134,   # Internetworking Protocols Lab
}

# ============================================================
# 1. Update courses.csv - change teacher_id column
# ============================================================
courses_path = os.path.join(DATA_DIR, 'courses.csv')
rows = []
with open(courses_path, 'r', encoding='utf-8') as f:
    content = f.read()

lines = content.strip().split('\n')
updated_count = 0
new_lines = []
for line in lines:
    parts = [p.strip() for p in line.split(',')]
    # Format: id, course_code, course_name, teacher_id, semester, credit_hours
    # But course_name might have commas if quoted
    # Let's use csv reader
    pass

# Use csv module for proper parsing
import io
reader = csv.reader(io.StringIO(content))
rows = list(reader)

new_rows = []
for row in rows:
    if len(row) >= 6:
        course_code = row[1].strip()
        if course_code in course_teacher_map:
            old_tid = row[3].strip()
            new_tid = str(course_teacher_map[course_code])
            if old_tid != new_tid:
                updated_count += 1
            row[3] = new_tid
    new_rows.append(row)

with open(courses_path, 'w', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(new_rows)

print(f"courses.csv: Updated {updated_count} teacher assignments")

# ============================================================
# 2. Update routine.csv - replace fake teacher names with real ones
# ============================================================
routine_path = os.path.join(DATA_DIR, 'routine.csv')
with open(routine_path, 'r', encoding='utf-8') as f:
    routine_content = f.read()

reader = csv.reader(io.StringIO(routine_content))
routine_rows = list(reader)

# Format: day, period, room, course_name, course_code, teacher_name, semester
routine_updated = 0
new_routine_rows = []
for row in routine_rows:
    if len(row) >= 7:
        course_code = row[4].strip()
        if course_code in course_teacher_map:
            tid = course_teacher_map[course_code]
            real_name = teacher_names.get(tid, row[5].strip())
            if row[5].strip() != real_name:
                routine_updated += 1
            row[5] = real_name
    new_routine_rows.append(row)

with open(routine_path, 'w', encoding='utf-8', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(new_routine_rows)

print(f"routine.csv: Updated {routine_updated} teacher names")
print("Done!")
