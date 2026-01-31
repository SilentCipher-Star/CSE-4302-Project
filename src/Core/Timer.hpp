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
private:
    // Check if a key has been pressed (non-blocking)
    bool kbhit()
    {
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if (ch != EOF)
        {
            ungetc(ch, stdin);
            return true;
        }

        return false;
    }

    // Get a character without waiting for Enter
    char getch()
    {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
            perror("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("tcsetattr ~ICANON");
        return (buf);
    }

public:
    // Returns true if completed, false if stopped
    bool start(int minutes)
    {
        if (minutes <= 0)
            return false;

        // Clear input buffer to avoid immediate trigger from previous cin
        tcflush(STDIN_FILENO, TCIFLUSH);

        long long totalDeciSeconds = minutes * 60 * 10;
        long long remaining = totalDeciSeconds;
        bool paused = false;

        cout << "\n--- Focus Timer (" << minutes << " min) ---\n";
        cout << "Controls: [P]ause/Resume  [S]top\n\n";

        while (remaining >= 0)
        {
            if (kbhit())
            {
                char c = getch();
                if (c == 'p' || c == 'P')
                {
                    paused = !paused;
                    if (paused)
                        cout << "\n[PAUSED] Press P to resume...   ";
                    else
                        cout << "\n[RESUMED]                       \n";
                }
                else if (c == 's' || c == 'S')
                {
                    cout << "\n[STOPPED] Session terminated.\n";
                    return false;
                }
            }

            if (!paused)
            {
                int min = (remaining / 10) / 60;
                int sec = (remaining / 10) % 60;
                int deci = remaining % 10;

                cout << "\rTime Remaining: "
                     << setfill('0') << setw(2) << min << ":"
                     << setfill('0') << setw(2) << sec << "."
                     << deci << "   " << flush;

                if (remaining == 0)
                    break;

                this_thread::sleep_for(chrono::milliseconds(100));
                remaining--;
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }

        cout << "\n\n[COMPLETE] Session finished successfully!\n";
        return true;
    }
};