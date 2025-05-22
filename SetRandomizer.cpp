#include "SetRandomizer.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <numeric>
#include <emmintrin.h>

/**
* TODO:
* Unify scalar types
*   Primary bits should be uint64_t
* 
* Shuffles
*   Precompute number of shuffles, and determine # more carefully
*   Allow callers to limit number of shuffles
*   Allow callers to limit zooming in shuffles?
*   Math: Are any permutation combinations equivalent to another? Proof?
*   Are there any extended permutation + shuffle techniques?
*
* RNG
*   Document bias in the RNG from modulos that aren't a power of 2
*   Allow callers to feed obfuscating into the RNG
*   Remove RNG function. Replace with fed in RNG only
*   This should also remove the thread-unsafe std::vector
* 
* Refactors
*   kPermutationIndexesPerBlock replaced with Block's MaxFactorial
*   Double check thread safety
*/


namespace Factoradics
{

    constexpr size_t cNumBlock = 28;



    struct Block
    {
        int64_t MinFactorial = 1;
        int64_t MaxFactorial;
        int64_t MinSetPosition = 0;
        bool CoinflipFinalTwo = false;

        constexpr Block(uint64_t entries)
            : MaxFactorial(entries)
        {}
    };

    constexpr std::array<Block, cNumBlock> cBlocks = []() constexpr
    {
        std::array<Block, cNumBlock> blocks =
        {
            20, 13, 11, 11,
            10, 10, 9, 9,
            9, 9, 9, 9,
            8, 8, 8, 8,
            8, 8, 8, 8,
            8, 8, 8, 8,
            8, 7, 7, 7
        };

        uint64_t baseFactorial = 0;
        for (Block& block : blocks)
        {
            const uint64_t nextBase = baseFactorial + block.MaxFactorial;
            block.MinFactorial += baseFactorial;
            block.MaxFactorial += baseFactorial;
            block.MinSetPosition += baseFactorial;
            baseFactorial = nextBase;
        }
        blocks[0].CoinflipFinalTwo = true;

        return blocks;
    }();

    template<int64_t MaxFactorial>
    consteval int64_t ConstMax(int64_t value)
    {
        return (value > MaxFactorial) ? MaxFactorial : value;
    }

    consteval int64_t ConstFactorialRange(int64_t min, int64_t max)
    {
        int64_t value = min == 0 ? 1 : min;
        for (; max > min; --max)
        {
            value *= max;
        }
        return value;
    }

    template<int64_t MinFactorial, int64_t MaxFactorial>
    constexpr int64_t FactorialRange(int64_t m)
    {
        switch (m)
        {
        // Uncomment this to see the constexpr fail for running out of bit space
        // case (MinFactorial + 21): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial + 1>(MinFactorial + 21));
        case (MinFactorial + 20): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 20));
        case (MinFactorial + 19): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 19));
        case (MinFactorial + 18): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 18));
        case (MinFactorial + 17): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 17));
        case (MinFactorial + 16): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 16));
        case (MinFactorial + 15): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 15));
        case (MinFactorial + 14): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 14));
        case (MinFactorial + 13): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 13));
        case (MinFactorial + 12): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 12));
        case (MinFactorial + 11): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 11));
        case (MinFactorial + 10): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 10));
        case (MinFactorial + 9): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 9));
        case (MinFactorial + 8): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 8));
        case (MinFactorial + 7): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 7));
        case (MinFactorial + 6): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 6));
        case (MinFactorial + 5): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 5));
        case (MinFactorial + 4): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 4));
        case (MinFactorial + 3): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 3));
        case (MinFactorial + 2): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 2));
        case (MinFactorial + 1): return ConstFactorialRange(MinFactorial, ConstMax<MaxFactorial>(MinFactorial + 1));
        case (MinFactorial): return ConstFactorialRange(MinFactorial, MinFactorial);
        default: return 1;
        }
    }
};

/**
 * Build an indexed permutation sequence for our current random seed
 * See: https://oeis.org/A030299 (but off by 1)
 */
template<size_t tMaxBlockIndex, Factoradics::Block tMaxBlock = Factoradics::cBlocks[tMaxBlockIndex]>
class PermutationBuilder
{
public:
    PermutationBuilder(int32_t setSize)
        : mNextEntry(setSize > tMaxBlock.MaxFactorial ? tMaxBlock.MaxFactorial : setSize)
    {
        std::iota(mPositionSet.begin(), mPositionSet.end(), 0);
    }

    template<size_t tBlockIndex, Factoradics::Block tBlock = Factoradics::cBlocks[tBlockIndex]>
    void FillBlock(SetRandomizerInternal::PermutationBlock& permutationIndexes, int64_t randomBits)
    {
        mValueAndRemainder = { 0, randomBits % Factoradics::FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry)};

