#pragma once
template<size_t tSize>
concept WithinPermutationBlockBounds = tSize > 0 && tSize <= 28;

class SetRandomizerInternal
{
public:
    SetRandomizerInternal() = delete;
    static constexpr size_t kPermutationIndexesPerBlock = 20;
    using PermutationBlock = std::array<uint8_t, kPermutationIndexesPerBlock>;

private:
    SetRandomizerInternal(uint32_t(*randomFunc)(), uint32_t setSize)
        : mRandomFunc(randomFunc)
        , mSetSize(setSize)
    {}

    void Randomize(std::span<SetRandomizerInternal::PermutationBlock> tPermutationBlocks);
    void FillWithPermutationExtended(size_t maxBlockIndex, std::span<SetRandomizerInternal::PermutationBlock>& tPermutationBlocks);
    uint32_t GetWheeledIndex(uint32_t index, std::span<const SetRandomizerInternal::PermutationBlock> tPermutationBlocks) const;
    int64_t MakeRandom() const;

    enum class ShuffleMode : uint8_t
    {
        kNone,
        kCoinFlip,
        kPermutation,
        kPermutationExtended,
        kRepeatedShuffling,
        kRepeatedShufflingWithBlockMixing
    };

    uint32_t(* const mRandomFunc)();
    const uint32_t mSetSize;
    ShuffleMode mShuffleMode = ShuffleMode::kNone;
    uint32_t mCombinatoricMultiplier = 0;

    template<size_t tPermutationBlocks> requires WithinPermutationBlockBounds<tPermutationBlocks> friend class SetRandomizer;
};

template <size_t tPermutationBlocks> requires WithinPermutationBlockBounds<tPermutationBlocks>
class SetRandomizer
{
public:
    SetRandomizer(uint32_t(*randomFunc)(), uint32_t setSize)
    : mInternalRandomizer(randomFunc, setSize)
    {
        Randomize();
    }

    void Randomize()
    {
        mInternalRandomizer.Randomize(std::span(mCombinatoricIndexes));
    }

    [[nodiscard]] uint32_t GetWheeledIndex(uint32_t index) const
    {
        return mInternalRandomizer.GetWheeledIndex(index, std::span(mCombinatoricIndexes));
    }

    [[nodiscard]] uint32_t GetSetSize() const { return mInternalRandomizer.mSetSize; }
    [[nodiscard]] consteval size_t GetNumBlocks() const { return tPermutationBlocks; }

private:
    SetRandomizerInternal mInternalRandomizer;
    alignas(16) std::array<SetRandomizerInternal::PermutationBlock, tPermutationBlocks> mCombinatoricIndexes;
};