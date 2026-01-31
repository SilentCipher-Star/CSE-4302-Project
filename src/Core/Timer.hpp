#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

class Timer
{
public:
    // Returns true if completed, false if stopped
    bool start(int minutes)
    {
        if (minutes <= 0)
            return false;

        // Setup terminal for non-blocking input
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        // Clear input buffer
        char trash;
        while (read(STDIN_FILENO, &trash, 1) > 0)
            ;

        long long totalDeciSeconds = minutes * 60 * 10;
        long long remaining = totalDeciSeconds;
        bool paused = true; // Start paused

        cout << "\n--- Focus Timer (" << minutes << " min) ---\n";
        cout << "Controls: [SPACE] Start/Pause/Resume  [S] Stop\n";
        cout << "Press SPACE to start...\n";

        bool completed = false;
        bool running = true;

        while (running)
        {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0)
            {
                if (c == ' ')
                {
                    paused = !paused;
                    if (paused)
                        cout << "\r[PAUSED] Press SPACE to resume...   " << flush;
                    else
                        cout << "\r                                   " << flush;
                }
                else if (c == 's' || c == 'S')
                {
                    cout << "\n[STOPPED] Session terminated.\n";
                    running = false;
                }
            }

            if (!paused && running)
            {
                int min = (remaining / 10) / 60;
                int sec = (remaining / 10) % 60;
                int deci = remaining % 10;

                cout << "\rTime Remaining: "
                     << setfill('0') << setw(2) << min << ":"
                     << setfill('0') << setw(2) << sec << "."
                     << deci << "   " << flush;

                if (remaining <= 0)
                {
                    cout << "\n\n[COMPLETE] Session finished successfully!\n";
                    completed = true;
                    running = false;
                }
                else
                {
                    remaining--;
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }
            else if (running)
            {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        return completed;
    }
};