        --mNextEntry;
        mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, Factoradics::FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry));
        permutationIndexes[mNextEntry - tBlock.MinSetPosition] = GetAndRemovePosition_Careful(mValueAndRemainder.quot);

        constexpr int64_t cLoopMin = (tBlock.MinFactorial + ((int64_t)tBlock.CoinflipFinalTwo));
        while (mNextEntry > cLoopMin)
        {
            --mNextEntry;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, Factoradics::FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry));
            permutationIndexes[mNextEntry - tBlock.MinSetPosition] = GetAndRemovePosition_Fast(mValueAndRemainder.quot);
        }

        if constexpr (tBlock.CoinflipFinalTwo)
        {
            permutationIndexes[1] = GetCoinflip<true>();
            permutationIndexes[0] = GetCoinflip<false>();
        }
        else
        {
            --mNextEntry;
            permutationIndexes[mNextEntry - tBlock.MinSetPosition] = GetAndRemovePosition_Fast(mValueAndRemainder.rem);
        }
    }

    template<size_t tBlockIndex>
    inline void FillBlockAndFallThrough(std::span<SetRandomizerInternal::PermutationBlock>& permutationBlocks, std::span<int64_t> randomBits)
    {
        FillBlock<tBlockIndex>(permutationBlocks[tBlockIndex], randomBits[tBlockIndex]);
        if constexpr (tBlockIndex > 0)
        {
            FillBlockAndFallThrough<tBlockIndex - 1>(permutationBlocks, randomBits);
        }
    }

private:
    inline uint8_t GetAndRemovePosition_Fast(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        memmove(&mPositionSet[position], &mPositionSet[position + 1], (tMaxBlock.MaxFactorial - position) * sizeof(uint8_t));
        return positionValue;
    }

    inline uint8_t GetAndRemovePosition_Careful(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        if ((position + 1) < tMaxBlock.MaxFactorial)
        {
            memmove(&mPositionSet[position], &mPositionSet[position + 1], (tMaxBlock.MaxFactorial - position) * sizeof(uint8_t));
        }
        return positionValue;
    }

    template<bool tFirstPos>
    inline uint8_t GetCoinflip()
    {
        if constexpr (tFirstPos)
        {
            return mPositionSet[mValueAndRemainder.rem & 0b1];
        }
        else
        {
            return mPositionSet[(mValueAndRemainder.rem & 0b1) ^ 0b1];
        }
    }

    alignas(16) std::array<uint8_t, tMaxBlock.MaxFactorial> mPositionSet;
    int64_t mNextEntry;
    std::lldiv_t mValueAndRemainder;
};

void SetRandomizerInternal::Randomize(std::span<SetRandomizerInternal::PermutationBlock> permutationBlocks)
{
    const int64_t randomBits = MakeRandom();

    if (mSetSize < 2)
    {
        mShuffleMode = ShuffleMode::kNone;
        return;
    }

    if (mSetSize == 2)
    {
        // Caution: PermutationBuilder::FillStandardBlock expects a set size of atleast 3
        // This shuffle mode is here specifically to guard for that
        permutationBlocks[0][0] = (uint8_t)randomBits & 0b1;
        mShuffleMode = ShuffleMode::kCoinFlip;
        return;
    }

    if (mSetSize <= Factoradics::cBlocks[0].MaxFactorial)
    {
        PermutationBuilder<0> permutationBuilder(mSetSize);
        permutationBuilder.FillBlock<0>(permutationBlocks[0], randomBits);
        mShuffleMode = ShuffleMode::kPermutation;
        return;
    }

    if (permutationBlocks.size() == 1)
    {
        PermutationBuilder<0> permutationBuilder(mSetSize);
        permutationBuilder.FillBlock<0>(permutationBlocks[0], randomBits);
        mPermutationMultiplier = mSetSize / kPermutationIndexesPerBlock;
        mShuffleMode = ShuffleMode::kRepeatedShuffling;
        return;
    }

    if (permutationBlocks.size() <= Factoradics::cBlocks.size())
    {
        for (size_t blockIndex = 0; blockIndex < permutationBlocks.size(); ++blockIndex)
        {
            if (mSetSize <= Factoradics::cBlocks[blockIndex].MaxFactorial)
            {
                FillWithPermutationExtended(blockIndex, permutationBlocks);
                return;
            }
        }
    }

    // mSetSize > 1 && all other options exhausted
    {
        mPermutationMultiplier = mSetSize / kPermutationIndexesPerBlock;
        for (SetRandomizerInternal::PermutationBlock& permutationBlock : permutationBlocks)
        {
            const int64_t permutationRandomEx = MakeRandom();
            PermutationBuilder<0> permutationBuilder(mSetSize);
            permutationBuilder.FillBlock<0>(permutationBlock, permutationRandomEx);
        }

        mShuffleMode = ShuffleMode::kRepeatedShufflingWithBlockMixing;
    }
}

