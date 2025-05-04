#pragma once
#include "ConsoleMenu.h"

class SetRandomizerMenu
{
public:
    SetRandomizerMenu(const char* const input, ConsoleMenu& parentMenu);

private:
    void Cmd_VisualizeN_1(uint64_t n);
    void Cmd_VisualizeN_2(uint64_t n);
    void Cmd_VisualizeN_3(uint64_t n);

    ConsoleMenu mMenu;
    ConsoleMenu mMenuVisualize;
};

