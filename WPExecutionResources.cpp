#include "WPExecutionResources.h"
#include "RNG.h"

/**
 * Dice
 */

void DieRoll::Print() const
{
	MathPrint::PrintResetCC();
	MathPrint::PrintFGColorCode(mDiceColor);
	MathPrint::PrintCharacter(MathPrint::Character::BlockRight);
	MathPrint::PrintResetCC();
	MathPrint::PrintBGColorCode(mDiceColor);
	printf("%d", mValue);
	MathPrint::PrintResetCC();
	MathPrint::PrintFGColorCode(mDiceColor);
	MathPrint::PrintCharacter(MathPrint::Character::BlockLeft);
	MathPrint::PrintResetCC();
}

int32_t DieRoll::GetSimplePredictedValue() const
{
	return mValue + (mDiceEffect == DiceEffect::PlayCard ? 1 : 0);
}

const DieRoll DieRoll::kInvalidDie;

DiceFace WPExecutionResources::RandomDieFace() const
{
	return (DiceFace)mRng.RandomIndex(+DiceFace::Count);
}

void WPExecutionResources::RollNOfEachDie(int32_t numOfEach)
{
	mRemainingDice.clear();
	for (DiceType diceType = DiceType::Heavy; diceType < DiceType::CountCombatSetupDice; ++diceType)
	{
		for (int32_t dieCount = 0; dieCount < numOfEach; ++dieCount)
		{
			mRemainingDice.push_back(MakeDieRoll(diceType, RandomDieFace()));
		}
	}
}

void WPExecutionResources::RerollDie(int32_t index)
{
	mRemainingDice[index] = MakeDieRoll(mRemainingDice[index].mDiceType, RandomDieFace());
}

void WPExecutionResources::ReplaceDie(int32_t index, DiceType diceType)
{
	mRemainingDice[index] = MakeDieRoll(diceType, RandomDieFace());
}

void WPExecutionResources::RemoveDice(std::vector<int32_t> indexes)
{
	std::vector<DieRoll> swappedInRolls;
	for (int32_t i = 0; i < mRemainingDice.size(); ++i)
	{
		if (std::find(indexes.begin(), indexes.end(), i) == indexes.end())
		{
			swappedInRolls.push_back(mRemainingDice[i]);
		}
	}
	std::swap(mRemainingDice, swappedInRolls);
}

const DieRoll& WPExecutionResources::GetDieRoll(int32_t index) const
{
	if (index < (int32_t)mRemainingDice.size())
	{
		return mRemainingDice.at(index);
	}
	return DieRoll::kInvalidDie;
}

/*static*/ DieRoll WPExecutionResources::MakeDieRoll(DiceType diceType, DiceFace face)
{
	//static const std::vector<int32_t> sSides[+DiceType::Count] =
	//{
	//	{3,3,4,4,4,9},
	//	{1,2,2,7,7,8},
	//	{0,5,5,5,6,6},
	//	{1,1,1,2,2,3}
	//};
	static const std::vector<int32_t> sSides[+DiceType::Count] =
	{
		{6,7,7,8,8,9}, // Heavy
		{1,2,3,4,5,6}, // Swing
		{3,3,4,4,5,6}, // Bonus
		{0,1,1,2,2,2}, // Card

		// RandomEffect
		{2,2,6,6,9,9}
	};
	static const std::vector<DiceEffect> sEffects[+DiceType::Count] =
	{
		MakeDiceEffectList(DiceEffect::None),		 // Heavy
		MakeDiceEffectList(DiceEffect::None),		 // Swing
		MakeDiceEffectList(DiceEffect::ResultBonus), // Bonus
		MakeDiceEffectList(DiceEffect::PlayCard),    // Card

		// RandomEffect
		{DiceEffect::PlayCard, DiceEffect::PlayCard, DiceEffect::ResultBonus, DiceEffect::ResultBonus, DiceEffect::None, DiceEffect::None}
	};
	static const MathPrint::PrintColor sDieColor[+DiceType::Count] =
	{
		MathPrint::PrintColor::DarkRed, // Heavy
		MathPrint::PrintColor::Brown,	// Swing
		MathPrint::PrintColor::Purple,	// Bonus
		MathPrint::PrintColor::Blue,	// Card

		// RandomEffect
		MathPrint::PrintColor::Orange
	};

	const int32_t missingPotential = diceType < DiceType::CountCombatSetupDice ? (sSides[+diceType][5] - sSides[+diceType][+face]) : 0;
	return DieRoll(diceType, sSides[+diceType][+face], sEffects[+diceType][+face], sDieColor[+diceType], missingPotential);
}

/*static*/ std::vector<DiceEffect> WPExecutionResources::MakeDiceEffectList(DiceEffect diceEffect)
{
	std::vector<DiceEffect> returnValue;
	for (DiceFace face = DiceFace::Worst; face < DiceFace::Count; ++face)
	{
		returnValue.push_back(diceEffect);
	}
	return returnValue;
}

std::vector<int32_t> WPExecutionResources::GetPossibleDieRollIndexes(int32_t skipIndex /*= -1*/) const
{
	std::vector<int32_t> result;
	for (int32_t i = 0; i < GetNumRemainingDice(); ++i)
	{
		if (i != skipIndex)
		{
			result.push_back(i);
		}
	}
	return result;
}

