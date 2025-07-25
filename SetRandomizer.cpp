#include "SetRandomizer.h"

#include <bit>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <numeric>

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
*   Rejection Sampling
* 
* Shuffle Math
*   If setsize > max && (setsize / max) < 2:
*   else: shift via remainder
*
* RNG
*   Document bias in the RNG from modulos that aren't a power of 2
*   Allow callers to feed obfuscating into the RNG
*   Remove RNG function. Replace with fed in RNG only
*   This should also remove the thread-unsafe std::vector
* 
* Refactors
*   cPermutationIndexesPerBlock replaced with Block's MaxFactorial
*   Double check thread safety
*   [[noexcept]]
*   [[nodiscard]]
*/


namespace Factoradics
{
    using bits = int64_t;
    using setsize = size_t;

    constexpr size_t cNumBlock = 28;
    
    template<bits tMax>
    consteval bits ConstMax(bits value)
    {
        return (value > tMax) ? tMax : value;
    }

    consteval bits ConstFactorialRange(bits min, bits max)
    {
        bits value = min == 0 ? 1 : min;
        for (; max > min; --max)
        {
            value *= max;
        }
        return value;
    }

    template<bits MinFactorial, bits MaxFactorial>
    constexpr bits FactorialRange(setsize m)
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


    struct Block
    {
        setsize MinFactorial = 1;
        setsize MaxFactorial;
        setsize MinSetPosition = 0;
        bits MaxNumOfMaxFactoradic = 0;
        bool CoinflipFinalTwo = false;

        constexpr Block(setsize entries)
            : MaxFactorial(entries)
        {}
    };

    constexpr std::array<Block, cNumBlock> cBlocks = []() consteval
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

        /**
        If the safe multiplier of the final factoradic base in a block is a divisor (to an integer) of the first base of
        the next block, then we can combine we perform the division and include that last bit of information in the next

        Or combine multiple of them over many blocks
        */

        setsize baseFactorial = 0;
        for (Block& block : blocks)
        {
            const setsize nextBase = baseFactorial + block.MaxFactorial;
            block.MinFactorial += baseFactorial;
            block.MaxFactorial += baseFactorial;
            block.MinSetPosition += baseFactorial;

            const bits unbiasedMaxBase = ConstFactorialRange(block.MinFactorial, block.MaxFactorial);
            block.MaxNumOfMaxFactoradic = std::numeric_limits<bits>::max() / unbiasedMaxBase; // Truncation intentional

            baseFactorial = nextBase;
        }
        blocks[0].CoinflipFinalTwo = true;

