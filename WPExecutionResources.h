#pragma once

#include "MathCommon.h"
#include "MathPrint.h"
#include "WPCard.h"

class RNG;
class Stats;

enum class DiceType : uint8_t
{
	Heavy,
	Swing,
	Bonus,
	Card,

	CountCombatSetupDice,

	RandomEffect = CountCombatSetupDice,

	Count
};
ENUM_OPS(DiceType);

enum class DiceEffect : uint8_t
{
	None,
	ResultBonus,
	PlayCard
};
ENUM_OPS(DiceEffect);

enum class DiceFace : uint8_t
{
	Worst = 0,
	Bad = 1,
	Meh = 2,
	Decent = 3,
	Good = 4,
	Best = 5,

	Count
};
ENUM_OPS(DiceFace);

struct DieRoll
{
	DiceType mDiceType;
	int32_t mValue;
	DiceEffect mDiceEffect;
	MathPrint::PrintColor mDiceColor;
	int32_t mMissingPotential;

	DieRoll(DiceType diceType, int32_t value, DiceEffect diceEffect, MathPrint::PrintColor diceColor, int32_t missingPotential)
	: mDiceType(diceType)
	, mValue(value)
	, mDiceEffect(diceEffect)
	, mDiceColor(diceColor)
	, mMissingPotential(missingPotential)
	{}

	DieRoll()
	: mDiceType(DiceType::Swing)
	, mValue(-10)
	, mDiceEffect(DiceEffect::None)
	, mDiceColor(MathPrint::PrintColor::Red)
	, mMissingPotential(0)
	{}

	void Print() const;

	// Assumes level 1 cards
	// TODO: Take in a worker to make a more complex prediction
	int32_t GetSimplePredictedValue() const;

	static const DieRoll kInvalidDie;
};

class WPExecutionResources
{
public:
	WPExecutionResources(const RNG& rng)
	: mRng(rng)
	{}

	/**
	 * Dice
	 */
	DiceFace RandomDieFace() const;
	void RollNOfEachDie(int32_t numOfEach);

	void RerollDie(int32_t index);
	void ReplaceDie(int32_t index, DiceType diceType);
	void RemoveDice(std::vector<int32_t> indexes);

	const std::vector<DieRoll>& GetRemainingDice() const { return mRemainingDice; }
	int32_t GetNumRemainingDice() const { return (int32_t)mRemainingDice.size(); }
	int32_t GetNumRemainingCombatRounds() const { return (int32_t)mRemainingDice.size() / 2; }
	const DieRoll& GetDieRoll(int32_t index) const;

	static DieRoll MakeDieRoll(DiceType diceType, DiceFace face);
	static std::vector<DiceEffect> MakeDiceEffectList(DiceEffect diceEffect);

	std::vector<int32_t> GetPossibleDieRollIndexes(int32_t skipIndex = -1) const;
	std::vector<int32_t> GetNWorstDieRollIndexes(size_t n, int32_t skipIndex = -1) const;
	std::vector<int32_t> GetNBestDieRollIndexes(size_t n, int32_t skipIndex = -1) const;
	std::vector<int32_t> GetNHighestMissingPotentialDieRollIndexes(size_t n, int32_t skipIndex = -1) const;
	int32_t GetFirstRemainingDiceIndexWithEffect(DiceEffect diceEffect, int32_t skipIndex = -1) const;
	/**
	 * Cards
	 */
	void ResetCardsToDefault();
	void AddCardVariation(int32_t variationId);

	const int32_t GetNumAllCards() const { return (int32_t)mAllAvailableCards.size(); }
	const WPCard& GetCard(int32_t index) const;

	std::vector<int32_t> GetNHighestCardLevels(size_t n) const;

	void PrintCardVariations(const char* const headerName) const;
private:
	const RNG& mRng;

	std::vector<DieRoll> mRemainingDice;
	std::vector<WPCard> mAllAvailableCards;
};

