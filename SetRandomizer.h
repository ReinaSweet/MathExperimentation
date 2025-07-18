#pragma once
template<size_t tSize>
concept WithinPermutationBlockBounds = tSize > 0 && tSize <= 28;

class SetRandomizerInternal
{
public:
    SetRandomizerInternal() = delete;
    static constexpr size_t cPermutationIndexesPerBlock = 20;
    using PermutationBlock = std::array<uint8_t, cPermutationIndexesPerBlock>;

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

    SetRandomizerInternal(uint32_t(*randomFunc)(), uint32_t setSize) noexcept
        : mRandomFunc(randomFunc)
        , mSetSize(setSize)
    {}

    void RandomizeSingleBlock(SetRandomizerInternal::PermutationBlock& permutationBlock) noexcept;
    template<ShuffleDataSize tDataSize>
    void Randomize(std::span<SetRandomizerInternal::PermutationBlock> permutationBlocks);

    void FillWithPermutationExtended(size_t maxBlockIndex, std::span<SetRandomizerInternal::PermutationBlock>& permutationBlocks);
    [[nodiscard]] uint32_t GetWheeledIndex(uint32_t index, std::span<const SetRandomizerInternal::PermutationBlock> permutationBlocks) const;
    [[nodiscard]] int64_t MakeRandom() const;

    uint32_t(* const mRandomFunc)();
    uint32_t mSetSize = 0;
    uint32_t mPermutationMultiplier = 0;
    ShuffleMode mShuffleMode = ShuffleMode::None;

    template<size_t tPermutationBlocks> requires WithinPermutationBlockBounds<tPermutationBlocks> friend class SetRandomizer;
};

template <size_t tPermutationBlocks> requires WithinPermutationBlockBounds<tPermutationBlocks>
class SetRandomizer
{
public:
    SetRandomizer(uint32_t(*randomFunc)(), uint32_t setSize) noexcept
    : mInternalRandomizer(randomFunc, setSize)
    {
        Randomize();
    }

    void Randomize()
    {
        if constexpr (tPermutationBlocks == 1)
        {
            mInternalRandomizer.RandomizeSingleBlock(mPermutationIndexes[0]);
        }
        /**
        * "6" is somewhat arbitrary here. Changing the number should have no actual impact on functionality.
        * This branch is only done for the intent of runtime performance for some use cases
        */
        else if constexpr (tPermutationBlocks < 6)
        {
            mInternalRandomizer.Randomize<SetRandomizerInternal::ShuffleDataSize::Small>(std::span(mPermutationIndexes));
        }
        else
        {
            mInternalRandomizer.Randomize<SetRandomizerInternal::ShuffleDataSize::Large>(std::span(mPermutationIndexes));
        }
    }

    [[nodiscard]] uint32_t GetWheeledIndex(uint32_t index) const
    {
        return mInternalRandomizer.GetWheeledIndex(index, std::span(mPermutationIndexes));
    }

    [[nodiscard]] uint32_t GetSetSize() const { return mInternalRandomizer.mSetSize; }
    [[nodiscard]] consteval size_t GetNumBlocks() const { return tPermutationBlocks; }

private:
    SetRandomizerInternal mInternalRandomizer;
    alignas(16) std::array<SetRandomizerInternal::PermutationBlock, tPermutationBlocks> mPermutationIndexes;
};