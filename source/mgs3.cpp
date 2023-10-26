#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>
#include <thread>
#include <iostream>
#include "mgs3.h"

const char *MGS3_PauseHook_OriginalBytes = "FF 25 BA 55 17 00";
uint8_t MGS3_PauseHook_NewBytes[] = {0x48, 0x31, 0xC0, 0xC3, 0xCC, 0xCC}; // XOR RAX, RAX; RET;
CodeReplacement MGS3_PauseHookCode = {sizeof(MGS3_PauseHook_NewBytes), MGS3_PauseHook_OriginalBytes, MGS3_PauseHook_NewBytes};

const char *MGS3_MinimizeHook_OriginalBytes = "BD 02 00 00 00 0F 45 E8 8B D5";
uint8_t MGS3_MinimizeHook_NewBytes[] = {0xBD, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x45, 0xE8, 0x8B, 0xD5, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MGS3_MinimizeHookCode = {sizeof(MGS3_MinimizeHook_NewBytes), MGS3_MinimizeHook_OriginalBytes, MGS3_MinimizeHook_NewBytes};

const char *MGS3_CursorLockHook_OriginalBytes = "FF 15 61 41 78 00";
uint8_t MGS3_CursorLockHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MGS3_CursorLockHookCode = {sizeof(MGS3_CursorLockHook_NewBytes), MGS3_CursorLockHook_OriginalBytes, MGS3_CursorLockHook_NewBytes};

const char *MGS3_CursorHideHook_OriginalBytes = "FF 15 26 3D 82 00";
uint8_t MGS3_CursorHideHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MGS3_CursorHideHookCode = {sizeof(MGS3_CursorHideHook_NewBytes), MGS3_CursorHideHook_OriginalBytes, MGS3_CursorHideHook_NewBytes};

struct OsContext
{
    uint8_t unk_0x00[0x50];
    HWND window;
};

OsContext *MGS3_ = (OsContext *)nullptr; // 0x7FF740E21050; // 0x141EF8878;
HWND MGS3_GameWindow = nullptr;

std::thread MGS3_UpdateThread;
int MGS3_UpdateThreadShouldClose = 0;

bool MGS3_IsWindowFocused(HWND window)
{
    return GetForegroundWindow() == window;
}

void MGS3_UpdateThreadFn(void)
{
    std::cout << "MGS3 UpdateThreadFN\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // this didn't work, so we are just lazily using FindWindow
    /*
    HMODULE hMainModule = GetModuleHandle(NULL);
    if (hMainModule == NULL) {
        return;
    }

    gpOsContext = (OsContext*)((uint8_t *)hMainModule + 0x871050);
    */

    MGS3_GameWindow = FindWindowA(NULL, "METAL GEAR SOLID 3 SNAKE EATER");

    while (1)
    {
        if (MGS3_UpdateThreadShouldClose)
        {
            break;
        }

        if (MGS3_GameWindow == NULL)
        {
            continue;
        }
        if (MGS3_IsWindowFocused(MGS3_GameWindow))
        {
            //std::cout << "MGS3 Focused\n";
            if (MGS3_CursorLockHookCode.Enabled == 1)
            {
                std::cout << "MGS3 Cursor Locked\n";
                CodeReplacement_ResetBytes(&MGS3_CursorLockHookCode);
                CodeReplacement_ResetBytes(&MGS3_CursorHideHookCode);
            }
        }
        else
        {
            //std::cout << "MGS3 Not Focused\n";
            if (MGS3_CursorLockHookCode.Enabled == 0)
            {
                //std::cout << "MGS3 Tabbed Out\n";
                std::cout << "MGS3 Cursor Unlocked\n";
                CodeReplacement_WriteBytes(&MGS3_CursorLockHookCode);
                CodeReplacement_WriteBytes(&MGS3_CursorHideHookCode);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void MGS3_Init()
{
    std::cout << "MGS3 Init Start\n";
    CodeReplacement_Construct(&MGS3_PauseHookCode);
    CodeReplacement_Construct(&MGS3_MinimizeHookCode);
    CodeReplacement_Construct(&MGS3_CursorLockHookCode);
    CodeReplacement_Construct(&MGS3_CursorHideHookCode);

    // BP_COsContext_ShouldPauseApplication
    CodeReplacement_WriteBytes(&MGS3_PauseHookCode);

    // ShowWindow
    CodeReplacement_WriteBytes(&MGS3_MinimizeHookCode);

    std::cout << "MGS3 Init Finish\n";
    MGS3_UpdateThread = std::thread(MGS3_UpdateThreadFn);
}

void MGS3_Destroy()
{
    std::cout << "MGS3 Destroy Start\n";
    MGS3_UpdateThreadShouldClose = 1;
    if (MGS3_UpdateThread.joinable())
    {
        MGS3_UpdateThread.join();
    }

    CodeReplacement_Destruct(&MGS3_PauseHookCode);
    CodeReplacement_Destruct(&MGS3_MinimizeHookCode);
    CodeReplacement_Destruct(&MGS3_CursorLockHookCode);
    CodeReplacement_Destruct(&MGS3_CursorHideHookCode);
    std::cout << "MGS3 Finish\n";
}
