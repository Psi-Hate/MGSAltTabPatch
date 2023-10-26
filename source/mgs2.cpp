#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>
#include <thread>
#include <iostream>
#include "mgs2.h"

const char *MGS2_PauseHook_OriginalBytes = "FF 25 40 4D 0A 00";
uint8_t MGS2_PauseHook_NewBytes[] = {0x48, 0x31, 0xC0, 0xC3, 0xCC, 0xCC}; // XOR RAX, RAX; RET;
CodeReplacement MGS2_PauseHookCode = {sizeof(MGS2_PauseHook_NewBytes), MGS2_PauseHook_OriginalBytes, MGS2_PauseHook_NewBytes};

const char *MGS2_MinimizeHook_OriginalBytes = "BD 02 00 00 00 0F 45 E8 8B D5";
uint8_t MGS2_MinimizeHook_NewBytes[] = {0xBD, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x45, 0xE8, 0x8B, 0xD5, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MGS2_MinimizeHookCode = {sizeof(MGS2_MinimizeHook_NewBytes), MGS2_MinimizeHook_OriginalBytes, MGS2_MinimizeHook_NewBytes};

// const char *CursorLockHook_OriginalBytes = "FF 15 61 41 78 00";
// uint8_t CursorLockHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
// CodeReplacement CursorLockHookCode = {sizeof(CursorLockHook_NewBytes), CursorLockHook_OriginalBytes, CursorLockHook_NewBytes};

const char *MGS2_CursorHideHook_OriginalBytes = "ff 15 BE 6F 6D 00";
uint8_t MGS2_CursorHideHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MGS2_CursorHideHookCode = {sizeof(MGS2_CursorHideHook_NewBytes), MGS2_CursorHideHook_OriginalBytes, MGS2_CursorHideHook_NewBytes};

struct OsContext
{
    uint8_t unk_0x00[0x50];
    HWND window;
};

OsContext *MGS2_gpOsContext = (OsContext *)nullptr; // 0x7FF740E21050; // 0x141EF8878;
HWND MGS2_GameWindow = nullptr;

std::thread MGS2_UpdateThread;
int MGS2_UpdateThreadShouldClose = 0;

bool MGS2_IsWindowFocused(HWND window)
{
    return GetForegroundWindow() == window;
}
void MGS2_UpdateThreadFn(void)
{
    std::cout << "MGS2 UpdateThreadFN\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // this didn't work, so we are just lazily using FindWindow
    /*
    HMODULE hMainModule = GetModuleHandle(NULL);
    if (hMainModule == NULL) {
        return;
    }

    gpOsContext = (OsContext*)((uint8_t *)hMainModule + 0x871050);
    */

    MGS2_GameWindow = FindWindowA(NULL, "METAL GEAR SOLID 2 SONS OF LIBERTY");

    while (1)
    {
        if (MGS2_UpdateThreadShouldClose)
        {
            break;
        }

        if (MGS2_GameWindow == NULL)
        {
            continue;
        }
        if (MGS2_IsWindowFocused(MGS2_GameWindow))
        {
            //std::cout << "MGS2 Focused\n";
            if (MGS2_CursorHideHookCode.Enabled == 1)
            {
                std::cout << "MGS2 Cursor Locked\n";
                CodeReplacement_ResetBytes(&MGS2_CursorHideHookCode);
            }
        }
        else
        {
            //std::cout << "MGS2 Not Focused\n";
            if (MGS2_CursorHideHookCode.Enabled == 0)
            {
                //std::cout << "MGS2 Tabbed Out\n";
                std::cout << "MGS2 Cursor Unlocked\n";
                CodeReplacement_WriteBytes(&MGS2_CursorHideHookCode);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void MGS2_Init()
{
    std::cout << "MGS2 Init Start\n";
    CodeReplacement_Construct(&MGS2_PauseHookCode);
    CodeReplacement_Construct(&MGS2_MinimizeHookCode);
    CodeReplacement_Construct(&MGS2_CursorHideHookCode);

    // BP_COsContext_ShouldPauseApplication
    CodeReplacement_WriteBytes(&MGS2_PauseHookCode);

    // ShowWindow
    CodeReplacement_WriteBytes(&MGS2_MinimizeHookCode);

    std::cout << "MGS2 Init Finish\n";
    MGS2_UpdateThread = std::thread(MGS2_UpdateThreadFn);
}

void MGS2_Destroy()
{
    std::cout << "MGS2 Destroy Start\n";
    MGS2_UpdateThreadShouldClose = 1;
    if (MGS2_UpdateThread.joinable())
    {
        MGS2_UpdateThread.join();
    }

    CodeReplacement_Destruct(&MGS2_PauseHookCode);
    CodeReplacement_Destruct(&MGS2_MinimizeHookCode);
    CodeReplacement_Destruct(&MGS2_CursorHideHookCode);
    std::cout << "MGS2 Finish\n";
}