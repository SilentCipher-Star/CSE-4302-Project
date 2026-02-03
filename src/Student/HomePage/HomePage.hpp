#pragma once
#include <string>
#include <memory>
#include "../Routine/Routine.hpp"
#include "../Habit_Tracker/Habit_Tracker.hpp"

using namespace std;

class HomePage
{
private:
    string studentName;
    string studentID;
    int semester;
    string email;
    
public:
    HomePage(string name, string id, int sem, string email);
    ~HomePage() = default;
    
    void display(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker);
    
private:
    void showWelcomeBanner();
    void showAcademicInfo();
    void showTodayHighlights(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker);
    void showQuickStats(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker);
    void showUpcomingEvents(unique_ptr<Routine>& routine);
};
