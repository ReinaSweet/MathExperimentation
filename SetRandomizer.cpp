#include "SetRandomizer.h"

#include <cmath>
#include <cstring>
#include <cstdlib>

/**
 * Build an indexed permutation sequence for our current random seed
 * See: https://oeis.org/A030299 (but off by 1)
 */
template<size_t NumPositions = SetRandomizerInternal::kNumCombinatoricIndexes>
class CombinatoricBuilder
{
public:
    CombinatoricBuilder(int32_t setSize)
        : mNextCombinatoricSize(setSize > NumPositions ? NumPositions : setSize)
        , mValueAndRemainder{0,0}
    {
        for (uint8_t i = 0; i < mNextCombinatoricSize; ++i)
        {
            mPositionSet[i] = i;
        }
    }

    void FillExtendedExtendedBlock(SetRandomizerInternal::CombinatoricBlock& combinatoricIndexes, int64_t combinatoricRandom)
    {
        mValueAndRemainder = { 0, combinatoricRandom % FactorialRangeExtendedExtended(mNextCombinatoricSize) };

        --mNextCombinatoricSize;
        mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeExtendedExtended(mNextCombinatoricSize));
        combinatoricIndexes[mNextCombinatoricSize - SetRandomizerInternal::kNumCombinatoricIndexesExtended] = GetAndRemovePosition_Careful(mValueAndRemainder.quot);

        while (mNextCombinatoricSize > SetRandomizerInternal::kNumCombinatoricIndexesExtended)
        {
            --mNextCombinatoricSize;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeExtendedExtended(mNextCombinatoricSize));
            combinatoricIndexes[mNextCombinatoricSize - SetRandomizerInternal::kNumCombinatoricIndexesExtended] = GetAndRemovePosition_Fast(mValueAndRemainder.quot);
        }
    }

    void FillExtendedBlock(SetRandomizerInternal::CombinatoricBlock& combinatoricIndexes, int64_t combinatoricRandom)
    {
        mValueAndRemainder = { 0, combinatoricRandom % FactorialRangeExtended(mNextCombinatoricSize) };

        --mNextCombinatoricSize;
        mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeExtended(mNextCombinatoricSize));
        combinatoricIndexes[mNextCombinatoricSize - SetRandomizerInternal::kNumCombinatoricIndexes] = GetAndRemovePosition_Careful(mValueAndRemainder.quot);

        while (mNextCombinatoricSize > SetRandomizerInternal::kNumCombinatoricIndexes)
        {
            --mNextCombinatoricSize;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeExtended(mNextCombinatoricSize));
            combinatoricIndexes[mNextCombinatoricSize - SetRandomizerInternal::kNumCombinatoricIndexes] = GetAndRemovePosition_Fast(mValueAndRemainder.quot);
        }
    }

    void FillStandardBlock(SetRandomizerInternal::CombinatoricBlock& combinatoricIndexes, int64_t combinatoricRandom)
    {
        mValueAndRemainder = { 0, combinatoricRandom % FactorialRangeBase(mNextCombinatoricSize) };

        --mNextCombinatoricSize;
        mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeBase(mNextCombinatoricSize));
        combinatoricIndexes[mNextCombinatoricSize] = GetAndRemovePosition_Careful(mValueAndRemainder.quot);

        while (mNextCombinatoricSize > 2)
        {
            --mNextCombinatoricSize;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRangeBase(mNextCombinatoricSize));
            combinatoricIndexes[mNextCombinatoricSize] = GetAndRemovePosition_Fast(mValueAndRemainder.quot);
        } 
        combinatoricIndexes[1] = mPositionSet[mValueAndRemainder.rem & 0b1];
        combinatoricIndexes[0] = mPositionSet[!(mValueAndRemainder.rem & 0b1)];
    }