        return blocks;
    }();

    consteval Block GetBlock(setsize index)
    {
        return cBlocks[index];
    }

    constexpr const Block& GetBlockAtRunTime(setsize index)
    {
        return cBlocks[index];
    }

    /**
     * Build an indexed permutation sequence for our current random seed
     * See: https://oeis.org/A030299 (but off by 1)
     * 
     * ... Technically no longer true (because of the new set indexing)
     */
    template<setsize tMaxBlockIndex, Block tMaxBlock = GetBlock(tMaxBlockIndex)>
    class PermutationBuilder
    {
    public:
        PermutationBuilder(int32_t setSize)
            : mNextEntry(setSize > tMaxBlock.MaxFactorial ? tMaxBlock.MaxFactorial : setSize)
        {
            std::iota(mPositionSet.begin(), mPositionSet.end(), 0);
        }

        template<size_t tBlockIndex, Block tBlock = GetBlock(tBlockIndex)>
        void FillBlock(SetRandomizerInternal::PermutationBlock& permutationIndexes, int64_t randomBits)
        {
            // TODO: Rejection Sampling

            mValueAndRemainder = { 0, randomBits % FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry) };

            --mNextEntry;
            mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry));
            permutationIndexes[mNextEntry - tBlock.MinSetPosition] = GetAndRemovePosition(mValueAndRemainder.quot);

            constexpr int64_t cLoopMin = (tBlock.MinFactorial + ((int64_t)tBlock.CoinflipFinalTwo));
            while (mNextEntry > cLoopMin)
            {
                --mNextEntry;
                mValueAndRemainder = std::lldiv(mValueAndRemainder.rem, FactorialRange<tBlock.MinFactorial, tBlock.MaxFactorial>(mNextEntry));
                permutationIndexes[mNextEntry - tBlock.MinSetPosition] = GetAndRemovePosition(mValueAndRemainder.quot);
            }

            if constexpr (tBlock.CoinflipFinalTwo)
            {
                const size_t finalBit = mValueAndRemainder.rem & 0b1;
                permutationIndexes[finalBit] = mPositionSet[1];
                permutationIndexes[finalBit ^ 0b1] = mPositionSet[0];
            }
            else
            {
                --mNextEntry;
                const uint8_t finalEntry = GetAndRemovePosition(mValueAndRemainder.rem);
                const int64_t finalEntryIndex = mNextEntry - tBlock.MinSetPosition;
                permutationIndexes[finalEntryIndex] = finalEntry;
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
        inline uint8_t GetAndRemovePosition(int64_t position)
        {
            const uint8_t positionValue = mPositionSet[position];
            mPositionSet[position] = mPositionSet[mNextEntry];
            return positionValue;
        }

        alignas(16) std::array<uint8_t, tMaxBlock.MaxFactorial> mPositionSet;
        int64_t mNextEntry;
        std::lldiv_t mValueAndRemainder;
    };
};


void SetRandomizerInternal::RandomizeSingleBlock(SetRandomizerInternal::PermutationBlock& permutationBlock) noexcept
{
    using namespace Factoradics;
    const int64_t randomBits = MakeRandom();
    mShuffleMode = ShuffleMode::None;

    if (mSetSize < 2)
    {
        return;
    }

    if (mSetSize == 2)
    {
        // Caution: PermutationBuilder::FillStandardBlock expects a set size of atleast 3
        // This shuffle mode is here specifically to guard for that
        permutationBlock[0] = (uint8_t)randomBits & 0b1;
        mShuffleMode = ShuffleMode::CoinFlip;
        return;
    }

    PermutationBuilder<0> permutationBuilder(mSetSize);
    permutationBuilder.FillBlock<0>(permutationBlock, randomBits);

    if (mSetSize <= GetBlock(0).MaxFactorial)
    {
        mShuffleMode = ShuffleMode::Permutation;
        return;
    }

    // TODO: Deal with sets sized 30~39, esp 35~39
    mPermutationMultiplier = mSetSize / cPermutationIndexesPerBlock;
    mShuffleMode = ShuffleMode::RepeatedShuffling;
    return;
}