void SetRandomizerInternal::FillWithPermutationExtended(size_t maxBlockIndex, std::span<SetRandomizerInternal::PermutationBlock>& permutationBlocks)
{
    mShuffleMode = ShuffleMode::kPermutationExtended;

    std::vector<int64_t> randomBits;
    randomBits.reserve(maxBlockIndex + 1);
    for (size_t i = 0; i <= maxBlockIndex; ++i)
    {
        randomBits.push_back(MakeRandom());
    }
    switch (maxBlockIndex)
    {
    // [Factoradics::Block Limited]
    case 27: { PermutationBuilder<27> builder(mSetSize); builder.FillBlockAndFallThrough<27>(permutationBlocks, randomBits); } break;
    case 26: { PermutationBuilder<26> builder(mSetSize); builder.FillBlockAndFallThrough<26>(permutationBlocks, randomBits); } break;
    case 25: { PermutationBuilder<25> builder(mSetSize); builder.FillBlockAndFallThrough<25>(permutationBlocks, randomBits); } break;
    case 24: { PermutationBuilder<24> builder(mSetSize); builder.FillBlockAndFallThrough<24>(permutationBlocks, randomBits); } break;
    case 23: { PermutationBuilder<23> builder(mSetSize); builder.FillBlockAndFallThrough<23>(permutationBlocks, randomBits); } break;
    case 22: { PermutationBuilder<22> builder(mSetSize); builder.FillBlockAndFallThrough<22>(permutationBlocks, randomBits); } break;
    case 21: { PermutationBuilder<21> builder(mSetSize); builder.FillBlockAndFallThrough<21>(permutationBlocks, randomBits); } break;
    case 20: { PermutationBuilder<20> builder(mSetSize); builder.FillBlockAndFallThrough<20>(permutationBlocks, randomBits); } break;
    case 19: { PermutationBuilder<19> builder(mSetSize); builder.FillBlockAndFallThrough<19>(permutationBlocks, randomBits); } break;
    case 18: { PermutationBuilder<18> builder(mSetSize); builder.FillBlockAndFallThrough<18>(permutationBlocks, randomBits); } break;
    case 17: { PermutationBuilder<17> builder(mSetSize); builder.FillBlockAndFallThrough<17>(permutationBlocks, randomBits); } break;
    case 16: { PermutationBuilder<16> builder(mSetSize); builder.FillBlockAndFallThrough<16>(permutationBlocks, randomBits); } break;
    case 15: { PermutationBuilder<15> builder(mSetSize); builder.FillBlockAndFallThrough<15>(permutationBlocks, randomBits); } break;
    case 14: { PermutationBuilder<14> builder(mSetSize); builder.FillBlockAndFallThrough<14>(permutationBlocks, randomBits); } break;
    case 13: { PermutationBuilder<13> builder(mSetSize); builder.FillBlockAndFallThrough<13>(permutationBlocks, randomBits); } break;
    case 12: { PermutationBuilder<12> builder(mSetSize); builder.FillBlockAndFallThrough<12>(permutationBlocks, randomBits); } break;
    case 11: { PermutationBuilder<11> builder(mSetSize); builder.FillBlockAndFallThrough<11>(permutationBlocks, randomBits); } break;
    case 10: { PermutationBuilder<10> builder(mSetSize); builder.FillBlockAndFallThrough<10>(permutationBlocks, randomBits); } break;
    case  9: { PermutationBuilder< 9> builder(mSetSize); builder.FillBlockAndFallThrough< 9>(permutationBlocks, randomBits); } break;
    case  8: { PermutationBuilder< 8> builder(mSetSize); builder.FillBlockAndFallThrough< 8>(permutationBlocks, randomBits); } break;
    case  7: { PermutationBuilder< 7> builder(mSetSize); builder.FillBlockAndFallThrough< 7>(permutationBlocks, randomBits); } break;
    case  6: { PermutationBuilder< 6> builder(mSetSize); builder.FillBlockAndFallThrough< 6>(permutationBlocks, randomBits); } break;
    case  5: { PermutationBuilder< 5> builder(mSetSize); builder.FillBlockAndFallThrough< 5>(permutationBlocks, randomBits); } break;
    case  4: { PermutationBuilder< 4> builder(mSetSize); builder.FillBlockAndFallThrough< 4>(permutationBlocks, randomBits); } break;
    case  3: { PermutationBuilder< 3> builder(mSetSize); builder.FillBlockAndFallThrough< 3>(permutationBlocks, randomBits); } break;
    case  2: { PermutationBuilder< 2> builder(mSetSize); builder.FillBlockAndFallThrough< 2>(permutationBlocks, randomBits); } break;
    case  1: { PermutationBuilder< 1> builder(mSetSize); builder.FillBlockAndFallThrough< 1>(permutationBlocks, randomBits); } break;
    case  0: { PermutationBuilder< 0> builder(mSetSize); builder.FillBlockAndFallThrough< 0>(permutationBlocks, randomBits); } break;
    default: mShuffleMode = ShuffleMode::kNone; break;
    }
}

