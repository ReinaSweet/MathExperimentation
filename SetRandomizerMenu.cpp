#include "SetRandomizerMenu.h"

#include "SetRandomizer.h"
#include "MathPrint.h"

#include <random>

std::mt19937 sDist;

namespace
{
    uint32_t RandomNumber()
    {
        return sDist();
    }

    template<size_t Blocks>
    void PrintRandomizer(SetRandomizer<Blocks>& randomizer)
    {
        const uint64_t setSize = randomizer.GetSetSize();
        uint32_t padding = 1;
        for (uint64_t i = setSize; i > 0; i /= 10)
        {
            ++padding;
        }

        putchar('/');
        for (uint32_t i = 0; i < (32 * padding + 1); ++i)
        {
            putchar('-');
        }
        printf("\\\n");

        std::set<uint32_t> existing;
        std::set<uint32_t> errors;
        std::vector<uint32_t> results;
        uint32_t index = 0;
        uint32_t numErrors = 0;
        uint32_t totalTime = 0;

        {
            for (index = 0; index < setSize; ++index)
            {
                const std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
                const uint32_t val = randomizer.GetWheeledIndex(static_cast<uint32_t>(index));
                const std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
                totalTime += (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

                results.push_back(val);

                if (existing.find(val) == existing.end())
                {
                    existing.insert(val);
                }
                else if (errors.find(val) == errors.end())
                {
                    errors.insert(val);
                }
            }
        }

        for (index = 0; index < setSize; ++index)
        {
            if ((index % 32) == 0)
            {
                putchar('|');
            }
            const uint32_t val = results.at(index);

            if (errors.find(val) == errors.end())
            {
                printf("%*u", padding, val);
            }
            else
            {
                MathPrint::PrintFGColorCode(MathPrint::PrintColor::DarkRed);
                printf("%*u", padding, val);
                MathPrint::PrintResetCC();
                ++numErrors;
            }
            if (((index % 32) == 31) && (index + 1 < setSize))
            {
                printf(" |\n|");
                for (uint32_t i = 0; i < (32 * padding + 1); ++i)
                {
                    putchar(' ');
                }
                printf("|\n");
            }
        }

        for (uint32_t i = 0; i < ((32 - index) % 32 * padding + 1); ++i)
        {
            putchar(' ');
        }

        printf("|\n\\");
        for (uint32_t i = 0; i < (32 * padding + 1); ++i)
        {
            putchar('-');
        }
        putchar('/');

        const uint32_t timePerEntry = setSize > 0 ? totalTime / (uint32_t)setSize : 0;
        printf("\n^ SetSize: %llu, Blocks: %u, Num Errors: %u, Time: %uns, Time per Entry: %uns", 
            setSize, (uint32_t)Blocks, numErrors, totalTime, timePerEntry);
    }
}



SetRandomizerMenu::SetRandomizerMenu(const char* const input, ConsoleMenu& parentMenu)
: mMenu("Set Randomizer", parentMenu)
, mMenuVisualize("Visualizer", mMenu)
, mStartTime(std::chrono::high_resolution_clock::now())
{
    parentMenu.AddSubmenu(input, mMenu);

    mMenu.AddSubmenu("v", mMenuVisualize);
    mMenuVisualize.AddCommand("ba", "Set Size 19 (1 blocks)", [this]() { Cmd_VisualizeN_1(19); });
    mMenuVisualize.AddCommand("bb", "Set Size 31 (2 blocks)", [this]() { Cmd_VisualizeN_2(31); });
    mMenuVisualize.AddCommand("bc", "Set Size 43 (3 blocks)", [this]() { Cmd_VisualizeN_3(43); });
    mMenuVisualize.AddCommand("ca", "Set Size 178 (1 block)", [this]() { Cmd_VisualizeN_1(178); });
    mMenuVisualize.AddCommand("cb", "Set Size 178 (2 blocks)", [this]() { Cmd_VisualizeN_2(178); });
    mMenuVisualize.AddCommand("cc", "Set Size 178 (3 blocks)", [this]() { Cmd_VisualizeN_3(178); });
    mMenuVisualize.AddCommand("da", "Set Size 1783 (1 block)", [this]() { Cmd_VisualizeN_1(1783); });
    mMenuVisualize.AddCommand("db", "Set Size 1783 (2 blocks)", [this]() { Cmd_VisualizeN_2(1783); });
    mMenuVisualize.AddCommand("dc", "Set Size 1783 (3 blocks)", [this]() { Cmd_VisualizeN_3(1783); });
    mMenuVisualize.AddCommand("sa", "Set Size 128 (13 blocks)", [this]() { Cmd_VisualizeN_13(128); });
    mMenuVisualize.AddCommand("za", "Set Size 251 (28 blocks)", [this]() { Cmd_VisualizeN_28(251); });
    mMenuVisualize.AddCommand("zb", "Set Size 252 (28 blocks)", [this]() { Cmd_VisualizeN_28(252); });
    mMenuVisualize.AddCommand("zc", "Set Size 253 (28 blocks)", [this]() { Cmd_VisualizeN_28(253); });
    mMenuVisualize.AddCommand("wa", "Whatever Set Size You Want (1 block);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_1(setSize); });
    mMenuVisualize.AddCommand("wb", "Whatever Set Size You Want (2 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_2(setSize); });
    mMenuVisualize.AddCommand("wc", "Whatever Set Size You Want (3 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_3(setSize); });
    mMenuVisualize.AddCommand("wm", "Whatever Set Size You Want (13 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_13(setSize); });
    mMenuVisualize.AddCommand("tl", "Test Large", [this]() { Cmd_TestLarge(); });
    mMenuVisualize.AddCommand("tv", "Test Various", [this]() { Cmd_TestVarious(); });

    mMenu.AddCommand("mdt", "Make Docs: Transformer;dSet Size", [this](uint64_t setSize) { Cmd_MakeDocs_Transformer(setSize); });
    mMenu.AddCommand("mds", "Make Docs: Standard Transform in cpp", [this]() { Cmd_MakeDocs_StandardTransform(); });
}

void SetRandomizerMenu::Cmd_VisualizeN_1(uint64_t n)
{
    Reseed();
    SetRandomizer<1> randomizer(&RandomNumber, (uint32_t)n);
    PrintRandomizer(randomizer);
}

void SetRandomizerMenu::Cmd_VisualizeN_2(uint64_t n)
{
    Reseed();
    SetRandomizer<2> randomizer(&RandomNumber, (uint32_t)n);
    PrintRandomizer(randomizer);
}

void SetRandomizerMenu::Cmd_VisualizeN_3(uint64_t n)
{
    Reseed();
    SetRandomizer<3> randomizer(&RandomNumber, (uint32_t)n);
    PrintRandomizer(randomizer);
}

void SetRandomizerMenu::Cmd_VisualizeN_13(uint64_t n)
{
    Reseed();
    SetRandomizer<13> randomizer(&RandomNumber, (uint32_t)n);
    PrintRandomizer(randomizer);
}

void SetRandomizerMenu::Cmd_VisualizeN_28(uint64_t n)
{
    Reseed();
    SetRandomizer<28> randomizer(&RandomNumber, (uint32_t)n);
    PrintRandomizer(randomizer);
}

void SetRandomizerMenu::Cmd_TestVarious()
{
    Cmd_VisualizeN_1(0);  printf("\n\n");
    Cmd_VisualizeN_1(1);  printf("\n\n");
    Cmd_VisualizeN_1(2);  printf("\n\n");
    Cmd_VisualizeN_1(3);  printf("\n\n");
    Cmd_VisualizeN_1(19); printf("\n\n");
    Cmd_VisualizeN_1(20); printf("\n\n");
    Cmd_VisualizeN_1(21); printf("\n\n");
    Cmd_VisualizeN_1(31); printf("\n\n");
    Cmd_VisualizeN_1(32); printf("\n\n");
    Cmd_VisualizeN_1(33); printf("\n\n");
    Cmd_VisualizeN_2(31); printf("\n\n");
    Cmd_VisualizeN_2(32); printf("\n\n");
    Cmd_VisualizeN_2(33); printf("\n\n");
    Cmd_VisualizeN_13(128); printf("\n\n");
    Cmd_VisualizeN_28(253); printf("\n\n");
}

void SetRandomizerMenu::Cmd_TestLarge()
{
    Cmd_VisualizeN_13(4680);  printf("\n\n");
}

struct SetRandomizerTNode
{
    SetRandomizerTNode(SetRandomizerMenu& parent, uint64_t inNumericalValue)
        : mParent(parent)
        , NumericalValue(inNumericalValue)
    {}

    void Div(uint64_t divisor)
    {
        if (NumericalValue >= mParent.mTransformerLimit)
        {
            return;
        }
        if (Quotient)
        {
            Quotient->Div(divisor);
        }
        else
        {
            Quotient = new SetRandomizerTNode(mParent, NumericalValue / divisor);
            Remainder = new SetRandomizerTNode(mParent, NumericalValue % divisor);
            NumericalValue = divisor;
        }
    }

    uint64_t Combine()
    {
        if (Quotient)
        {
            if (Quotient->Quotient)
            {
                return Quotient->Combine();
            }
            else
            {
                const uint64_t oldDivisor = NumericalValue;
                NumericalValue = (Quotient->NumericalValue * oldDivisor) + Remainder->NumericalValue;
                delete Quotient;
                Quotient = nullptr;
                delete Remainder;
                Remainder = nullptr;
                return oldDivisor;
            }
        }
        return 0;
    }

    void MapQuotient(uint64_t indexA, uint64_t indexB)
    {
        if (Quotient)
        {
            if (Quotient->Quotient)
            {
                Quotient->MapQuotient(indexA, indexB);
            }
            else
            {
                if (Quotient->NumericalValue == indexA)
                {
                    Quotient->NumericalValue = indexB;
                }
                else if (Quotient->NumericalValue == indexB)
                {
                    Quotient->NumericalValue = indexA;
                }
            }
        }
    }

    void MapRemainder(uint64_t indexA, uint64_t indexB)
    {
        if (Quotient)
        {
            if (Quotient->Quotient)
            {
                Quotient->MapRemainder(indexA, indexB);
            }
            else
            {
                if (Remainder->NumericalValue == indexA)
                {
                    Remainder->NumericalValue = indexB;
                }
                else if (Remainder->NumericalValue == indexB)
                {
                    Remainder->NumericalValue = indexA;
                }
            }
        }
    }

    void Shift(uint64_t amount)
    {
        const uint64_t nodes = mParent.mTransformerNodes.size();
        NumericalValue = (NumericalValue + (nodes - amount)) % nodes;
    }

    void Print()
    {
        if (Quotient)
        {
            putchar('[');
            Quotient->Print();
            putchar('_');
            Remainder->Print();
            putchar(']');
        }
        else
        {
            printf("%llu", NumericalValue);
        }
    }

private:
    SetRandomizerMenu& mParent;
    uint64_t NumericalValue = 0;
    SetRandomizerTNode* Quotient = nullptr;
    SetRandomizerTNode* Remainder = nullptr;
};

void SetRandomizerMenu::Cmd_MakeDocs_Transformer(uint64_t setSize)
{
    MakeDocs_Transformer_Reset(setSize);

    ConsoleMenu transformerMenu("Make Docs: Transformer");
    transformerMenu.AddCommand("r", "Reset", [this]() { MakeDocs_Transformer_Reset(mTransformerNodes.size()); });
    transformerMenu.AddCommand("l", "Limit Transforms;dMin value to no longer transform", [this](uint64_t limit) { MakeDocs_Transformer_Limit(limit); });
    transformerMenu.AddCommand("d", "Div;dDivisor", [this](uint64_t divisor) { MakeDocs_Transformer_Div(divisor); });
    transformerMenu.AddCommand("c", "Combine", [this]() { MakeDocs_Transformer_Combine(); });
    transformerMenu.AddCommand("mq", "Map Quotient;dIndex A;dIndex B", [this](uint64_t indexA, uint64_t indexB) { MakeDocs_Transformer_MapQuotient(indexA, indexB); });
    transformerMenu.AddCommand("mr", "Map Remainder;dIndex A;dIndex B", [this](uint64_t indexA, uint64_t indexB) { MakeDocs_Transformer_MapRemainder(indexA, indexB); });
    transformerMenu.AddCommand("s", "Shift;dAmount", [this](uint64_t amount) { MakeDocs_Transformer_Shift(amount); });

    transformerMenu.RunMenu();
    MakeDocs_Transformer_Clear();
}

void SetRandomizerMenu::Cmd_MakeDocs_StandardTransform()
{
    const uint64_t setSize = 16;
    MakeDocs_Transformer_Reset(setSize);

    MakeDocs_Transformer_Limit(15);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Limit(12);
    MakeDocs_Transformer_Div(2);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Limit(12);
    MakeDocs_Transformer_Div(4);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Shift(4);
    putchar('\n');

    MakeDocs_Transformer_Limit(15);
    MakeDocs_Transformer_Div(5);
    MakeDocs_Transformer_MapQuotient(0, 1);
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Shift(4);
    putchar('\n');

    MakeDocs_Transformer_Limit(15);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Limit(12);
    MakeDocs_Transformer_Div(2);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    MakeDocs_Transformer_Combine();
    putchar('\n');

    MakeDocs_Transformer_Limit(12);
    MakeDocs_Transformer_Div(4);
    MakeDocs_Transformer_Div(3);
    MakeDocs_Transformer_MapRemainder(0, 1);
    MakeDocs_Transformer_Combine();
    MakeDocs_Transformer_Combine();
    putchar('\n');

    PrintCurrentTransformer();
    printf(" from ");
    MakeDocs_Transformer_Reset(setSize);
    MakeDocs_Transformer_Clear();
}

void SetRandomizerMenu::MakeDocs_Transformer_Clear()
{
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        delete node;
    }
    mTransformerNodes.clear();
}

void SetRandomizerMenu::MakeDocs_Transformer_Reset(uint64_t setSize)
{
    MakeDocs_Transformer_Clear();

    for (uint64_t i = 0; i < setSize; ++i)
    {
        mTransformerNodes.push_back(new SetRandomizerTNode(*this, i));
    }

    PrintCurrentTransformer();
    putchar('\n');
}

void SetRandomizerMenu::MakeDocs_Transformer_Limit(uint64_t limit)
{
    mTransformerLimit = limit;
}

void SetRandomizerMenu::MakeDocs_Transformer_Combine()
{
    uint64_t oldDivisor = 0;
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        oldDivisor = std::max(node->Combine(), oldDivisor);
    }
    PrintCurrentTransformer();
    printf(" :: combine %llu\n", oldDivisor);
}

void SetRandomizerMenu::MakeDocs_Transformer_Div(uint64_t divisor)
{
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        node->Div(divisor);
    }
    PrintCurrentTransformer();
    printf(" :: div %llu\n", divisor);
}

void SetRandomizerMenu::MakeDocs_Transformer_MapQuotient(uint64_t indexA, uint64_t indexB)
{
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        node->MapQuotient(indexA, indexB);
    }
    PrintCurrentTransformer();
    printf(" :: qmap(%llu,%llu)\n", indexA, indexB);
}

void SetRandomizerMenu::MakeDocs_Transformer_MapRemainder(uint64_t indexA, uint64_t indexB)
{
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        node->MapRemainder(indexA, indexB);
    }
    PrintCurrentTransformer();
    printf(" :: rmap(%llu,%llu)\n", indexA, indexB);
}

void SetRandomizerMenu::MakeDocs_Transformer_Shift(uint64_t amount)
{
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        node->Shift(amount);
    }
    PrintCurrentTransformer();
    printf(" :: shift %llu\n", amount);
}

void SetRandomizerMenu::PrintCurrentTransformer()
{
    bool firstNode = true;
    for (SetRandomizerTNode* node : mTransformerNodes)
    {
        if (firstNode)
        {
            firstNode = false;
        }
        else
        {
            putchar(',');
        }
        node->Print();
    }
}

void SetRandomizerMenu::Reseed()
{
    const std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
    const uint32_t timeSinceStart = (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(time - mStartTime).count();
    sDist.seed(timeSinceStart);
}
