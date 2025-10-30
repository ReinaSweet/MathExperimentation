#pragma once

namespace LazyElementShuffler
{
    using Size = size_t;
    using UInt = uint64_t;
    using Byte = uint8_t;

    template<Size tSize>
    concept WithinPermutationBlockBounds = tSize > 0 && tSize <= 28;

    struct Bits
    {
        Bits() = delete;

        constexpr Bits(UInt value)
            : mValue(value)
        {}

        constexpr Bits(const Bits& value)
            : mValue(value.mValue)
        {}

        static constexpr Bits Max()
        {
            return std::numeric_limits<UInt>::max();
        }

        constexpr Bits& operator=(UInt value) { mValue = value; return *this; }
        constexpr Bits& operator--() { mValue--; return *this; }
        constexpr Bits& operator*=(const Bits& value) { mValue = mValue * value.mValue; return *this; }

        constexpr Bits DivAndSetToRemainder(const Bits& value)
        {
            const UInt quot = (mValue / value.mValue);
            const UInt rem = (mValue % value.mValue);

            mValue = rem;
            return Bits(quot);
        }

        inline constexpr Bits operator+(const UInt& value) const { return Bits(mValue + value); }
        inline constexpr Bits operator/(const Bits& value) const { return Bits(mValue / value.mValue); }

        inline constexpr bool operator>(const Bits& other) const { return mValue > other.mValue; }
        inline constexpr bool operator==(const Bits& other) const { return mValue == other.mValue; }

        constexpr UInt FinalBitUInt() const { return (mValue & 0b1); }
        constexpr Byte FinalBitByte() const { return (Byte)(mValue & 0b1); }

        constexpr Size AsSize() const { return (Size)mValue; }
        constexpr UInt AsUInt() const { return (UInt)mValue; }
        constexpr Byte AsByte() const { return (Byte)mValue; }

        UInt mValue;
    };




    constexpr Size cNumBlock = 28;

    /**
    * Factorial Tools
    */
    template<Bits tMax>
    consteval Bits ConstMax(Bits value)
    {
        return (value > tMax) ? tMax : value;
    }

    consteval Bits ConstFactorialRange(Bits min, Bits max)
    {
        Bits value(1);
        for (; max > min; --max)
        {
            value *= max;
        }
        return value;
    }