std::vector<int32_t> WPExecutionResources::GetNWorstDieRollIndexes(size_t n, int32_t skipIndex /*= -1*/) const
{
	std::vector<int32_t> result = GetPossibleDieRollIndexes(skipIndex);
	std::sort(result.begin(), result.end(), [this](int32_t indexA, int32_t indexB) -> bool
		{
			return GetDieRoll(indexA).GetSimplePredictedValue() < GetDieRoll(indexB).GetSimplePredictedValue();
		});
	result.resize(std::min(n, result.size()));
	return result;
}

std::vector<int32_t> WPExecutionResources::GetNBestDieRollIndexes(size_t n, int32_t skipIndex /*= -1*/) const
{
	std::vector<int32_t> result = GetPossibleDieRollIndexes(skipIndex);
	std::sort(result.begin(), result.end(), [this](int32_t indexA, int32_t indexB) -> bool
		{
			return GetDieRoll(indexA).GetSimplePredictedValue() > GetDieRoll(indexB).GetSimplePredictedValue();
		});
	result.resize(std::min(n, result.size()));
	return result;
}

std::vector<int32_t> WPExecutionResources::GetNHighestMissingPotentialDieRollIndexes(size_t n, int32_t skipIndex /*= -1*/) const
{
	std::vector<int32_t> result = GetPossibleDieRollIndexes(skipIndex);
	std::sort(result.begin(), result.end(), [this](int32_t indexA, int32_t indexB) -> bool
		{
			return GetDieRoll(indexA).mMissingPotential > GetDieRoll(indexB).mMissingPotential;
		});
	result.resize(std::min(n, result.size()));
	return result;
}

int32_t WPExecutionResources::GetFirstRemainingDiceIndexWithEffect(DiceEffect diceEffect, int32_t skipIndex /*= -1*/) const
{
	for (int32_t i = 0; i < GetNumRemainingDice(); ++i)
	{
		if ((i != skipIndex) && (GetDieRoll(i).mDiceEffect == diceEffect))
		{
			return i;
		}
	}
	return -1;
}

/**
 * Cards
 */

const WPCard& WPExecutionResources::GetCard(int32_t index) const
{
	if (index < (int32_t)mAllAvailableCards.size())
	{
		return mAllAvailableCards.at(index);
	}
	return WPCard::kInvalidCard;
}

void WPExecutionResources::ResetCardsToDefault()
{
	mAllAvailableCards.clear();

	mAllAvailableCards.emplace_back(1, "Reroll 2 Dice", std::vector<CardEffectAndMagnitude>{
		{ CardEffect::RerollDice, 2 }
	});
	mAllAvailableCards.emplace_back(1, "Extra Eval Next Turn", std::vector<CardEffectAndMagnitude>{
		{ CardEffect::ExtraEvaluate, 1 }
	});
	mAllAvailableCards.emplace_back(1, "+50 skill", std::vector<CardEffectAndMagnitude>{
		{ CardEffect::TempSkill, 50 }
	});
	mAllAvailableCards.emplace_back(1, "Heal 1", std::vector<CardEffectAndMagnitude>{
		{ CardEffect::Heal, 1 }
	});
}

void WPExecutionResources::AddCardVariation(int32_t variationId)
{
	switch (variationId)
	{
	case 1:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Draw a Card, +1 Temp Bonus Result", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::DrawCard, 1 },
			{ CardEffect::TempBonusResult, 1 }
		});
		break;
	}
	case 2:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Swap Opponent Dice", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::SwapOpponentDice, 1 }
		});
		break;
	}
	case 3:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Force 1 Damage", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::ForceDamage, 1 }
		});
		break;
	}
	case 4:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "+1 Permanent Damage", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::PermanentDamage, 1 }
		});
		break;
	}
	case 5:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "+2x Eval Next Turn", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::ExtraEvaluate, 2 }
		});
		break;
	}
	case 6:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Heal 3", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::Heal, 3 }
		});
		break;
	}
	case 7:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Reroll 4 Dice", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::RerollDice, 4 }
		});
		break;
	}
	case 8:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "+20 ONLY PERMANENT BONUS", std::vector<CardEffectAndMagnitude>{
		});
		break;
	}
	case 9:
	{
		mAllAvailableCards.pop_back();
		mAllAvailableCards.emplace(mAllAvailableCards.begin(), 2, "Exchange Dice for Super Dice", std::vector<CardEffectAndMagnitude>{
			{ CardEffect::ReplaceWithRandomEffectDice, 1 }
		});
		break;
	}

	default: break;
	}
}

std::vector<int32_t> WPExecutionResources::GetNHighestCardLevels(size_t n) const
{
	std::vector<int32_t> result;
	for (const WPCard& card : mAllAvailableCards)
	{
		result.push_back(card.mCardLevel);
	}
	std::sort(result.begin(), result.end(), [&result](int32_t indexA, int32_t indexB) -> bool
		{
			return result[indexA] > result[indexB];
		});
	result.resize(std::min(n, result.size()));
	return result;
}

void WPExecutionResources::PrintCardVariations(const char* const headerName) const
{
	bool bHasPrintedHeader = false;

	for (const WPCard& card : mAllAvailableCards)
	{
		if (card.mCardLevel > 1)
		{
			if (!bHasPrintedHeader)
			{
				printf("\n%s: ", headerName);
				bHasPrintedHeader = true;
			}
			else
			{
				printf(" & ");
			}

			printf(card.mCardName.c_str());
		}
	}
}

