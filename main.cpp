#include <raylib.h>
#include <raymath.h>
#include <bits/stdc++.h>


int main()
{
    InitWindow(1000, 800, "Acadence");
    SetTargetFPS(60);

    InitAudioDevice();

    srand(time(NULL));

    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}