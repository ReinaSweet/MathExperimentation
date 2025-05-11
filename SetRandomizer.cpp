#include "SetRandomizer.h"

#include <cmath>
#include <cstring>
#include <cstdlib>


namespace Factoradics
{
    struct Block
    {
        int64_t Min;
        int64_t Max;
        bool CoinflipFinalTwo;
    };

    template<int64_t Min, int64_t Increment, bool CoinflipFinalTwo = false>
    consteval Block DeclareBlock()
    {
        return Block{ Min, Min + Increment, CoinflipFinalTwo };
    }

    constexpr std::array<Block, 13> cBlocks
    {
        DeclareBlock<0, 20, true>(),
        DeclareBlock<20, 12>(),
        DeclareBlock<32, 11>(),
        DeclareBlock<43, 10>(),

        DeclareBlock<53, 9>(),
        DeclareBlock<62, 9>(),
        DeclareBlock<71, 9>(),

        DeclareBlock<80, 8>(),
        DeclareBlock<88, 8>(),
        DeclareBlock<96, 8>(),
        DeclareBlock<104, 8>(),
        DeclareBlock<112, 8>(),
        DeclareBlock<120, 8>()
    };

    template<int64_t Max>
    consteval int64_t ConstMax(int64_t value)
    {
        return (value > Max) ? Max : value;
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

    template<int64_t Min, int64_t Max>
    constexpr int64_t FactorialRange(int64_t m)
    {
        switch (m)
        {
        // Uncomment this to see the constexpr fail for running out of bit space
        // case (Min + 21): return ConstFactorialRange(Min, ConstMax<Max + 1>(Min + 21));
        case (Min + 20): return ConstFactorialRange(Min, ConstMax<Max>(Min + 20));
        case (Min + 19): return ConstFactorialRange(Min, ConstMax<Max>(Min + 19));
        case (Min + 18): return ConstFactorialRange(Min, ConstMax<Max>(Min + 18));
        case (Min + 17): return ConstFactorialRange(Min, ConstMax<Max>(Min + 17));
        case (Min + 16): return ConstFactorialRange(Min, ConstMax<Max>(Min + 16));
        case (Min + 15): return ConstFactorialRange(Min, ConstMax<Max>(Min + 15));
        case (Min + 14): return ConstFactorialRange(Min, ConstMax<Max>(Min + 14));
        case (Min + 13): return ConstFactorialRange(Min, ConstMax<Max>(Min + 13));
        case (Min + 12): return ConstFactorialRange(Min, ConstMax<Max>(Min + 12));
        case (Min + 11): return ConstFactorialRange(Min, ConstMax<Max>(Min + 11));
        case (Min + 10): return ConstFactorialRange(Min, ConstMax<Max>(Min + 10));
        case (Min + 9): return ConstFactorialRange(Min, ConstMax<Max>(Min + 9));
        case (Min + 8): return ConstFactorialRange(Min, ConstMax<Max>(Min + 8));
        case (Min + 7): return ConstFactorialRange(Min, ConstMax<Max>(Min + 7));
        case (Min + 6): return ConstFactorialRange(Min, ConstMax<Max>(Min + 6));
        case (Min + 5): return ConstFactorialRange(Min, ConstMax<Max>(Min + 5));
        case (Min + 4): return ConstFactorialRange(Min, ConstMax<Max>(Min + 4));
        case (Min + 3): return ConstFactorialRange(Min, ConstMax<Max>(Min + 3));
        case (Min + 2): return ConstFactorialRange(Min, ConstMax<Max>(Min + 2));
        case (Min + 1): return ConstFactorialRange(Min, ConstMax<Max>(Min + 1));
        case (Min): return ConstFactorialRange(Min, Min);
        default: return 1;
        }
    }
};

/**
 * Build an indexed permutation sequence for our current random seed
 * See: https://oeis.org/A030299 (but off by 1)
 */
template<size_t tMaxBlockIndex, Factoradics::Block tMaxBlock = Factoradics::cBlocks[tMaxBlockIndex]>
class CombinatoricBuilder
{
public:
    CombinatoricBuilder(int32_t setSize)
        : mNextCombinatoricSize(setSize > tMaxBlock.Max ? tMaxBlock.Max : setSize)
        , mValueAndRemainder{0,0}
    {
        for (uint8_t i = 0; i < mNextCombinatoricSize; ++i)
        {
            mPositionSet[i] = i;
        }
    }

    template<size_t tBlockIndex, Factoradics::Block tBlock = Factoradics::cBlocks[tBlockIndex]>
    void FillBlock(SetRandomizerInternal::CombinatoricBlock& combinatoricIndexes, int64_t combinatoricRandom)
    {
        mValueAndRemainder = { 0, combinatoricRandom % Factoradics::FactorialRange<tBlock.Min, tBlock.Max>(mNextCombinatoricSize)};

        --mNextCombinatoricSize;
        mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, Factoradics::FactorialRange<tBlock.Min, tBlock.Max>(mNextCombinatoricSize));
        combinatoricIndexes[mNextCombinatoricSize - tBlock.Min] = GetAndRemovePosition_Careful(mValueAndRemainder.quot);

