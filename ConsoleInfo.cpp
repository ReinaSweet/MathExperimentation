#include "stdafx.h"
#include "ConsoleInfo.h"

#include <windows.h>
#include <wincon.h>

size_t ConsoleInfo::GetConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenBuffer);
    return (size_t)screenBuffer.dwSize.X;
}
