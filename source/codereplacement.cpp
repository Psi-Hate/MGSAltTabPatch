#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>
#include <thread>
#include "codereplacement.h"

void CodeReplacement_Construct(CodeReplacement *thisx)
{
    thisx->OriginalBytes = nullptr;
    thisx->Address = nullptr;
    thisx->Enabled = 0;
}

void CodeReplacement_Destruct(CodeReplacement *thisx)
{
    if (thisx->OriginalBytes != nullptr)
    {
        free(thisx->OriginalBytes);
    }
}

void CodeReplacement_WriteBytes(CodeReplacement *thisx)
{
    if (thisx->Address == nullptr)
    {
        auto pattern = hook::pattern(thisx->OriginalBytesString);
        if (!pattern.empty())
        {
            thisx->Address = pattern.get_first(0);
        }
    }

    if (thisx->Address == nullptr)
    {
        thisx->Enabled = 0;
        return;
    }

    DWORD protection[2];
    if (VirtualProtect(thisx->Address, thisx->Length, PAGE_EXECUTE_READWRITE, &protection[0]))
    {
        size_t offset = 0;

        thisx->OriginalBytes = (uint8_t *)malloc(thisx->Length);
        if (thisx->OriginalBytes != nullptr)
        {
            while (offset < thisx->Length)
            {
                thisx->OriginalBytes[offset] = ((uint8_t *)thisx->Address)[offset];
                ((uint8_t *)thisx->Address)[offset] = thisx->NewBytes[offset];
                offset++;
            }
        }

        thisx->Enabled = 1;
        VirtualProtect(thisx->Address, thisx->Length, protection[0], &protection[1]);
    }
}

void CodeReplacement_ResetBytes(CodeReplacement *thisx)
{
    thisx->Enabled = 0;
    if (thisx->Address == nullptr || thisx->OriginalBytes == nullptr)
    {
        return;
    }

    DWORD protection[2];
    if (VirtualProtect(thisx->Address, thisx->Length, PAGE_EXECUTE_READWRITE, &protection[0]))
    {
        size_t offset = 0;
        while (offset < thisx->Length)
        {
            ((uint8_t *)thisx->Address)[offset] = thisx->OriginalBytes[offset];
            offset++;
        }
        VirtualProtect(thisx->Address, thisx->Length, protection[0], &protection[1]);
    }
}