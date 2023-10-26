#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>
#include <thread>
#include <tchar.h>
#include <psapi.h>
#include <iostream>
#include "mgs2.h"
#include "mgs3.h"

std::thread InitThread;
int InitThreadShouldClose = 0;

bool isMGS3 = false;
bool isMGS2 = false;

void InitThreadFn(void)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    while (1)
    {
        if (InitThreadShouldClose)
        {
            return;
        }
        if (FindWindowA(NULL, "METAL GEAR SOLID 3 SNAKE EATER") != 0x0000000000000000 || FindWindowA(NULL, "METAL GEAR SOLID 3 SNAKE EATER") != NULL)
        {
            std::cout << "MGS3 Init\n";
            isMGS3 = true;
            MGS3_Init();
            InitThreadShouldClose = 1;
        }
        else if (FindWindowA(NULL, "METAL GEAR SOLID 2 SONS OF LIBERTY") != 0x0000000000000000 || FindWindowA(NULL, "METAL GEAR SOLID 2 SONS OF LIBERTY") != NULL)
        {
            std::cout << "MGS2 Init\n";
            isMGS2 = true;
            MGS2_Init();
            InitThreadShouldClose = 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    if (reason == DLL_PROCESS_ATTACH)
    {
        InitThread = std::thread(InitThreadFn);
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        if (isMGS2)
        {
            std::cout << "MGS2 Destroy\n";
            MGS2_Destroy();
        }
        if (isMGS3)
        {
            std::cout << "MGS3 Destroy\n";
            MGS3_Destroy();
        }
    }

    return TRUE;
}