namespace
{
    template<bool MixedBlocks>
    inline uint32_t RepeatedShuffling(const uint32_t setSize, uint32_t index, uint32_t permutationMultiplier, std::span<const SetRandomizerInternal::PermutationBlock>& permutationBlocks)
    {
        const uint32_t shuffles = 3 + (((uint32_t)std::log2((double)setSize)) >> 1);
        for (uint32_t i = 0; true;)
        {
            if (index < (SetRandomizerInternal::kPermutationIndexesPerBlock * permutationMultiplier))
            {
                // Shuffle specifics
                const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::kPermutationIndexesPerBlock);

                uint32_t usedBlock = 0;
                if constexpr (MixedBlocks)
                {
                    usedBlock = i % permutationBlocks.size();
                }
                index = permutationBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::kPermutationIndexesPerBlock);
            }

            for (uint32_t revMultiplier = 2, combMult = permutationMultiplier >> 1;
                combMult > 0;
                revMultiplier <<= 1, combMult >>= 1)
            {
                const uint32_t blockSize = SetRandomizerInternal::kPermutationIndexesPerBlock * revMultiplier;
                if (index < (blockSize * combMult))
                {
                    const std::ldiv_t baseMultDiv = std::ldiv(index, revMultiplier);
                    index = baseMultDiv.quot;

                    const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::kPermutationIndexesPerBlock);
                    uint32_t usedBlock = 0;
                    if constexpr (MixedBlocks)
                    {
                        usedBlock = i % permutationBlocks.size();
                    }
                    index = permutationBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::kPermutationIndexesPerBlock);

                    index = (index * revMultiplier) + baseMultDiv.rem;
                }
            }

            if (++i; i >= shuffles)
            {
                break;
            }
            index = (index + (setSize >> 1)) % setSize;
        }
        return index;
    }
}

uint32_t SetRandomizerInternal::GetWheeledIndex(uint32_t index, std::span<const SetRandomizerInternal::PermutationBlock> permutationBlocks) const
{
    if (index > mSetSize)
    {
        return index;
    }

    switch (mShuffleMode)
    {
    default:
    case ShuffleMode::kNone:
        return mSetSize - 1;

    case ShuffleMode::kCoinFlip:
        return (index ^ permutationBlocks[0][0]) & 0b1;

    case ShuffleMode::kPermutation:
        return permutationBlocks[0][index];


    case ShuffleMode::kPermutationExtended:
    {
        for (int64_t blockIndex = 0; blockIndex < (int64_t)permutationBlocks.size(); ++blockIndex)
        {
            if (index < Factoradics::cBlocks[blockIndex].MaxFactorial)
            {
                return permutationBlocks[blockIndex][index - Factoradics::cBlocks[blockIndex].MinSetPosition];
            }
        }
        return index;
    }

    /**
     * For both RepeatedShuffling types:
     *
     *  We have to repeat the shuffle at least 3 times in order to de-bias the remainder ("cutting" the set each time)
     *  Since every shuffle has to be in increasing block sizes (starting from 19),
     *  there will be an "unshuffled" bit at the bottom, which we need to shuffle in and then out
     *  e.g. At a set size of 200, with a divisor of 19, we'd have 10 indexes unshuffled at the end
     *  So we:
     *      Shuffle the 190, leaving an unshuffled 10
     *      Split the bottom 100 to the top.
     *      Shuffle again (now including the unshuffled 10).
     *          Note: This puts us in a state where the final 10 are guaranteed to NOT be the largest values
     *          This is non-ideal. It biases the shuffler to have the bottom 190 be slightly larger if we left it like this
     *      Split the bottom 100 to the top (again).
     *      Shuffle again, redistributing the once-lowered 10 back into the base 190
     */
    case ShuffleMode::kRepeatedShuffling:
    {
        return RepeatedShuffling<false>(mSetSize, index, mPermutationMultiplier, permutationBlocks);
    }

    case ShuffleMode::kRepeatedShufflingWithBlockMixing:
    {
        return RepeatedShuffling<true>(mSetSize, index, mPermutationMultiplier, permutationBlocks);
    }
    }
}

int64_t SetRandomizerInternal::MakeRandom() const
{
    return ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
}
