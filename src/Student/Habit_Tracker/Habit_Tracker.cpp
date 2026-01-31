#include "Habit_Tracker.hpp"
#include <iomanip>

// --- Base Habit ---
Habit::Habit(string n, HabitType t, Frequency f)
    : name(n), type(t), frequency(f), streak(0), lastUpdated(time(0)), isCompleted(false) {}

void Habit::checkReset()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int currentDay = ltm->tm_yday;
    int currentYear = ltm->tm_year;

    tm *lastTm = localtime(&lastUpdated);
    int lastDay = lastTm->tm_yday;
    int lastYear = lastTm->tm_year;

    bool reset = false;
    if (frequency == Frequency::DAILY)
    {
        if (currentDay != lastDay || currentYear != lastYear)
            reset = true;
    }
    else if (frequency == Frequency::WEEKLY)
    {
        if (currentYear != lastYear || (currentDay / 7) != (lastDay / 7))
            reset = true;
    }

    if (reset)
    {
        if (!isCompleted && streak > 0)
            streak = 0; // Reset streak if missed
        isCompleted = false;
        lastUpdated = now;
        // Derived classes handle their specific resets in perform() or display() if needed,
        // but ideally we'd have a virtual reset method. For MVP, we reset flags here.
    }
}

void Habit::markComplete()
{
    if (!isCompleted)
    {
        isCompleted = true;
        streak++;
        cout << "Habit '" << name << "' completed! Streak: " << streak << endl;
    }
}

string Habit::getFrequencyString() const { return (frequency == Frequency::DAILY) ? "Daily" : "Weekly"; }
string Habit::getTypeString() const
{
    switch (type)
    {
    case HabitType::DURATION:
        return "Duration";
    case HabitType::PRAYER:
        return "Prayer";
    case HabitType::COUNT:
        return "Count";
    default:
        return "Unknown";
    }
}

// --- Duration Habit ---
DurationHabit::DurationHabit(string n, Frequency f, int target)
    : Habit(n, HabitType::DURATION, f), targetMinutes(target), currentMinutes(0) {}

void DurationHabit::display() const
{
    cout << "[Timer] " << name << " | " << getFrequencyString()
         << " | Progress: " << currentMinutes << "/" << targetMinutes << " mins"
         << " | Streak: " << streak << (isCompleted ? " [DONE]" : "") << endl;
}

void DurationHabit::perform()
{
    checkReset();
    if (isCompleted)
    {
        cout << "Already completed for this period.\n";
        return;
    }

    cout << "Start timer for " << name << "? (Target: " << targetMinutes << " mins)\n";
    int mins;
    cout << "Enter minutes to perform now: ";
    cin >> mins;

    Timer timer;
    if (timer.start(mins))
    {
        currentMinutes += mins;
        if (currentMinutes >= targetMinutes)
        {
            currentMinutes = targetMinutes;
            markComplete();
        }
        else
        {
            cout << "Progress updated: " << currentMinutes << "/" << targetMinutes << " mins.\n";
        }
    }
}

string DurationHabit::serialize() const
{
    return "DURATION " + name + " " + to_string((int)frequency) + " " + to_string(streak) + " " + to_string(lastUpdated) + " " + to_string(isCompleted) + " " + to_string(targetMinutes) + " " + to_string(currentMinutes);
}

void DurationHabit::deserialize(stringstream &ss)
{
    int f;
    ss >> f >> streak >> lastUpdated >> isCompleted >> targetMinutes >> currentMinutes;
    frequency = (Frequency)f;
}

// --- Count Habit ---
CountHabit::CountHabit(string n, Frequency f, int target, string u)
    : Habit(n, HabitType::COUNT, f), targetCount(target), currentCount(0), unit(u) {}

void CountHabit::display() const
{
    cout << "[Count] " << name << " | " << getFrequencyString()
         << " | Progress: " << currentCount << "/" << targetCount << " " << unit
         << " | Streak: " << streak << (isCompleted ? " [DONE]" : "") << endl;
}