        constexpr int64_t cLoopMin = (tBlock.Min + ((int64_t)tBlock.CoinflipFinalTwo * 2));
        while (mNextCombinatoricSize > cLoopMin)
        {
            --mNextCombinatoricSize;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, Factoradics::FactorialRange<tBlock.Min, tBlock.Max>(mNextCombinatoricSize));
            combinatoricIndexes[mNextCombinatoricSize - tBlock.Min] = GetAndRemovePosition_Fast(mValueAndRemainder.quot);
        }

        if constexpr (tBlock.CoinflipFinalTwo)
        {
            combinatoricIndexes[1] = mPositionSet[mValueAndRemainder.rem & 0b1];
            combinatoricIndexes[0] = mPositionSet[!(mValueAndRemainder.rem & 0b1)];
        }
    }

    void FillAllBlocks(std::span<SetRandomizerInternal::CombinatoricBlock> combinatoricBlocks, std::span<int64_t> randomBits)
    {
        switch (combinatoricBlocks.size())
        {
        case 13: FillBlock<12>(combinatoricBlocks[12], randomBits[12]); [[fallthrough]];
        case 12: FillBlock<11>(combinatoricBlocks[11], randomBits[11]); [[fallthrough]];
        case 11: FillBlock<10>(combinatoricBlocks[10], randomBits[10]); [[fallthrough]];
        case 10: FillBlock< 9>(combinatoricBlocks[9], randomBits[9]); [[fallthrough]];
        case  9: FillBlock< 8>(combinatoricBlocks[8], randomBits[8]); [[fallthrough]];
        case  8: FillBlock< 7>(combinatoricBlocks[7], randomBits[7]); [[fallthrough]];
        case  7: FillBlock< 6>(combinatoricBlocks[6], randomBits[6]); [[fallthrough]];
        case  6: FillBlock< 5>(combinatoricBlocks[5], randomBits[5]); [[fallthrough]];
        case  5: FillBlock< 4>(combinatoricBlocks[4], randomBits[4]); [[fallthrough]];
        case  4: FillBlock< 3>(combinatoricBlocks[3], randomBits[3]); [[fallthrough]];
        case  3: FillBlock< 2>(combinatoricBlocks[2], randomBits[2]); [[fallthrough]];
        case  2: FillBlock< 1>(combinatoricBlocks[1], randomBits[1]); [[fallthrough]];
        case  1: FillBlock< 0>(combinatoricBlocks[0], randomBits[0]); [[fallthrough]];
        default:
            break;
        }
    }

private:
    inline uint8_t GetAndRemovePosition_Fast(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        memmove(&mPositionSet[position], &mPositionSet[position + 1], (tMaxBlock.Max - position) * sizeof(uint8_t));
        return positionValue;
    }

    inline uint8_t GetAndRemovePosition_Careful(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        if ((position + 1) < tMaxBlock.Max)
        {
            memmove(&mPositionSet[position], &mPositionSet[position + 1], (tMaxBlock.Max - position) * sizeof(uint8_t));
        }
        return positionValue;
    }

    std::array<uint8_t, tMaxBlock.Max> mPositionSet;
    int64_t mNextCombinatoricSize;
    std::lldiv_t mValueAndRemainder;
};

void SetRandomizerInternal::Randomize(std::span<SetRandomizerInternal::CombinatoricBlock> combinatoricBlocks)
{
    const int64_t combinatoricRandom = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();

    if (mSetSize < 2)
    {
        mShuffleMode = ShuffleMode::kNone;
        return;
    }

    if (mSetSize == 2)
    {
        // Caution: CombinatoricBuilder::FillStandardBlock expects a set size of atleast 3
        // This shuffle mode is here specifically to guard for that
        combinatoricBlocks[0][0] = (uint8_t)combinatoricRandom & 0b1;
        mShuffleMode = ShuffleMode::kCoinFlip;
        return;
    }

    if (mSetSize <= Factoradics::cBlocks[0].Max)
    {
        CombinatoricBuilder<0> combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillBlock<0>(combinatoricBlocks[0], combinatoricRandom);
        mShuffleMode = ShuffleMode::kCombinatoric;
        return;
    }

    if (combinatoricBlocks.size() == 1)
    {
        CombinatoricBuilder<0> combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillBlock<0>(combinatoricBlocks[0], combinatoricRandom);
        mCombinatoricMultiplier = mSetSize / kNumCombinatoricIndexes;
        mShuffleMode = ShuffleMode::kRepeatedShuffling;
        return;
    }

    if (combinatoricBlocks.size() <= Factoradics::cBlocks.size())
    {
        for (int64_t blockIndex = combinatoricBlocks.size() - 1; blockIndex >= 0; --blockIndex)
        {
            if (mSetSize <= Factoradics::cBlocks[blockIndex].Max)
            {
                FillWithPermutationExtended(blockIndex, combinatoricBlocks);
                return;
            }
        }
    }

    // mSetSize > 1 && all other options exhausted
    {
        mCombinatoricMultiplier = mSetSize / kNumCombinatoricIndexes;
        for (SetRandomizerInternal::CombinatoricBlock& combinatoricBlock : combinatoricBlocks)
        {
            const int64_t combinatoricRandomEx = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
            CombinatoricBuilder<0> combinatoricBuilder(mSetSize);
            combinatoricBuilder.FillBlock<0>(combinatoricBlock, combinatoricRandomEx);
        }

        mShuffleMode = ShuffleMode::kRepeatedShufflingWithBlockMixing;
    }
}

