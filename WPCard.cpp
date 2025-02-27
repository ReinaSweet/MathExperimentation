#include "WPCard.h"

const WPCard WPCard::kInvalidCard(0, "Invalid", {});

WPCard::WPCard(int32_t cardLevel, const char* const cardName, std::vector<CardEffectAndMagnitude> cardEffectsAndMagnitude)
: mCardLevel(cardLevel)
, mCardName(cardName)
, mCardEffectsAndMagnitude(cardEffectsAndMagnitude)
{

}

bool WPCard::GetCardEffect(CardEffect cardEffect, int32_t& outMagnitude) const
{
	for (const CardEffectAndMagnitude& pair : mCardEffectsAndMagnitude)
	{
		if (pair.mCardEffect == cardEffect)
		{
			outMagnitude = pair.mMagnitude;
			return true;
		}
	}
	return false;
}