    template<Bits tMinFactorial, Bits tMaxFactorial>
    constexpr Bits FactorialRange(Size m)
    {
        const SetSize offsetFromMinFactorial = m - tMinFactorial.AsSetSize();
        switch (offsetFromMinFactorial)
        {
            // Uncomment this to see the constexpr fail for running out of bit space
            // case (21): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 21));
        case (20): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 20));
        case (19): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 19));
        case (18): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 18));
        case (17): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 17));
        case (16): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 16));
        case (15): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 15));
        case (14): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 14));
        case (13): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 13));
        case (12): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 12));
        case (11): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 11));
        case (10): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 10));
        case (9): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 9));
        case (8): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 8));
        case (7): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 7));
        case (6): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 6));
        case (5): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 5));
        case (4): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 4));
        case (3): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 3));
        case (2): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 2));
        case (1): return ConstFactorialRange(tMinFactorial, ConstMax<tMaxFactorial>(tMinFactorial + 1));
        case (0): return ConstFactorialRange(tMinFactorial, tMinFactorial);
        default: return 1;
        }
    }

    /**
    * Blocks of Factoradic Numbers 
    */

    struct Block
    {
        Size MinFactorial = 1;
        Size MaxFactorial;
        Size MinSetPosition = 0;
        Bits MaxNumOfMaxFactoradic = 0;
        bool CoinflipFinalTwo = false;

        constexpr Block(Size entries)
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

            Size baseFactorial = 0;
            for (Block& block : blocks)
            {
                const Size nextBase = baseFactorial + block.MaxFactorial;
                block.MinFactorial += baseFactorial;
                block.MaxFactorial += baseFactorial;
                block.MinSetPosition += baseFactorial;

                const Bits unbiasedMaxBase = ConstFactorialRange(block.MinFactorial, block.MaxFactorial);
                block.MaxNumOfMaxFactoradic = Bits::Max() / unbiasedMaxBase; // Truncation intentional

                baseFactorial = nextBase;
            }
            blocks[0].CoinflipFinalTwo = true;

            return blocks;
        }();

    consteval Block GetBlock(Size index)
    {
        return cBlocks[index];
    }

    constexpr const Block& GetBlockAtRunTime(Size index)
    {
        return cBlocks[index];
    }

    template<Size tBitSourceSize, Size tSetSize>
    constexpr void FillPermutationFromFixedSet(const std::array<UInt, tBitSourceSize>& bitSource, std::array<Size, 1>& outPermutation)
    {
        
    }

    template<Size tBitSourceSize>
    constexpr void FillPermutation(Size setSize, const std::array<UInt, tBitSourceSize>& bitSource, std::array<Size, 1>& outPermutation)
    {

    }

    class ArbitraryShuffler
    {
    public:
        constexpr ArbitraryShuffler() {}


    private:
        enum class ShuffleDataSize : uint8_t
        {
            Single,
            Small,
            Large
        };

        enum class ShuffleMode : uint8_t
        {
            None,
            CoinFlip,
            Permutation,
            PermutationExtended,
            RepeatedShuffling,
            RepeatedShufflingWithBlockMixing
        };


        Size mSetSize = 0;
        UInt mPermutationMultiplier = 0;
        ShuffleMode mShuffleMode = ShuffleMode::None;
    };

    template<Size tBitSourceSize, Size tSetSize>
    class FixedSetStackShuffler
    {
    public:
        constexpr FixedSetStackShuffler(const std::array<UInt, tBitSourceSize>& bitSource)
        {
            FillPermutationFromFixedSet<tBitSourceSize, tSetSize>(bitSource, mPermutation);
        }

        constexpr UInt Get(UInt index) const
        {
            return index;
        }

    private:
        std::array<Size, 1> mPermutation;
    };

    template<Size tBitSourceSize>
    class StackShuffler
    {
    public:
        constexpr StackShuffler(Size setSize, const std::array<UInt, tBitSourceSize>& bitSource)
        {
            FillPermutation<tBitSourceSize>(setSize, bitSource, mPermutation);
        }

        constexpr UInt Get(UInt index) const
        {
            return index;
        }

    private:
        std::array<Size, 1> mPermutation;
    };












    class SetRandomizerInternal
    {
    public:
        SetRandomizerInternal() = delete;
        static constexpr Size cPermutationIndexesPerBlock = 20;
        using PermutationBlock = std::array<Byte, cPermutationIndexesPerBlock>;

    private:
        enum class ShuffleDataSize : uint8_t
        {
            Single,
            Small,
            Large
        };

        enum class ShuffleMode : uint8_t
        {
            None,
            CoinFlip,
            Permutation,
            PermutationExtended,
            RepeatedShuffling,
            RepeatedShufflingWithBlockMixing
        };

        SetRandomizerInternal(Size setSize) noexcept
            : mSetSize(setSize)
        {}

        void RandomizeSingleBlock(SetRandomizerInternal::PermutationBlock& permutationBlock) noexcept;
        template<ShuffleDataSize tDataSize>
        void Randomize(std::span<SetRandomizerInternal::PermutationBlock> permutationBlocks);

        void FillWithPermutationExtended(Size maxBlockIndex, std::span<SetRandomizerInternal::PermutationBlock>& permutationBlocks);
        [[nodiscard]] UInt GetWheeledIndex(UInt index, std::span<const SetRandomizerInternal::PermutationBlock> permutationBlocks) const;
        [[nodiscard]] Bits MakeRandom() const;

        Size mSetSize = 0;
        UInt mPermutationMultiplier = 0;
        ShuffleMode mShuffleMode = ShuffleMode::None;

        template<size_t tPermutationBlocks> requires WithinPermutationBlockBounds<tPermutationBlocks> friend class SetRandomizer;
    };
}