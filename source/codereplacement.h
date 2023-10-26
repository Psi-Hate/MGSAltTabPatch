#include <windows.h>
#include <Hooking.Patterns\Hooking.Patterns.h>

#ifndef CODEREPLACEMENT_H
#define CODEREPLACEMENT_H

struct CodeReplacement
{
    size_t Length;
    const char *OriginalBytesString;
    uint8_t *NewBytes;
    uint8_t *OriginalBytes;
    LPVOID Address;
    int Enabled;
};

void CodeReplacement_Construct(CodeReplacement*);
void CodeReplacement_Destruct(CodeReplacement*);
void CodeReplacement_WriteBytes(CodeReplacement*);
void CodeReplacement_ResetBytes(CodeReplacement*);

#endif /* CODEREPLACEMENT_H */