void CountHabit::perform()
{
    checkReset();
    if (isCompleted)
    {
        cout << "Already completed for this period.\n";
        return;
    }

    int add;
    cout << "Add to count (" << unit << "): ";
    cin >> add;
    currentCount += add;
    if (currentCount >= targetCount)
    {
        currentCount = targetCount;
        markComplete();
    }
    else
    {
        cout << "Progress updated: " << currentCount << "/" << targetCount << " " << unit << ".\n";
    }
}

string CountHabit::serialize() const
{
    return "COUNT " + name + " " + to_string((int)frequency) + " " + to_string(streak) + " " + to_string(lastUpdated) + " " + to_string(isCompleted) + " " + to_string(targetCount) + " " + to_string(currentCount) + " " + unit;
}

void CountHabit::deserialize(stringstream &ss)
{
    int f;
    ss >> f >> streak >> lastUpdated >> isCompleted >> targetCount >> currentCount >> unit;
    frequency = (Frequency)f;
}

// --- Prayer Habit ---
PrayerHabit::PrayerHabit() : Habit("Daily Prayers", HabitType::PRAYER, Frequency::DAILY)
{
    for (int i = 0; i < 5; i++)
        prayers[i] = false;
}

void PrayerHabit::display() const
{
    string pNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
    cout << "[Prayer] Daily Prayers | Streak: " << streak << (isCompleted ? " [DONE]" : "") << endl;
    for (int i = 0; i < 5; i++)
    {
        cout << "  " << pNames[i] << ": " << (prayers[i] ? "[X]" : "[ ]") << " ";
    }
    cout << endl;
}

void PrayerHabit::perform()
{
    checkReset();
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int hour = ltm->tm_hour;

    string pNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
    // Simple windows: Fajr 5-6, Dhuhr 13-16, Asr 16-18, Maghrib 18-19, Isha 20-23
    int currentIdx = -1;
    if (hour >= 5 && hour < 7)
        currentIdx = 0;
    else if (hour >= 13 && hour < 16)
        currentIdx = 1;
    else if (hour >= 16 && hour < 18)
        currentIdx = 2;
    else if (hour >= 18 && hour < 20)
        currentIdx = 3;
    else if (hour >= 20 && hour <= 23)
        currentIdx = 4;

    cout << "Current Time: " << hour << ":00. ";
    if (currentIdx != -1)
        cout << "It is time for " << pNames[currentIdx] << ".\n";
    else
        cout << "No specific prayer time now.\n";

    cout << "Check-in Prayer:\n";
    for (int i = 0; i < 5; i++)
        cout << i + 1 << ". " << pNames[i] << (prayers[i] ? " (Done)" : "") << endl;
    cout << "0. Back\nChoice: ";
    int c = getInt();
    if (c >= 1 && c <= 5)
    {
        if (!prayers[c - 1])
        {
            prayers[c - 1] = true;
            cout << "Checked in " << pNames[c - 1] << ".\n";

            bool all = true;
            for (int i = 0; i < 5; i++)
                if (!prayers[i])
                    all = false;
            if (all)
                markComplete();
        }
        else
            cout << "Already checked in.\n";
    }
}

string PrayerHabit::serialize() const
{
    string s = "PRAYER " + name + " " + to_string((int)frequency) + " " + to_string(streak) + " " + to_string(lastUpdated) + " " + to_string(isCompleted);
    for (int i = 0; i < 5; i++)
        s += " " + to_string(prayers[i]);
    return s;
}

void PrayerHabit::deserialize(stringstream &ss)
{
    int f;
    ss >> f >> streak >> lastUpdated >> isCompleted;
    frequency = (Frequency)f;
    for (int i = 0; i < 5; i++)
        ss >> prayers[i];
}

// --- Habit Tracker Manager ---
HabitTracker::HabitTracker(string user) : username(user) { loadHabits(); }