void SetRandomizerInternal::FillWithPermutationExtended(size_t maxBlockIndex, std::span<SetRandomizerInternal::CombinatoricBlock>& combinatoricBlocks)
{
    mShuffleMode = ShuffleMode::kCombinatoricExtended;

    std::vector<int64_t> randomBits;
    randomBits.reserve(maxBlockIndex + 1);
    for (size_t i = 0; i <= maxBlockIndex; ++i)
    {
        randomBits.push_back(MakeRandom());
    }
    switch (maxBlockIndex)
    {
    case 12: { CombinatoricBuilder<12> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case 11: { CombinatoricBuilder<11> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case 10: { CombinatoricBuilder<10> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  9: { CombinatoricBuilder< 9> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  8: { CombinatoricBuilder< 8> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  7: { CombinatoricBuilder< 7> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  6: { CombinatoricBuilder< 6> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  5: { CombinatoricBuilder< 5> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  4: { CombinatoricBuilder< 4> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  3: { CombinatoricBuilder< 3> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  2: { CombinatoricBuilder< 2> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  1: { CombinatoricBuilder< 1> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    case  0: { CombinatoricBuilder< 0> builder(mSetSize); builder.FillAllBlocks(combinatoricBlocks, randomBits); } break;
    default: mShuffleMode = ShuffleMode::kNone; break;
    }
}

namespace
{
    template<bool MixedBlocks>
    inline uint32_t RepeatedShuffling(const uint32_t setSize, uint32_t index, uint32_t combinatoricMultiplier, std::span<const SetRandomizerInternal::CombinatoricBlock>& combinatoricBlocks)
    {
        const uint32_t shuffles = 3 + (((uint32_t)std::log2((double)setSize)) >> 1);
        for (uint32_t i = 0; true;)
        {
            if (index < (SetRandomizerInternal::kNumCombinatoricIndexes * combinatoricMultiplier))
            {
                // Shuffle specifics
                const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::kNumCombinatoricIndexes);

                uint32_t usedBlock = 0;
                if constexpr (MixedBlocks)
                {
                    usedBlock = i % combinatoricBlocks.size();
                }
                index = combinatoricBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::kNumCombinatoricIndexes);
            }

            for (uint32_t revMultiplier = 2, combMult = combinatoricMultiplier >> 1;
                combMult > 0;
                revMultiplier <<= 1, combMult >>= 1)
            {
                const uint32_t blockSize = SetRandomizerInternal::kNumCombinatoricIndexes * revMultiplier;
                if (index < (blockSize * combMult))
                {
                    const std::ldiv_t baseMultDiv = std::ldiv(index, revMultiplier);
                    index = baseMultDiv.quot;

                    const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::kNumCombinatoricIndexes);
                    uint32_t usedBlock = 0;
                    if constexpr (MixedBlocks)
                    {
                        usedBlock = i % combinatoricBlocks.size();
                    }
                    index = combinatoricBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::kNumCombinatoricIndexes);

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

uint32_t SetRandomizerInternal::GetWheeledIndex(uint32_t index, std::span<const SetRandomizerInternal::CombinatoricBlock> combinatoricBlocks) const
{
    switch (mShuffleMode)
    {
    default:
    case ShuffleMode::kNone:
        return mSetSize - 1;

    case ShuffleMode::kCoinFlip:
        return (index ^ combinatoricBlocks[0][0]) & 0b1;

    case ShuffleMode::kCombinatoric:
        return combinatoricBlocks[0][index];


    case ShuffleMode::kCombinatoricExtended:
    {
        for (int64_t blockIndex = 0; blockIndex < (int64_t)combinatoricBlocks.size(); ++blockIndex)
        {
            if (index < Factoradics::cBlocks[blockIndex].Max)
            {
                return combinatoricBlocks[blockIndex][index - Factoradics::cBlocks[blockIndex].Min];
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
        return RepeatedShuffling<false>(mSetSize, index, mCombinatoricMultiplier, combinatoricBlocks);
    }

    case ShuffleMode::kRepeatedShufflingWithBlockMixing:
    {
        return RepeatedShuffling<true>(mSetSize, index, mCombinatoricMultiplier, combinatoricBlocks);
    }
    }
}

int64_t SetRandomizerInternal::MakeRandom() const
{
    return ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
}