template<SetRandomizerInternal::ShuffleDataSize tDataSize>
void SetRandomizerInternal::Randomize(std::span<SetRandomizerInternal::PermutationBlock> permutationBlocks)
{
    using namespace Factoradics;
    const int64_t randomBits = MakeRandom();
    mShuffleMode = ShuffleMode::None;

    if (mSetSize < 2)
    {
        return;
    }

    if (mSetSize == 2)
    {
        // Caution: PermutationBuilder::FillStandardBlock expects a set size of atleast 3
        // This shuffle mode is here specifically to guard for that
        permutationBlocks[0][0] = (uint8_t)randomBits & 0b1;
        mShuffleMode = ShuffleMode::CoinFlip;
        return;
    }

    if (mSetSize <= GetBlock(0).MaxFactorial)
    {
        PermutationBuilder<0> permutationBuilder(mSetSize);
        permutationBuilder.FillBlock<0>(permutationBlocks[0], randomBits);
        mShuffleMode = ShuffleMode::Permutation;
        return;
    }

    if (mSetSize <= cBlocks.back().MaxFactorial)
    {
        const size_t blockSize = std::min(permutationBlocks.size(), cBlocks.size());

        if constexpr (tDataSize == SetRandomizerInternal::ShuffleDataSize::Small)
        {
            for (size_t blockIndex = 0; blockIndex < blockSize; ++blockIndex)
            {
                if (mSetSize <= GetBlockAtRunTime(blockIndex).MaxFactorial)
                {
                    FillWithPermutationExtended(blockIndex, permutationBlocks);
                    return;
                }
            }
        }
        else if constexpr (tDataSize == SetRandomizerInternal::ShuffleDataSize::Large)
        {
            /**
            * This polynomial fits set size to be equal to or 1 greater than what fits in each block
            * Doing this only lets us save a few checks. Which is why we don't both with smaller permutation block sizes
            */
            const double setAsDouble = (double)mSetSize;
            size_t blockIndex = (size_t)(1.177 + (0.07143 * setAsDouble) + (0.000162 * setAsDouble * setAsDouble));
            blockIndex = std::min(blockIndex, blockSize - 1);

            for (; blockIndex > 0; --blockIndex)
            {
                if (mSetSize > GetBlockAtRunTime(blockIndex).MinSetPosition)
                {
                    FillWithPermutationExtended(blockIndex, permutationBlocks);
                    return;
                }
            }
        }


        // The 0th block index was already verified as not valid by this point. Reaching this point should be impossible
        // See above: mShuffleMode = ShuffleMode::Permutation;
        mShuffleMode = ShuffleMode::None;
        return;
    }

    // TODO: Don't do block mixing
    // Instead do permutation extended.
    // And handle weird cases of [1.5x, 2.0x) blocks of data
    // Or more so especially, [1.75x, 2.0x)

    // mSetSize > 1 && all other options exhausted
    {
        mPermutationMultiplier = mSetSize / cPermutationIndexesPerBlock;
        for (SetRandomizerInternal::PermutationBlock& permutationBlock : permutationBlocks)
        {
            const int64_t permutationRandomEx = MakeRandom();
            PermutationBuilder<0> permutationBuilder(mSetSize);
            permutationBuilder.FillBlock<0>(permutationBlock, permutationRandomEx);
        }

        mShuffleMode = ShuffleMode::RepeatedShufflingWithBlockMixing;
    }
}

template void SetRandomizerInternal::Randomize<SetRandomizerInternal::ShuffleDataSize::Small>(std::span<SetRandomizerInternal::PermutationBlock>);
template void SetRandomizerInternal::Randomize<SetRandomizerInternal::ShuffleDataSize::Large>(std::span<SetRandomizerInternal::PermutationBlock>);


void SetRandomizerInternal::FillWithPermutationExtended(Factoradics::setsize maxBlockIndex, std::span<SetRandomizerInternal::PermutationBlock>& permutationBlocks)
{
    using namespace Factoradics;
    mShuffleMode = ShuffleMode::PermutationExtended;

    std::vector<bits> randomBits;
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
    default: mShuffleMode = ShuffleMode::None; break;
    }
}

