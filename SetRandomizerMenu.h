#pragma once
#include "ConsoleMenu.h"
#include <chrono>

class SetRandomizerMenu
{
public:
    SetRandomizerMenu(const char* const input, ConsoleMenu& parentMenu);

private:
    void Cmd_VisualizeN_1(uint64_t n);
    void Cmd_VisualizeN_2(uint64_t n);
    void Cmd_VisualizeN_3(uint64_t n);
    void Cmd_VisualizeN_13(uint64_t n);
    void Cmd_VisualizeN_28(uint64_t n);
    void Cmd_TestVarious();

    void Reseed();

    ConsoleMenu mMenu;
    ConsoleMenu mMenuVisualize;

    const std::chrono::high_resolution_clock::time_point mStartTime;
};

