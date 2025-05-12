#pragma once
class SetRandomizerInternal
{
public:
    SetRandomizerInternal() = delete;
    static constexpr size_t kNumCombinatoricIndexes = 20;
    static constexpr size_t kNumCombinatoricIndexesExtended = 32;
    static constexpr size_t kNumCombinatoricIndexesExtendedExtended = 43;
    using CombinatoricBlock = std::array<uint8_t, kNumCombinatoricIndexes>;

private:
    SetRandomizerInternal(uint32_t(*randomFunc)(), uint32_t setSize)
        : mRandomFunc(randomFunc)
        , mSetSize(setSize)
    {}

    void Randomize(std::span<SetRandomizerInternal::CombinatoricBlock> combinatoricBlocks);
    void FillWithPermutationExtended(size_t maxBlockIndex, std::span<SetRandomizerInternal::CombinatoricBlock>& combinatoricBlocks);
    uint32_t GetWheeledIndex(uint32_t index, std::span<const SetRandomizerInternal::CombinatoricBlock> combinatoricBlocks) const;
    int64_t MakeRandom() const;

    enum class ShuffleMode : uint8_t
    {
        kNone,
        kCoinFlip,
        kCombinatoric,
        kCombinatoricExtended,
        kCombinatoricExtendedExtended,
        kRepeatedShuffling,
        kRepeatedShufflingWithBlockMixing
    };

    uint32_t(* const mRandomFunc)();
    const uint32_t mSetSize;
    ShuffleMode mShuffleMode = ShuffleMode::kNone;
    uint32_t mCombinatoricMultiplier = 0;

    template<size_t CombinatoricBlocks> requires (CombinatoricBlocks > 0) friend class SetRandomizer;
};

template <size_t CombinatoricBlocks> requires (CombinatoricBlocks > 0)
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
    [[nodiscard]] consteval size_t GetNumBlocks() const { return CombinatoricBlocks; }

private:
    SetRandomizerInternal mInternalRandomizer;
    std::array<SetRandomizerInternal::CombinatoricBlock, CombinatoricBlocks> mCombinatoricIndexes;
};