namespace
{
    template<bool MixedBlocks>
    inline uint32_t RepeatedShuffling(const uint32_t setSize, uint32_t index, uint32_t permutationMultiplier, std::span<const SetRandomizerInternal::PermutationBlock>& permutationBlocks)
    {
        /**
        
        OPERATIONS:
        [Q_R] combine # --> Q * # + R
        e.g.
        [9_2] combine 3 --> 9 * 3 + 2 = 29

        [[Q2_R2]_R1] combine # --> [[Q2 * # + R2]_R1]
        e.g.
        [[4_1]_2] combine 2 --> [[4 * 2 + 1]_2] = [[9]_2] = [9_2]

        # div # --> [quotient_remainder] aka [Q_R]
        e.g.
        29 div 3 --> [9_2] = (9 * 3 + 2) = (27 + 2) = 29

        [Q_R] div # --> [[quotient2_remainder2]_remainder] aka [[Q2_R2]]_R1]
        e.g.
        [9_2] div 2 --> [[4_1]_2] = ((4 * 2 + 1) * 3 + 2) = ((8 + 1) * 3 + 2) = (27 + 2) = 29

        map(#,#) --> swap innermost remainders with the matching numbers (a very very simple permutation)
        e.g. with a permutation that swaps 1 and 0
        [[4_1]_2] map(0,1) --> [[4_0]_2] = ((4 * 2 + 0) * 3 + 2) = ((8 + 0) * 3 + 2) = (24 + 2) = 26

        # shift # --> Subtracts number, rolling over when reaching 0 to the max of the set size (aka "# - # mod set size")
        e.g. (set size of 16)
        1 roll 4 --> (4 arrows) 1->0->15->14->13 --> 13
        OR
        1 roll 4 --> (1-4) mod 16 --> 13


        Final rule:
        We operate only on numbers that can be grouped under ALL divs in the operation.
        e.g.
        If we div 2 and div 3, 2 * 3 = 6. We operate on numbers that fill full blocks of 6.
        So, [0,1,2,3,4,5] = Good. [6,7,8,9,10,11] = Good. [12] = Not good.
        This can be done at each block of steps.

        FULL EXAMPLE (set size 16, permutation size 3):

0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
[0_0],[0_1],[0_2],[1_0],[1_1],[1_2],[2_0],[2_1],[2_2],[3_0],[3_1],[3_2],[4_0],[4_1],[4_2],15 :: div 3
[0_1],[0_0],[0_2],[1_1],[1_0],[1_2],[2_1],[2_0],[2_2],[3_1],[3_0],[3_2],[4_1],[4_0],[4_2],15 :: rmap(0,1)
1,0,2,4,3,5,7,6,8,10,9,11,13,12,14,15 :: combine 3

[0_1],[0_0],[1_0],[2_0],[1_1],[2_1],[3_1],[3_0],[4_0],[5_0],[4_1],[5_1],13,12,14,15 :: div 2
[[0_0]_1],[[0_0]_0],[[0_1]_0],[[0_2]_0],[[0_1]_1],[[0_2]_1],[[1_0]_1],[[1_0]_0],[[1_1]_0],[[1_2]_0],[[1_1]_1],[[1_2]_1],13,12,14,15 :: div 3
[[0_1]_1],[[0_1]_0],[[0_0]_0],[[0_2]_0],[[0_0]_1],[[0_2]_1],[[1_1]_1],[[1_1]_0],[[1_0]_0],[[1_2]_0],[[1_0]_1],[[1_2]_1],13,12,14,15 :: rmap(0,1)
[1_1],[1_0],[0_0],[2_0],[0_1],[2_1],[4_1],[4_0],[3_0],[5_0],[3_1],[5_1],13,12,14,15 :: combine 3
3,2,0,4,1,5,9,8,6,10,7,11,13,12,14,15 :: combine 2

[0_3],[0_2],[0_0],[1_0],[0_1],[1_1],[2_1],[2_0],[1_2],[2_2],[1_3],[2_3],13,12,14,15 :: div 4
[[0_0]_3],[[0_0]_2],[[0_0]_0],[[0_1]_0],[[0_0]_1],[[0_1]_1],[[0_2]_1],[[0_2]_0],[[0_1]_2],[[0_2]_2],[[0_1]_3],[[0_2]_3],13,12,14,15 :: div 3
[[0_1]_3],[[0_1]_2],[[0_1]_0],[[0_0]_0],[[0_1]_1],[[0_0]_1],[[0_2]_1],[[0_2]_0],[[0_0]_2],[[0_2]_2],[[0_0]_3],[[0_2]_3],13,12,14,15 :: rmap(0,1)
[1_3],[1_2],[1_0],[0_0],[1_1],[0_1],[2_1],[2_0],[0_2],[2_2],[0_3],[2_3],13,12,14,15 :: combine 3
7,6,4,0,5,1,9,8,2,10,3,11,13,12,14,15 :: combine 4

3,2,0,12,1,13,5,4,14,6,15,7,9,8,10,11 :: shift 4

[0_3],[0_2],[0_0],[2_2],[0_1],[2_3],[1_0],[0_4],[2_4],[1_1],15,[1_2],[1_4],[1_3],[2_0],[2_1] :: div 5
[1_3],[1_2],[1_0],[2_2],[1_1],[2_3],[0_0],[1_4],[2_4],[0_1],15,[0_2],[0_4],[0_3],[2_0],[2_1] :: qmap(0,1)
8,7,5,12,6,13,0,9,14,1,15,2,4,3,10,11 :: combine 5

4,3,1,8,2,9,12,5,10,13,11,14,0,15,6,7 :: shift 4

[1_1],[1_0],[0_1],[2_2],[0_2],[3_0],[4_0],[1_2],[3_1],[4_1],[3_2],[4_2],[0_0],15,[2_0],[2_1] :: div 3
[1_0],[1_1],[0_0],[2_2],[0_2],[3_1],[4_1],[1_2],[3_0],[4_0],[3_2],[4_2],[0_1],15,[2_1],[2_0] :: rmap(0,1)
3,4,0,8,2,10,13,5,9,12,11,14,1,15,7,6 :: combine 3

[1_1],[2_0],[0_0],[4_0],[1_0],[5_0],13,[2_1],[4_1],12,[5_1],14,[0_1],15,[3_1],[3_0] :: div 2
[[0_1]_1],[[0_2]_0],[[0_0]_0],[[1_1]_0],[[0_1]_0],[[1_2]_0],13,[[0_2]_1],[[1_1]_1],12,[[1_2]_1],14,[[0_0]_1],15,[[1_0]_1],[[1_0]_0] :: div 3
[[0_0]_1],[[0_2]_0],[[0_1]_0],[[1_0]_0],[[0_0]_0],[[1_2]_0],13,[[0_2]_1],[[1_0]_1],12,[[1_2]_1],14,[[0_1]_1],15,[[1_1]_1],[[1_1]_0] :: rmap(0,1)
[0_1],[2_0],[1_0],[3_0],[0_0],[5_0],13,[2_1],[3_1],12,[5_1],14,[1_1],15,[4_1],[4_0] :: combine 3
1,4,2,6,0,10,13,5,7,12,11,14,3,15,9,8 :: combine 2

[0_1],[1_0],[0_2],[1_2],[0_0],[2_2],13,[1_1],[1_3],12,[2_3],14,[0_3],15,[2_1],[2_0] :: div 4
[[0_0]_1],[[0_1]_0],[[0_0]_2],[[0_1]_2],[[0_0]_0],[[0_2]_2],13,[[0_1]_1],[[0_1]_3],12,[[0_2]_3],14,[[0_0]_3],15,[[0_2]_1],[[0_2]_0] :: div 3
[[0_1]_1],[[0_0]_0],[[0_1]_2],[[0_0]_2],[[0_1]_0],[[0_2]_2],13,[[0_0]_1],[[0_0]_3],12,[[0_2]_3],14,[[0_1]_3],15,[[0_2]_1],[[0_2]_0] :: rmap(0,1)
[1_1],[0_0],[1_2],[0_2],[1_0],[2_2],13,[0_1],[0_3],12,[2_3],14,[1_3],15,[2_1],[2_0] :: combine 3
5,0,6,2,4,10,13,1,3,12,11,14,7,15,9,8 :: combine 4

The above example is a showcase of a transform with a factoradic max base of 3 on a set size of 16, resulting in:
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 --> 5,0,6,2,4,10,13,1,3,12,11,14,7,15,9,8

        */


        // const uint32_t shuffles = 3 + (((uint32_t)std::log2((double)setSize)) >> 1
        // (std::bit_width(setSize) + 1) >> 1



        for (uint32_t revMultiplier = 1, combMult = permutationMultiplier;
            combMult > 0;
            revMultiplier <<= 1, combMult >>= 1)
        {
            const uint32_t blockSize = SetRandomizerInternal::cPermutationIndexesPerBlock * revMultiplier;
            if (index < (blockSize * combMult))
            {
                const std::ldiv_t baseMultDiv = std::ldiv(index, revMultiplier);
                const std::ldiv_t blockDiv = std::ldiv(baseMultDiv.quot, SetRandomizerInternal::cPermutationIndexesPerBlock);
                uint32_t usedBlock = 0;
                index = (blockDiv.quot * SetRandomizerInternal::cPermutationIndexesPerBlock) * permutationBlocks[usedBlock][blockDiv.rem];
                index = (index * revMultiplier) + baseMultDiv.rem;
            }
        }

        // index = (index + (setSize >> 1)) % setSize;

        // Shift by overshot
        // Map by quotient
        // 


        return index;
    }

