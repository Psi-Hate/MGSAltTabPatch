#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>
#include <thread>
#include <random>
#include <cstdio>

struct CodeReplacement {
    size_t Length;
    const char *OriginalBytesString;
    uint8_t *NewBytes;
    uint8_t *OriginalBytes;
    LPVOID Address;
    int Enabled;
};

const char *PauseHook_OriginalBytes = "FF 25 BA 55 17 00";
uint8_t PauseHook_NewBytes[] = {0x48, 0x31, 0xC0, 0xC3, 0xCC, 0xCC}; // XOR RAX, RAX; RET;
CodeReplacement PauseHookCode = {sizeof(PauseHook_NewBytes), PauseHook_OriginalBytes, PauseHook_NewBytes};

const char *MinimizeHook_OriginalBytes = "BD 02 00 00 00 0F 45 E8 8B D5";
uint8_t MinimizeHook_NewBytes[] = {0xBD, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x45, 0xE8, 0x8B, 0xD5, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement MinimizeHookCode = {sizeof(MinimizeHook_NewBytes), MinimizeHook_OriginalBytes, MinimizeHook_NewBytes};

const char *CursorLockHook_OriginalBytes = "FF 15 61 41 78 00";
uint8_t CursorLockHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement CursorLockHookCode = {sizeof(CursorLockHook_NewBytes), CursorLockHook_OriginalBytes, CursorLockHook_NewBytes};

const char *CursorHideHook_OriginalBytes = "FF 15 26 3D 82 00";
uint8_t CursorHideHook_NewBytes[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
CodeReplacement CursorHideHookCode = {sizeof(CursorHideHook_NewBytes), CursorHideHook_OriginalBytes, CursorHideHook_NewBytes};

void CodeReplacement_Construct(CodeReplacement* thisx) {
    thisx->OriginalBytes = nullptr;
    thisx->Address = nullptr;
    thisx->Enabled = 0;
}

void CodeReplacement_Destruct(CodeReplacement* thisx) {
    if (thisx->OriginalBytes != nullptr) {
        free(thisx->OriginalBytes);
    }
}

void CodeReplacement_WriteBytes(CodeReplacement* thisx) {
    if (thisx->Address == nullptr) {
        auto pattern = hook::pattern(thisx->OriginalBytesString);
        if (!pattern.empty()) {
            thisx->Address = pattern.get_first(0);
        }
    }

    if (thisx->Address == nullptr) {
        thisx->Enabled = 0;
        return;
    }

    DWORD protection[2];
    if (VirtualProtect(thisx->Address, thisx->Length, PAGE_EXECUTE_READWRITE, &protection[0])) {
        size_t offset = 0;

        thisx->OriginalBytes = (uint8_t *)malloc(thisx->Length);
		if (thisx->OriginalBytes != nullptr) {
			while (offset < thisx->Length) {
			    thisx->OriginalBytes[offset] = ((uint8_t *)thisx->Address)[offset];
			    ((uint8_t *)thisx->Address)[offset] = thisx->NewBytes[offset];
			    offset++;
			}
		}

		thisx->Enabled = 1;
		VirtualProtect(thisx->Address, thisx->Length, protection[0], &protection[1]);
    }
}

void CodeReplacement_ResetBytes(CodeReplacement* thisx) {
    thisx->Enabled = 0;
	if (thisx->Address == nullptr || thisx->OriginalBytes == nullptr) {
		return;
	}

	DWORD protection[2];
    if (VirtualProtect(thisx->Address, thisx->Length, PAGE_EXECUTE_READWRITE, &protection[0])) {
        size_t offset = 0;
        while (offset < thisx->Length) {
            ((uint8_t *)thisx->Address)[offset] = thisx->OriginalBytes[offset];
            offset++;
        }
        VirtualProtect(thisx->Address, thisx->Length, protection[0], &protection[1]);
    }
}

struct OsContext {
	uint8_t unk_0x00[0x50];
	HWND window;
};

OsContext *gpOsContext = (OsContext *)nullptr; //0x7FF740E21050; // 0x141EF8878;
HWND GameWindow = nullptr;

std::thread UpdateThread;
int UpdateThreadShouldClose = 0;

bool IsWindowFocused(HWND window) {
    return GetForegroundWindow() == window;
}

void UpdateThreadFn(void) {
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// this didn't work, so we are just lazily using FindWindow
	/*
	HMODULE hMainModule = GetModuleHandle(NULL);
	if (hMainModule == NULL) {
		return;
	}

	gpOsContext = (OsContext*)((uint8_t *)hMainModule + 0x871050);
	*/

	GameWindow = FindWindowA(NULL, "METAL GEAR SOLID 3 SNAKE EATER");

	while (1) {
		if (UpdateThreadShouldClose) {
			break;
		}

		if (GameWindow == NULL) {
			continue;
		}

		if (IsWindowFocused(GameWindow))
		{
			if (CursorLockHookCode.Enabled == 1) {
				CodeReplacement_ResetBytes(&CursorLockHookCode);
				CodeReplacement_ResetBytes(&CursorHideHookCode);
			}
		}
		else {
			if (CursorLockHookCode.Enabled == 0) {
				CodeReplacement_WriteBytes(&CursorLockHookCode);
				CodeReplacement_WriteBytes(&CursorHideHookCode);
			}
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH)
    {
		CodeReplacement_Construct(&PauseHookCode);
        CodeReplacement_Construct(&MinimizeHookCode);
        CodeReplacement_Construct(&CursorLockHookCode);
        CodeReplacement_Construct(&CursorHideHookCode);

		// BP_COsContext_ShouldPauseApplication
        CodeReplacement_WriteBytes(&PauseHookCode);

        // ShowWindow
        CodeReplacement_WriteBytes(&MinimizeHookCode);

        // SetCursorPos
        //CodeReplacement_WriteBytes(&CursorLockHookCode);

        // SetCursor
        //CodeReplacement_WriteBytes(&CursorHideHookCode);

		UpdateThread = std::thread(UpdateThreadFn);
	}

    if (reason == DLL_PROCESS_DETACH) {
		UpdateThreadShouldClose = 1;
		if (UpdateThread.joinable()) {
            UpdateThread.join();
        }

        CodeReplacement_Destruct(&PauseHookCode);
        CodeReplacement_Destruct(&MinimizeHookCode);
        CodeReplacement_Destruct(&CursorLockHookCode);
        CodeReplacement_Destruct(&CursorHideHookCode);
	}
    return TRUE;
}
