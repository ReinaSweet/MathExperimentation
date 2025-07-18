#pragma once
#include "ConsoleMenu.h"
#include <chrono>

struct SetRandomizerTNode;

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
    void Cmd_TestLarge();

    void Cmd_MakeDocs_Transformer(uint64_t setSize);
    void Cmd_MakeDocs_StandardTransform();
    void MakeDocs_Transformer_Clear();
    void MakeDocs_Transformer_Reset(uint64_t setSize);
    void MakeDocs_Transformer_Limit(uint64_t limit);
    void MakeDocs_Transformer_Combine();
    void MakeDocs_Transformer_Div(uint64_t divisor);
    void MakeDocs_Transformer_MapQuotient(uint64_t indexA, uint64_t indexB);
    void MakeDocs_Transformer_MapRemainder(uint64_t indexA, uint64_t indexB);
    void MakeDocs_Transformer_Shift(uint64_t amount);
    void PrintCurrentTransformer();

    void Reseed();

    ConsoleMenu mMenu;
    ConsoleMenu mMenuVisualize;

    std::vector<SetRandomizerTNode*> mTransformerNodes;
    uint64_t mTransformerLimit = 0;

    const std::chrono::high_resolution_clock::time_point mStartTime;

    friend struct SetRandomizerTNode;
};