    template<bool MixedBlocks>
    inline uint32_t ShuffleContinuousBatch(const uint32_t setSize, uint32_t index, uint32_t permutationMultiplier, std::span<const SetRandomizerInternal::PermutationBlock>& permutationBlocks)
    {
        const uint32_t shuffles = 3 + (((uint32_t)std::log2((double)setSize)) >> 1);
        // (std::bit_width(setSize) + 1) >> 1

        for (uint32_t i = 0; true;)
        {
            if (index < (SetRandomizerInternal::cPermutationIndexesPerBlock * permutationMultiplier))
            {
                // Shuffle specifics
                const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::cPermutationIndexesPerBlock);

                uint32_t usedBlock = 0;
                if constexpr (MixedBlocks)
                {
                    usedBlock = i % permutationBlocks.size();
                }
                index = permutationBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::cPermutationIndexesPerBlock);
            }

            for (uint32_t revMultiplier = 2, combMult = permutationMultiplier >> 1;
                combMult > 0;
                revMultiplier <<= 1, combMult >>= 1)
            {
                const uint32_t blockSize = SetRandomizerInternal::cPermutationIndexesPerBlock * revMultiplier;
                if (index < (blockSize * combMult))
                {
                    const std::ldiv_t baseMultDiv = std::ldiv(index, revMultiplier);
                    index = baseMultDiv.quot;

                    const std::ldiv_t blockDiv = std::ldiv(index, SetRandomizerInternal::cPermutationIndexesPerBlock);
                    uint32_t usedBlock = 0;
                    if constexpr (MixedBlocks)
                    {
                        usedBlock = i % permutationBlocks.size();
                    }
                    index = permutationBlocks[usedBlock][blockDiv.rem] + (blockDiv.quot * SetRandomizerInternal::cPermutationIndexesPerBlock);

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
    using namespace Factoradics;
    if (index > mSetSize)
    {
        return index;
    }

    switch (mShuffleMode)
    {
    default:
    case ShuffleMode::None:
        return mSetSize - 1;

    case ShuffleMode::CoinFlip:
        return (index ^ permutationBlocks[0][0]) & 0b1;

    case ShuffleMode::Permutation:
        return permutationBlocks[0][index];


    case ShuffleMode::PermutationExtended:
    {
        for (setsize blockIndex = 0; blockIndex < permutationBlocks.size(); ++blockIndex)
        {
            if (index < GetBlockAtRunTime(blockIndex).MaxFactorial)
            {
                return permutationBlocks[blockIndex][index - GetBlockAtRunTime(blockIndex).MinSetPosition];
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
    case ShuffleMode::RepeatedShuffling:
    {
        return RepeatedShuffling<false>(mSetSize, index, mPermutationMultiplier, permutationBlocks);
    }

    case ShuffleMode::RepeatedShufflingWithBlockMixing:
    {
        return RepeatedShuffling<true>(mSetSize, index, mPermutationMultiplier, permutationBlocks);
    }
    }
}

Factoradics::bits SetRandomizerInternal::MakeRandom() const
{
    return ((Factoradics::bits)mRandomFunc() << 32) & INT64_MAX | (Factoradics::bits)mRandomFunc();
}
