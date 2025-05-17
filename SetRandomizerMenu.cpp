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

        printf("\n^ SetSize: %llu, Blocks: %u, Num Errors: %u, Time: %uns", setSize, (uint32_t)Blocks, numErrors, totalTime);
    }
}



SetRandomizerMenu::SetRandomizerMenu(const char* const input, ConsoleMenu& parentMenu)
: mMenu("Set Randomizer", parentMenu)
, mMenuVisualize("Visualizer", mMenu)
, mStartTime(std::chrono::high_resolution_clock::now())
{
    parentMenu.AddSubmenu(input, mMenu);

    mMenu.AddSubmenu("v", mMenuVisualize);
    mMenuVisualize.AddCommand("ba", "Set Size 19 (1 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_1, this, 19)));
    mMenuVisualize.AddCommand("bb", "Set Size 31 (2 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_2, this, 31)));
    mMenuVisualize.AddCommand("bc", "Set Size 43 (3 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_3, this, 43)));
    mMenuVisualize.AddCommand("ca", "Set Size 178 (1 block)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_1, this, 178)));
    mMenuVisualize.AddCommand("cb", "Set Size 178 (2 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_2, this, 178)));
    mMenuVisualize.AddCommand("cc", "Set Size 178 (3 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_3, this, 178)));
    mMenuVisualize.AddCommand("da", "Set Size 1783 (1 block)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_1, this, 1783)));
    mMenuVisualize.AddCommand("db", "Set Size 1783 (2 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_2, this, 1783)));
    mMenuVisualize.AddCommand("dc", "Set Size 1783 (3 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_3, this, 1783)));
    mMenuVisualize.AddCommand("sa", "Set Size 128 (13 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_13, this, 128)));
    mMenuVisualize.AddCommand("za", "Set Size 251 (28 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_28, this, 251)));
    mMenuVisualize.AddCommand("zb", "Set Size 252 (28 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_28, this, 252)));
    mMenuVisualize.AddCommand("zc", "Set Size 253 (28 blocks)", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_VisualizeN_28, this, 253)));
    mMenuVisualize.AddCommand("wa", "Whatever Set Size You Want (1 block);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_1(setSize); });
    mMenuVisualize.AddCommand("wb", "Whatever Set Size You Want (2 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_2(setSize); });
    mMenuVisualize.AddCommand("wc", "Whatever Set Size You Want (3 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_3(setSize); });
    mMenuVisualize.AddCommand("wm", "Whatever Set Size You Want (13 blocks);dSet Size", [this](uint64_t setSize) { Cmd_VisualizeN_13(setSize); });
    mMenuVisualize.AddCommand("tv", "Test Various", std::function<void(void)>(std::bind(&SetRandomizerMenu::Cmd_TestVarious, this)));
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

void SetRandomizerMenu::Reseed()
{
    const std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
    const uint32_t timeSinceStart = (uint32_t)std::chrono::duration_cast<std::chrono::nanoseconds>(time - mStartTime).count();
    sDist.seed(timeSinceStart);
}