private:
    uint8_t GetAndRemovePosition_Fast(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        memmove(&mPositionSet[position], &mPositionSet[position + 1], (NumPositions - position) * sizeof(uint8_t));
        return positionValue;
    }

    uint8_t GetAndRemovePosition_Careful(int64_t position)
    {
        const uint8_t positionValue = mPositionSet[position];
        if ((position + 1) < mPositionSet.size())
        {
            memmove(&mPositionSet[position], &mPositionSet[position + 1], (NumPositions - position) * sizeof(uint8_t));
        }
        return positionValue;
    }

    static constexpr int64_t GetBlockLowerInclusiveBound(int64_t blockIndex)
    {
        if (blockIndex >= 21) { return 21; }
        return 1;
    }

    static consteval int64_t ConstFactorialRange(int64_t min, int64_t max)
    {
        int64_t value = min;
        for (; max > min; --max)
        {
            value *= max;
        }
        return value;
    }

    static constexpr int64_t FactorialRangeExtendedExtended(int64_t m)
    {
        switch (m)
        {
        // Uncomment this to see the constexpr fail for running out of bit space
        // case 44: return ConstFactorialRange(32, 44);
        case 43: return ConstFactorialRange(32, 43);
        case 42: return ConstFactorialRange(32, 42);
        case 41: return ConstFactorialRange(32, 41);
        case 40: return ConstFactorialRange(32, 40);
        case 39: return ConstFactorialRange(32, 39);
        case 38: return ConstFactorialRange(32, 38);
        case 37: return ConstFactorialRange(32, 37);
        case 36: return ConstFactorialRange(32, 36);
        case 35: return ConstFactorialRange(32, 35);
        case 34: return ConstFactorialRange(32, 34);
        case 33: return ConstFactorialRange(32, 33);
        case 32: return ConstFactorialRange(32, 32);
        default: return 1;
        }
    }

    static constexpr int64_t FactorialRangeExtended(int64_t m)
    {
        switch (m)
        {
        // Uncomment this to see the constexpr fail for running out of bit space
        // case 33: return ConstFactorialRange(20, 33);
        case 32: return ConstFactorialRange(20, 32);
        case 31: return ConstFactorialRange(20, 31);
        case 30: return ConstFactorialRange(20, 30);
        case 29: return ConstFactorialRange(20, 29);
        case 28: return ConstFactorialRange(20, 28);
        case 27: return ConstFactorialRange(20, 27);
        case 26: return ConstFactorialRange(20, 26);
        case 25: return ConstFactorialRange(20, 25);
        case 24: return ConstFactorialRange(20, 24);
        case 23: return ConstFactorialRange(20, 23);
        case 22: return ConstFactorialRange(20, 22);
        case 21: return ConstFactorialRange(20, 21);
        case 20: return ConstFactorialRange(20, 20);
        default: return 1;
        }
    }

    static constexpr int64_t FactorialRangeBase(int64_t m)
    {
        switch (m)
        {
        // Uncomment this to see the constexpr fail for running out of bit space
        // case 21: return ConstFactorialRange(1, 21);
        case 20: return ConstFactorialRange(1, 20);
        case 19: return ConstFactorialRange(1, 19);
        case 18: return ConstFactorialRange(1, 18);
        case 17: return ConstFactorialRange(1, 17);
        case 16: return ConstFactorialRange(1, 16);
        case 15: return ConstFactorialRange(1, 15);
        case 14: return ConstFactorialRange(1, 14);
        case 13: return ConstFactorialRange(1, 13);
        case 12: return ConstFactorialRange(1, 12);
        case 11: return ConstFactorialRange(1, 11);
        case 10: return ConstFactorialRange(1, 10);
        case 9: return ConstFactorialRange(1, 9);
        case 8: return ConstFactorialRange(1, 8);
        case 7: return ConstFactorialRange(1, 7);
        case 6: return ConstFactorialRange(1, 6);
        case 5: return ConstFactorialRange(1, 5);
        case 4: return ConstFactorialRange(1, 4);
        case 3: return ConstFactorialRange(1, 3);
        case 2: return ConstFactorialRange(1, 2);
        case 1: return ConstFactorialRange(1, 1);
        default: return 1;
        }
    }

    std::array<uint8_t, NumPositions> mPositionSet;
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

    if (mSetSize <= kNumCombinatoricIndexes)
    {
        CombinatoricBuilder combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillStandardBlock(combinatoricBlocks[0], combinatoricRandom);
        mShuffleMode = ShuffleMode::kCombinatoric;
        return;
    }

    if (combinatoricBlocks.size() == 1)
    {
        CombinatoricBuilder combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillStandardBlock(combinatoricBlocks[0], combinatoricRandom);
        mCombinatoricMultiplier = mSetSize / kNumCombinatoricIndexes;
        mShuffleMode = ShuffleMode::kRepeatedShuffling;
        return;
    }

    if (mSetSize <= SetRandomizerInternal::kNumCombinatoricIndexesExtended)
    {
        const int64_t combinatoricRandom2 = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
        CombinatoricBuilder<SetRandomizerInternal::kNumCombinatoricIndexesExtended> combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillExtendedBlock(combinatoricBlocks[1], combinatoricRandom2);
        combinatoricBuilder.FillStandardBlock(combinatoricBlocks[0], combinatoricRandom);

        mShuffleMode = ShuffleMode::kCombinatoricExtended;
        return;
    }

    if (combinatoricBlocks.size() > 2 && mSetSize <= SetRandomizerInternal::kNumCombinatoricIndexesExtendedExtended)
    {
        const int64_t combinatoricRandom2 = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
        const int64_t combinatoricRandom3 = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
        CombinatoricBuilder<SetRandomizerInternal::kNumCombinatoricIndexesExtendedExtended> combinatoricBuilder(mSetSize);
        combinatoricBuilder.FillExtendedExtendedBlock(combinatoricBlocks[2], combinatoricRandom3);
        combinatoricBuilder.FillExtendedBlock(combinatoricBlocks[1], combinatoricRandom2);
        combinatoricBuilder.FillStandardBlock(combinatoricBlocks[0], combinatoricRandom);

        mShuffleMode = ShuffleMode::kCombinatoricExtendedExtended;
        return;
    }

    // mSetSize > 1 && all other options exhausted
    {
        mCombinatoricMultiplier = mSetSize / kNumCombinatoricIndexes;
        for (SetRandomizerInternal::CombinatoricBlock& combinatoricBlock : combinatoricBlocks)
        {
            const int64_t combinatoricRandomEx = ((int64_t)mRandomFunc() << 32) & INT64_MAX | (int64_t)mRandomFunc();
            CombinatoricBuilder combinatoricBuilder(mSetSize);
            combinatoricBuilder.FillStandardBlock(combinatoricBlock, combinatoricRandomEx);
        }

        mShuffleMode = ShuffleMode::kRepeatedShufflingWithBlockMixing;
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
        if (index >= kNumCombinatoricIndexes) [[unlikely]]
        {
            return combinatoricBlocks[1][index - kNumCombinatoricIndexes];
        }
        return combinatoricBlocks[0][index];
    }

    case ShuffleMode::kCombinatoricExtendedExtended:
    {
        if (index >= kNumCombinatoricIndexesExtended) [[unlikely]]
        {
            return combinatoricBlocks[2][index - kNumCombinatoricIndexesExtended];
        }
        if (index >= kNumCombinatoricIndexes) [[unlikely]]
        {
            return combinatoricBlocks[1][index - kNumCombinatoricIndexes];
        }
        return combinatoricBlocks[0][index];
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
