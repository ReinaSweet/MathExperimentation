#pragma once
#include "MathCommon.h"

enum class CardEffect : uint8_t
{
	PermanentDamage,
	PermanentHold,
	PermanentFlee,
	ForceDamage,
	TempSkill,
	TempBonusResult,
	RerollDice,
	ReplaceWithRandomEffectDice,
	ExtraEvaluate,
	Heal,
	SwapOpponentDice,
	StealVP,
	DrawCard
};
ENUM_OPS(CardEffect);

struct CardEffectAndMagnitude
{
	CardEffect mCardEffect;
	int32_t mMagnitude;
};

class WPCard
{
public:
	WPCard(int32_t cardLevel, const char* const cardName, std::vector<CardEffectAndMagnitude> cardEffectsAndMagnitude);

	int32_t mCardLevel = 1;
	std::string mCardName;

	bool GetCardEffect(CardEffect cardEffect, int32_t& outMagnitude) const;
	static const WPCard kInvalidCard;

private:
	std::vector<CardEffectAndMagnitude> mCardEffectsAndMagnitude;
};