void HabitTracker::loadHabits()
{
    habits.clear();
    ifstream file(getFileName());
    if (!file.is_open())
        return;

    string line, typeStr, name;
    while (getline(file, line))
    {
        stringstream ss(line);
        ss >> typeStr >> name;
        shared_ptr<Habit> h;

        if (typeStr == "DURATION")
            h = make_shared<DurationHabit>(name, Frequency::DAILY, 0);
        else if (typeStr == "COUNT")
            h = make_shared<CountHabit>(name, Frequency::DAILY, 0, "");
        else if (typeStr == "PRAYER")
            h = make_shared<PrayerHabit>();

        if (h)
        {
            h->deserialize(ss);
            h->checkReset(); // Reset on load if day changed
            habits.push_back(h);
        }
    }
    file.close();
}

void HabitTracker::saveHabits()
{
    ofstream file(getFileName());
    for (auto &h : habits)
    {
        file << h->serialize() << endl;
    }
    file.close();
}

void HabitTracker::createHabit()
{
    cout << "\n--- Create Habit ---\n";
    cout << "1. Workout/Extracurricular (Timer based)\n";
    cout << "2. Hydration/Count (Counter based)\n";
    cout << "3. Daily Prayers (Check-in based)\n";
    cout << "Choice: ";
    int c = getInt();

    if (c == 3)
    {
        habits.push_back(make_shared<PrayerHabit>());
        cout << "Prayer habit added.\n";
        saveHabits();
        return;
    }

    string name;
    cout << "Habit Name: ";
    cin >> name;

    cout << "Frequency (1. Daily, 2. Weekly): ";
    int f = getIntInRange(1, 2);
    Frequency freq = (f == 1) ? Frequency::DAILY : Frequency::WEEKLY;

    if (c == 1)
    {
        cout << "Target Duration (minutes): ";
        int min = getInt();
        habits.push_back(make_shared<DurationHabit>(name, freq, min));
    }
    else if (c == 2)
    {
        cout << "Target Count: ";
        int cnt = getInt();
        string unit;
        cout << "Unit (e.g., glasses, pages): ";
        cin >> unit;
        habits.push_back(make_shared<CountHabit>(name, freq, cnt, unit));
    }
    saveHabits();
    cout << "Habit created.\n";
}

void HabitTracker::deleteHabit()
{
    viewHabits();
    cout << "Enter number to delete (0 to cancel): ";
    int idx = getInt();
    if (idx > 0 && idx <= (int)habits.size())
    {
        habits.erase(habits.begin() + idx - 1);
        saveHabits();
        cout << "Habit deleted.\n";
    }
}

void HabitTracker::viewHabits()
{
    cout << "\n--- Your Habits ---\n";
    if (habits.empty())
    {
        cout << "No habits found.\n";
        return;
    }
    int i = 1;
    for (auto &h : habits)
    {
        cout << i++ << ". ";
        h->display();
    }
}

void HabitTracker::checkReminders()
{
    cout << "\n[Reminders]\n";
    bool any = false;
    for (auto &h : habits)
    {
        if (!h->getCompleted())
        {
            cout << "! Reminder: You haven't completed '" << h->getName() << "' yet.\n";
            any = true;
        }
    }
    if (!any)
        cout << "All habits completed! Great job.\n";
}

void HabitTracker::menu()
{
    int choice;
    do
    {
        cout << "\n--- Habit Tracker ---\n";
        checkReminders();
        cout << "\n1. View Habits\n";
        cout << "2. Perform/Check-in Habit\n";
        cout << "3. Create Habit\n";
        cout << "4. Delete Habit\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        choice = getInt();

        switch (choice)
        {
        case 1:
            viewHabits();
            break;
        case 2:
            viewHabits();
            cout << "Enter number to perform: ";
            int idx;
            idx = getInt();
            if (idx > 0 && idx <= (int)habits.size())
            {
                habits[idx - 1]->perform();
                saveHabits();
            }
            break;
        case 3:
            createHabit();
            break;
        case 4:
            deleteHabit();
            break;
        }
    } while (choice != 0);
}
