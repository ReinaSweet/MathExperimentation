#include "WPWorker.h"
#include "WPScenario.h"

#include "RNG.h"
#include "Stats.h"

WPWorker::WPWorker(const RNG& rng, WPExecutionResources& executionResources, Stats* stats)
: mRng(rng)
, mExecutionResources(executionResources)
, mStats(stats)
{
}

void WPWorker::SetWorkerType(WorkerType workerType)
{
	mWorkerType = workerType;
}

void WPWorker::SetArbitraryCardVariation(int32_t arbitraryCardVariation)
{
	mArbitraryCardVariation = arbitraryCardVariation;
	mExecutionResources.ResetCardsToDefault();
	mExecutionResources.AddCardVariation(mArbitraryCardVariation);
}

void WPWorker::SetupStatsFromWorkerType()
{
	switch (mWorkerType)
	{
	case WorkerType::Basic:
	{
		mSkill = 0;
		mDamage = 1;
		mHealth_Max = 4;
		mHold = 3;
		mFlee = 1;
		mStartingHandSize = 1;
		break;
	}

	case WorkerType::Trained:
	{
		mSkill = 10;
		mDamage = 2;
		mHealth_Max = 6;
		mHold = 4;
		mFlee = 1;
		mStartingHandSize = 1;
		break;
	}

	case WorkerType::Warrior:
	{
		mSkill = 20;
		mDamage = 3;
		mHealth_Max = 8;
		mHold = 6;
		mFlee = 1;
		mStartingHandSize = 1;
		break;
	}

	case WorkerType::Rogue:
	{
		mSkill = 30;
		mDamage = 2;
		mHealth_Max = 6;
		mHold = 4;
		mFlee = 2;
		mStartingHandSize = 2;
		break;
	}

	default: break;
	}
}

void WPWorker::PrepForCombat()
{
	SetupStatsFromWorkerType();
	mHealth_Current = mHealth_Max;
	mFlee_Current = 0;

	mCurrentRound = 0;
	MoveToNextRound();
	ClearThisRoundStats();

	mCurrentDeckIndices.clear();
	for (int32_t i = 0; i < mExecutionResources.GetNumAllCards(); ++i)
	{
		mCurrentDeckIndices.push_back(i);
	}
	DrawCards(mStartingHandSize);

	mExecutionResources.RollNOfEachDie(2);
}

void WPWorker::ClearThisRoundStats()
{
	mThisRound_CardsPlayedAsIs.clear();
	mThisRound_Skill = 0;
	mThisRound_BonusResult = 0;
	mThisRound_ExtraEvaluate = 0;
	mThisRound_SwapDice = false;
}

void WPWorker::MoveToNextRound()
{
	++mCurrentRound;
	mThisRound_ExtraEvaluate = mNextRound_ExtraEvaluate;
	mNextRound_ExtraEvaluate = 0;
}

void WPWorker::SetOpponent(WPWorker& opponent)
{
	mOpponent = &opponent;
}

void WPWorker::SetDiceStrategy(std::vector<DicePlayStrategy> dicePlayStrategy)
{
	mDicePlayStrategy = dicePlayStrategy;
}

void WPWorker::SetCardStrategy(std::vector<CardPlayStrategy> cardPlayStrategy)
{
	mCardPlayStrategy = cardPlayStrategy;
}

void WPWorker::SetStrategyAsDefault()
{
	mDicePlayStrategy = {
		DicePlayStrategy::IfExtraEvalThrowBestTens,
		DicePlayStrategy::IfMaxHealthThrowWorst,
		DicePlayStrategy::TensTopHalfRandom,
		DicePlayStrategy::OnesBottomHalfRandom
	};

	mCardPlayStrategy.clear();
	for (CardPlayStrategy cardPlayStrategy = CardPlayStrategy::First; cardPlayStrategy < CardPlayStrategy::Count; ++cardPlayStrategy)
	{
		mCardPlayStrategy.push_back(cardPlayStrategy);
	}
}

void WPWorker::DetermineCommand()
{
	if (mForcedCommand != PlayCommand::UseDeterminedCommand)
	{
		mCurrentCommand = mForcedCommand;
		return;
	}

	mCurrentCommand = PlayCommand::Attack;

	int32_t ourTotalValue = 0;
	int32_t maxFleeWeCanGenerate = mFlee_Current;
	for (const DieRoll& roll : mExecutionResources.GetRemainingDice())
	{
		ourTotalValue += roll.mValue;
		maxFleeWeCanGenerate += (roll.mDiceEffect == DiceEffect::ResultBonus);
	}

	const int32_t opponentBestPossibleValue = mOpponent->GetCurrentBestPossibleTotal();
	const int32_t delta = opponentBestPossibleValue - ourTotalValue;

	const int32_t remainingRounds = (int32_t)mExecutionResources.GetNumRemainingCombatRounds();
	maxFleeWeCanGenerate += (mFlee * remainingRounds);
	const int32_t roundsWeThinkWeWillLose = std::min(remainingRounds, delta / 4);
	const int32_t opponentOkayPlayDamage = roundsWeThinkWeWillLose * mOpponent->mDamage;

	if ((opponentOkayPlayDamage >= mHealth_Current) && (maxFleeWeCanGenerate >= mOpponent->mHold))
	{
		mCurrentCommand = PlayCommand::Flee;
	}
}

WPWorker::FullRollResult WPWorker::PullDice()
{
	FullRollResult rollResult;
	if (mExecutionResources.GetNumRemainingCombatRounds() < 1)
	{
		return rollResult;
	}

	int32_t selectedTens = -1;
	int32_t selectedOnes = -1;
	for (DicePlayStrategy strategy : mDicePlayStrategy)
	{
		EvaluateDiceStrategy(strategy, selectedTens, selectedOnes);
		if ((selectedTens > -1) && (selectedOnes > -1))
		{
			break;
		}
	}

	/**
	 * Pure Random Fallback Strategy
	 */
	if (selectedTens == -1)
	{
		selectedTens = GetRandomIndexFromList(mExecutionResources.GetPossibleDieRollIndexes(selectedOnes));
	}

	if (selectedOnes == -1)
	{
		selectedOnes = GetRandomIndexFromList(mExecutionResources.GetPossibleDieRollIndexes(selectedTens));
	}

	/**
	 * Always put highest number first
	 */
	if (mExecutionResources.GetDieRoll(selectedOnes).mValue > mExecutionResources.GetDieRoll(selectedTens).mValue)
	{
		const int32_t swappedSelectedTens = selectedTens;
		selectedTens = selectedOnes;
		selectedOnes = swappedSelectedTens;
	}

	/**
	 * Commit
	 */
	rollResult.mTens = mExecutionResources.GetDieRoll(selectedTens);
	rollResult.mOnes = mExecutionResources.GetDieRoll(selectedOnes);

	mExecutionResources.RemoveDice({selectedOnes, selectedTens});

	/**
	 * Return
	 */
	rollResult.mTotalValue = rollResult.GetDieRollOnlyValue();
	if (mStats)
	{
		mStats->AddToIntDistribution(+WPStatsIds::RollValues, rollResult.mTotalValue);
	}
	mTotalRolls++;
	mTotalRollValues += rollResult.mTotalValue;

	return rollResult;
}

void WPWorker::AddSkillBonuses(FullRollResult& inOutRollResult)
{
	inOutRollResult.mTotalValue += mSkill + mThisRound_Skill;
}
void WPWorker::PlayCards(int32_t numCards, FullRollResult& ourCurrentRoll, FullRollResult& opponentCurrentRoll)
{
	if ((numCards == 0) || (mCurrentHandIndices.size() == 0))
	{
		return;
	}

	const int32_t oldThisRoundSkill = mThisRound_Skill;
	int32_t permanentSkillIncrease = 0;

	for (int32_t i = 0; i < mCardPlayStrategy.size(); ++i)
	{
		const CardPlayStrategy cardPlayStrategy = mCardPlayStrategy[i];
		std::vector<int32_t> cardsToPlayAsIs;
		std::vector<int32_t> cardsToPlayAsSkillBonus;
		EvaluateCardStrategy(cardPlayStrategy, numCards, ourCurrentRoll, opponentCurrentRoll, cardsToPlayAsIs, cardsToPlayAsSkillBonus);

		int32_t cardsPlayed = 0;

		for (int32_t index : cardsToPlayAsIs)
		{
			ExecuteSingleCard(mExecutionResources.GetCard(index), ourCurrentRoll, opponentCurrentRoll);
			mCurrentHandIndices.erase(std::find(mCurrentHandIndices.begin(), mCurrentHandIndices.end(), index));
			cardsPlayed++;

			mThisRound_CardsPlayedAsIs.push_back(index);
		}

		for (int32_t index : cardsToPlayAsSkillBonus)
		{
			permanentSkillIncrease += mExecutionResources.GetCard(index).mCardLevel * 10;
			mCurrentHandIndices.erase(std::find(mCurrentHandIndices.begin(), mCurrentHandIndices.end(), index));
			cardsPlayed++;
		}

		if (cardsPlayed > 0)
		{
			numCards -= cardsPlayed;
			DrawCards(cardsPlayed);

			// Restart back at the first strategy if we played a card
			i = -1;
		}

		if ((numCards == 0) || (mCurrentHandIndices.size() == 0))
		{
			break;
		}

		++i;
	}

	/**
	 * Fallback Strategy if we still have cards to play
	 * Just play random cards for permanent bonuses
	 */
	while ((numCards > 0) && (mCurrentHandIndices.size() > 0))
	{
		const int32_t randomIndex = mCurrentHandIndices.at(mRng.RandomIndex(mCurrentHandIndices.size()));
		permanentSkillIncrease += mExecutionResources.GetCard(randomIndex).mCardLevel * 10;
		mCurrentHandIndices.erase(std::find(mCurrentHandIndices.begin(), mCurrentHandIndices.end(), randomIndex));
		--numCards;
		DrawCards(1);
	}

	/**
	 * Commit
	 */
	mSkill += permanentSkillIncrease;
	ourCurrentRoll.mTotalValue += (mThisRound_Skill - oldThisRoundSkill) + permanentSkillIncrease;
}

void WPWorker::DrawCards(int32_t numCards)
{
	while ((numCards > 0) && (mCurrentDeckIndices.size() > 0))
	{
		const size_t drawnIndex = mRng.RandomIndex(mCurrentDeckIndices.size());
		mCurrentHandIndices.push_back(mCurrentDeckIndices.at(drawnIndex));
		mCurrentDeckIndices.erase(mCurrentDeckIndices.begin() + drawnIndex);
		--numCards;
	}
}

void WPWorker::DealStatBasedDamage(const FullRollResult& ourRollResult)
{
	DealDamage(mDamage + ourRollResult.CountEffects(DiceEffect::ResultBonus) + mThisRound_BonusResult);
}

void WPWorker::DealDamage(int32_t amount)
{
	if (mOpponent)
	{
		mOpponent->mHealth_Current -= amount;
	}
}

void WPWorker::BuildFlee(int32_t amount)
{
	mFlee_Current += amount;
}

void WPWorker::BuildStatBasedFlee(const FullRollResult& ourRollResult, bool wonValueCompare)
{
	mFlee_Current += mFlee + (ourRollResult.CountEffects(DiceEffect::ResultBonus) * (int32_t)wonValueCompare);
}

void WPWorker::IncreaseSkill(int32_t amount)
{
	mSkill += amount;
}

int32_t WPWorker::GetCurrentWorstPossibleTotal() const
{
	int32_t total = 0;
	for (const DieRoll& dieRoll : mExecutionResources.GetRemainingDice())
	{
		total += WPExecutionResources::MakeDieRoll(dieRoll.mDiceType, DiceFace::Worst).mValue;
	}
	return total;
}

int32_t WPWorker::GetCurrentBestPossibleTotal() const
{
	int32_t total = 0;
	for (const DieRoll& dieRoll : mExecutionResources.GetRemainingDice())
	{
		total += WPExecutionResources::MakeDieRoll(dieRoll.mDiceType, DiceFace::Best).mValue;
	}
	return total;
}

void WPWorker::PrintHealth() const
{
	MathPrint::PrintResetCC();
	if (!IsAlive())
	{
		MathPrint::PrintBGColorCode(MathPrint::PrintColor::DarkRed);
		printf(" 0");
	}
	else
	{
		if (mOpponent)
		{
			if (mHealth_Current <= mOpponent->mDamage)
			{
				MathPrint::PrintFGColorCode(MathPrint::PrintColor::DarkRed);
			}
			else if (mHealth_Current <= (mOpponent->mDamage * 2))
			{
				MathPrint::PrintFGColorCode(MathPrint::PrintColor::DarkYellow);
			}
		}
		printf("%2d", mHealth_Current);
	}

	MathPrint::PrintResetCC();
	printf("/%2d", mHealth_Max);
}

void WPWorker::PrintSkill() const
{
	MathPrint::PrintResetCC();
	printf("+%2d", mSkill);
}

void WPWorker::PrintCurrentRound() const
{
	MathPrint::PrintResetCC();
	printf("%d: ", mCurrentRound);
}

void WPWorker::PrintCardsPlayedThisRound() const
{
	if (mThisRound_CardsPlayedAsIs.size() > 0)
	{
		for (int32_t index : mThisRound_CardsPlayedAsIs)
		{
			printf("%s, ", mExecutionResources.GetCard(index).mCardName.c_str());
		}
	}
}

bool WPWorker::IsAlive() const
{
	return mHealth_Current > 0;
}

bool WPWorker::FleeMatchesOpposingHold() const
{
	return mFlee_Current >= mOpponent->mHold;
}

float WPWorker::GetTotalAverageRollValue() const
{
	const double TotalAverage = (double)mTotalRollValues / (double)mTotalRolls;
	return (float)TotalAverage;
}

int32_t WPWorker::GetTotalEvaluatesThisRound() const
{
	return 1 + mThisRound_ExtraEvaluate + (mOpponent ? mOpponent->mThisRound_ExtraEvaluate : 0);
}

bool WPWorker::HasPlayedCardForTheirEffectsThisRound() const
{
	return mThisRound_CardsPlayedAsIs.size() > 0;
}

std::vector<DiceEffect> WPWorker::MakeDiceEffectList(DiceEffect diceEffect)
{
	std::vector<DiceEffect> returnValue;
	for (DiceFace face = DiceFace::Worst; face < DiceFace::Count; ++face)
	{
		returnValue.push_back(diceEffect);
	}
	return returnValue;
}

const char* WPWorker::GetWorkerTypeName(WorkerType workerType)
{
	static const char* const sNames[+WorkerType::Count] =
	{
		"Basic",
		"Trained",
		"Warrior",
		"Rogue"
	};
	return sNames[+workerType];
}

void WPWorker::ExecuteSingleCard(const WPCard& card, FullRollResult& ourCurrentRoll, FullRollResult& opponentCurrentRoll)
{
	int32_t magnitude;

	if (card.GetCardEffect(CardEffect::PermanentDamage, magnitude))
	{
		mDamage += magnitude;
	}

	if (card.GetCardEffect(CardEffect::PermanentHold, magnitude))
	{
		mHold += magnitude;
	}

	if (card.GetCardEffect(CardEffect::PermanentFlee, magnitude))
	{
		mFlee += magnitude;
	}

	if (card.GetCardEffect(CardEffect::ForceDamage, magnitude))
	{
		if (mOpponent)
		{
			DealDamage(magnitude);
		}
	}

	if (card.GetCardEffect(CardEffect::TempSkill, magnitude))
	{
		mThisRound_Skill += magnitude;
	}

	if (card.GetCardEffect(CardEffect::TempBonusResult, magnitude))
	{
		mThisRound_BonusResult += magnitude;
	}

	if (card.GetCardEffect(CardEffect::RerollDice, magnitude))
	{
		// todo: Deciding which dice to reroll should be more complex than this
		std::vector<int32_t> missingPotentialIndices = mExecutionResources.GetNHighestMissingPotentialDieRollIndexes(magnitude);

		for (int32_t index : missingPotentialIndices)
		{
			// todo: This should probably allow even fewer through
			// but, the worker might be rerolling out of desperation (see: CardPlayStrategy::RerollInDesperation )
			// Likely need some sort of "this turn, card intents" that the strategy function can set to feed into this
			if (mExecutionResources.GetDieRoll(index).mMissingPotential > 0)
			{
				mExecutionResources.RerollDie(index);
			}
		}
	}

	if (card.GetCardEffect(CardEffect::ReplaceWithRandomEffectDice, magnitude))
	{
		std::vector<int32_t> reevaluatedWorstDieRollIndexes;
		std::vector<int32_t> worstDieRollIndexes = mExecutionResources.GetNWorstDieRollIndexes(2 + magnitude);

		for (int32_t index : worstDieRollIndexes)
		{
			const DieRoll& dieRoll = mExecutionResources.GetDieRoll(index);
			if ((dieRoll.mMissingPotential > 0) && (dieRoll.GetSimplePredictedValue() < 6))
			{
				reevaluatedWorstDieRollIndexes.push_back(index);
			}
		}

		for (int32_t index : reevaluatedWorstDieRollIndexes)
		{
			mExecutionResources.ReplaceDie(index, DiceType::RandomEffect);
		}
	}

	if (card.GetCardEffect(CardEffect::ExtraEvaluate, magnitude))
	{
		mNextRound_ExtraEvaluate += magnitude;
	}

	if (card.GetCardEffect(CardEffect::Heal, magnitude))
	{
		mHealth_Current += magnitude;
		if (mHealth_Current > mHealth_Max)
		{
			mHealth_Current = mHealth_Max;
		}
	}

	if (card.GetCardEffect(CardEffect::SwapOpponentDice, magnitude))
	{
		if (opponentCurrentRoll.mOnes.mValue < opponentCurrentRoll.mTens.mValue)
		{
			opponentCurrentRoll.SwapDice();
		}
	}

	if (card.GetCardEffect(CardEffect::StealVP, magnitude))
	{
		const DieRoll oldOnes = opponentCurrentRoll.mOnes;

		if (mOpponent)
		{
			mOpponent->mVP -= magnitude;
			mVP += magnitude;
		}
	}

	if (card.GetCardEffect(CardEffect::DrawCard, magnitude))
	{
		DrawCards(magnitude);
	}
}

void WPWorker::EvaluateDiceStrategy(DicePlayStrategy strategy, int32_t& inOutTensDigit, int32_t& inOutOnesDigit) const
{
	switch (strategy)
	{
	case DicePlayStrategy::IfExtraEvalThrowBest:
	{
		if (GetTotalEvaluatesThisRound() > 1)
		{
			if (inOutOnesDigit == -1) { inOutOnesDigit = mExecutionResources.GetNBestDieRollIndexes(1, inOutTensDigit)[0]; }
			if (inOutTensDigit == -1) { inOutTensDigit = mExecutionResources.GetNBestDieRollIndexes(1, inOutOnesDigit)[0]; }
		}
		break;
	}
	case DicePlayStrategy::IfExtraEvalThrowBestTens:
	{
		if (GetTotalEvaluatesThisRound() > 1)
		{
			if (inOutTensDigit == -1) { inOutTensDigit = mExecutionResources.GetNBestDieRollIndexes(1, inOutOnesDigit)[0]; }
		}
		break;
	}
	case DicePlayStrategy::IfMaxHealthThrowWorst:
	{
		if (mHealth_Current == mHealth_Max)
		{
			if (inOutOnesDigit == -1) { inOutOnesDigit = mExecutionResources.GetNWorstDieRollIndexes(1, inOutTensDigit)[0]; }
			if (inOutTensDigit == -1) { inOutTensDigit = mExecutionResources.GetNWorstDieRollIndexes(1, inOutOnesDigit)[0]; }
		}
		break;
	}
	case DicePlayStrategy::PlayCards:
	{
		if (inOutOnesDigit == -1) { inOutOnesDigit = mExecutionResources.GetFirstRemainingDiceIndexWithEffect(DiceEffect::PlayCard, inOutTensDigit); }
		if (inOutTensDigit == -1) { inOutTensDigit = mExecutionResources.GetFirstRemainingDiceIndexWithEffect(DiceEffect::PlayCard, inOutOnesDigit); }
		break;
	}
	case DicePlayStrategy::TensTopHalfRandom:
	{
		if (inOutTensDigit == -1) { inOutTensDigit = GetRandomIndexFromList(mExecutionResources.GetNBestDieRollIndexes(mExecutionResources.GetNumRemainingCombatRounds(), inOutOnesDigit)); }
		break;
	}
	case DicePlayStrategy::TensTopOrdered:
	{
		if (inOutTensDigit == -1) { inOutTensDigit = mExecutionResources.GetNBestDieRollIndexes(1, inOutOnesDigit)[0]; }
		break;
	}
	case DicePlayStrategy::OnesBottomHalfRandom:
	{
		if (inOutOnesDigit == -1) { inOutOnesDigit = GetRandomIndexFromList(mExecutionResources.GetNWorstDieRollIndexes(mExecutionResources.GetNumRemainingCombatRounds(), inOutTensDigit)); }
		break;
	}
	case DicePlayStrategy::OnesBottomOrdered:
	{
		if (inOutOnesDigit == -1) { inOutOnesDigit = mExecutionResources.GetNWorstDieRollIndexes(1, inOutTensDigit)[0]; }
		break;
	}
	default: break;
	}
}

void WPWorker::EvaluateCardStrategy(CardPlayStrategy cardPlayStrategy, int32_t numCards, const FullRollResult& ourCurrentRoll, const FullRollResult& opponentCurrentRoll, std::vector<int32_t>& outCardsToPlayAsIs, std::vector<int32_t>& outCardsToPlayAsSkillBonus)
{
	std::vector<int32_t> indexesToPlay;
	const int32_t roundsRemaining = (int32_t)mExecutionResources.GetNumRemainingCombatRounds();

	int32_t ourDamage = (mCurrentCommand == PlayCommand::Attack ? mDamage + ourCurrentRoll.CountEffects(DiceEffect::ResultBonus) : 0);
	const int32_t opponentDamage = mOpponent->mDamage + opponentCurrentRoll.CountEffects(DiceEffect::ResultBonus);

	int32_t opponentMaybeValue = opponentCurrentRoll.mTotalValue;

	// Attackers play cards first. Be cautious of Defender card plays
	if (mWorkerRole == WorkerRole::Attacker)
	{
		const int32_t opponentCardPlays = opponentCurrentRoll.CountEffects(DiceEffect::PlayCard);
		// todo: Evaluate for maybe higher level cards
		opponentMaybeValue += opponentCardPlays * 10;
	}

	switch (cardPlayStrategy)
	{
	default: break;
	case CardPlayStrategy::UseGuaranteedKill:
	{
		std::vector<int32_t> forceDamageOnlyIndexesToPlay;
		int32_t potentialDamage = 0;

		// Force Damage can still happen even if we're Fleeing
		const int32_t forceDamage = GetMaxMagnitudeOfCardEffectInHand(CardEffect::ForceDamage, numCards, forceDamageOnlyIndexesToPlay);
		if (forceDamage >= mOpponent->mHealth_Current)
		{
			outCardsToPlayAsIs = forceDamageOnlyIndexesToPlay;
			break;
		}

		if ((mCurrentCommand == PlayCommand::Attack) && (ourDamage < mOpponent->mHealth_Current))
		{
			indexesToPlay = forceDamageOnlyIndexesToPlay;
			potentialDamage += forceDamage;

			if (ourCurrentRoll.mTotalValue > opponentMaybeValue)
			{
				potentialDamage += GetMaxMagnitudeOfCardEffectInHand(CardEffect::PermanentDamage, numCards - (int32_t)indexesToPlay.size(), indexesToPlay);
				potentialDamage += GetMaxMagnitudeOfCardEffectInHand(CardEffect::TempBonusResult, numCards - (int32_t)indexesToPlay.size(), indexesToPlay);
			}

			if ((ourDamage + potentialDamage) >= mOpponent->mHealth_Current)
			{
				outCardsToPlayAsIs = indexesToPlay;
				break;
			}
		}
		break;
	}
	case CardPlayStrategy::MatchVSDice:
	{
		if (ourCurrentRoll.mTotalValue < opponentMaybeValue)
		{
			const int32_t potentialAddedSkill = GetMaxLevelOfCardsInHand(numCards, indexesToPlay) * 10;
			if ((ourCurrentRoll.mTotalValue + potentialAddedSkill) >= opponentMaybeValue)
			{
				outCardsToPlayAsSkillBonus = indexesToPlay;
				break;
			}

			if (GetMaxMagnitudeOfCardEffectInHand(CardEffect::SwapOpponentDice, numCards, indexesToPlay) > 0)
			{
				if (ourCurrentRoll.mTotalValue >= opponentCurrentRoll.GetTotalValueIfDiceSwapped())
				{
					outCardsToPlayAsIs = indexesToPlay;
					break;
				}
			}
		}
		break;
	}
	case CardPlayStrategy::BurstToSurvive:
	{
		if ((ourCurrentRoll.mTotalValue < opponentMaybeValue) && (opponentDamage >= mHealth_Current))
		{
			const int32_t potentialAddedSkill = GetMaxMagnitudeOfCardEffectInHand(CardEffect::TempSkill, numCards, indexesToPlay);
			if ((ourCurrentRoll.mTotalValue + potentialAddedSkill) >= opponentMaybeValue)
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::BurstToKill:
	{
		if ((ourCurrentRoll.mTotalValue < opponentMaybeValue) && (ourDamage >= mOpponent->mHealth_Current))
		{
			const int32_t potentialAddedSkill = GetMaxMagnitudeOfCardEffectInHand(CardEffect::TempSkill, numCards, indexesToPlay);
			if ((ourCurrentRoll.mTotalValue + potentialAddedSkill) >= opponentMaybeValue)
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::PermanentDamageIfDiceAreStrong:
	{
		if (GetMaxMagnitudeOfCardEffectInHand(CardEffect::PermanentDamage, numCards, indexesToPlay) > 0)
		{
			float strengthScore = 0.f;
			std::vector<int32_t> bestDieRollsRemaining = mExecutionResources.GetNBestDieRollIndexes(roundsRemaining);
			for (int32_t index : bestDieRollsRemaining)
			{
				if (mExecutionResources.GetDieRoll(index).mValue > 6)
				{
					strengthScore += (float)(mExecutionResources.GetDieRoll(index).mValue - 6);
				}
				else if (mExecutionResources.GetDieRoll(index).mValue == 6)
				{
					strengthScore += 0.5f;
				}
			}

			if (ourCurrentRoll.mTotalValue > opponentMaybeValue)
			{
				strengthScore += 3.0f;
			}

			if (strengthScore >= 8.0f)
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::ExtraEvalIfExtremelyConfident:
	{
		if ((roundsRemaining > 0) && mOpponent)
		{
			const int32_t extraEvals = GetMaxMagnitudeOfCardEffectInHand(CardEffect::ExtraEvaluate, numCards, indexesToPlay);
			if (extraEvals > 0 && CanLikelyWinAgainst(DiceFace::Best))
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::HealIfHurt:
	{
		if (mHealth_Current < mHealth_Max)
		{
			const int32_t potentialHeal = GetMaxMagnitudeOfCardEffectInHand(CardEffect::Heal, numCards, indexesToPlay);
			if (potentialHeal > 0)
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::ReplaceWorstDice:
	{
		if (roundsRemaining > 0)
		{
			const int32_t potentialReplaces = GetMaxMagnitudeOfCardEffectInHand(CardEffect::ReplaceWithRandomEffectDice, numCards, indexesToPlay);
			if (potentialReplaces > 0)
			{
				bool hasValidCardToReplace = false;
				std::vector<int32_t> worstDieRollIndexes = mExecutionResources.GetNWorstDieRollIndexes(2 + potentialReplaces);
				for (int32_t index : worstDieRollIndexes)
				{
					const DieRoll& dieRoll = mExecutionResources.GetDieRoll(index);
					if ((dieRoll.mMissingPotential > 0) && (dieRoll.GetSimplePredictedValue() < 6))
					{
						hasValidCardToReplace = true;
						break;
					}
				}

				if (hasValidCardToReplace)
				{
					outCardsToPlayAsIs = indexesToPlay;
				}
			}
		}
		break;
	}
	case CardPlayStrategy::RerollIfTooFewGoodTens:
	{
		if (roundsRemaining > 0)
		{
			const int32_t diceRerolls = GetMaxMagnitudeOfCardEffectInHand(CardEffect::RerollDice, numCards, indexesToPlay);
			if (diceRerolls > 0)
			{
				int32_t diceWithHighMissingPotential = CountDiceWithMissingPotentialNOrHigher(3);

				if (diceWithHighMissingPotential > 0)
				{
					const int32_t maxPotential = std::min(diceWithHighMissingPotential, diceRerolls);
					std::vector<int32_t> indexesOfPlayAsPermanent;
					const int32_t levelOfPlayableCards = GetMaxLevelOfCardsInHand(numCards, indexesOfPlayAsPermanent);

					if (maxPotential > levelOfPlayableCards)
					{
						outCardsToPlayAsIs = indexesToPlay;
					}
				}
			}
		}
		break;
	}
	case CardPlayStrategy::ExtraEvalIfVeryConfident:
	{
		if ((roundsRemaining > 0) && mOpponent)
		{
			const int32_t extraEvals = GetMaxMagnitudeOfCardEffectInHand(CardEffect::ExtraEvaluate, numCards, indexesToPlay);
			if (extraEvals > 0 && CanLikelyWinAgainst(DiceFace::Good))
			{
				outCardsToPlayAsIs = indexesToPlay;
			}
		}
		break;
	}
	case CardPlayStrategy::RerollInDesperation:
	{
		if ((roundsRemaining > 0) && mOpponent)
		{
			const int32_t diceRerolls = GetMaxMagnitudeOfCardEffectInHand(CardEffect::RerollDice, numCards, indexesToPlay);
			if (diceRerolls > 0 && !CanLikelyWinAgainst(DiceFace::Worst))
			{
				int32_t diceWithAnyMissingPotential = CountDiceWithMissingPotentialNOrHigher(1);

				if (diceWithAnyMissingPotential > 0)
				{
					const int32_t maxAnyPotential = std::min(diceWithAnyMissingPotential, diceRerolls);
					std::vector<int32_t> indexesOfPlayAsPermanent;
					const int32_t levelOfPlayableCards = GetMaxLevelOfCardsInHand(numCards, indexesOfPlayAsPermanent);

					if (maxAnyPotential > levelOfPlayableCards)
					{
						outCardsToPlayAsIs = indexesToPlay;
					}
				}
			}
		}
		break;
	}
	case CardPlayStrategy::DrawCardsIfDiceAreStrong:
	{
		if (roundsRemaining > 0 && (mExecutionResources.GetFirstRemainingDiceIndexWithEffect(DiceEffect::PlayCard) != -1))
		{
			int32_t drawsAvailable = GetMaxMagnitudeOfCardEffectInHand(CardEffect::DrawCard, numCards, indexesToPlay);
			drawsAvailable = std::min(drawsAvailable, (int32_t)mCurrentDeckIndices.size());
			if (drawsAvailable > 0)
			{
				const int32_t diceWithLowMissingPotential = CountDiceWithMissingPotentialNOrLower(0);
				if (diceWithLowMissingPotential >= roundsRemaining)
				{
					outCardsToPlayAsIs = indexesToPlay;
				}
			}
		}
		break;
	}
	}
}

bool WPWorker::CanLikelyWinAgainst(DiceFace diceFaceComparison) const
{
	const int32_t ourPredictedValue = mExecutionResources.GetDieRoll(mExecutionResources.GetNBestDieRollIndexes(GetExecutionResources().GetNumRemainingDice())[0]).GetSimplePredictedValue();

	for (const DieRoll& opponentDieRoll : mOpponent->GetExecutionResources().GetRemainingDice())
	{
		const int32_t opposingPredictedValue = WPExecutionResources::MakeDieRoll(opponentDieRoll.mDiceType, diceFaceComparison).GetSimplePredictedValue();
		if (opposingPredictedValue > ourPredictedValue)
		{
			return false;
		}
	}
	return true;
}

int32_t WPWorker::GetRandomIndexFromList(std::vector<int32_t> list) const
{
	if (list.size() < 1)
	{
		return -1;
	}
	return list[mRng.RandomIndex(list.size())];
}

int32_t WPWorker::GetMaxMagnitudeOfCardEffectInHand(CardEffect cardEffect, int32_t numCards, std::vector<int32_t>& inOutIndexesOfMax) const
{
	if (numCards <= 0)
	{
		return 0;
	}
	std::vector<std::pair<int32_t, int32_t>> indexToMagnitudePairs;

	for (int32_t index : mCurrentHandIndices)
	{
		const WPCard& card = mExecutionResources.GetCard(index);
		int32_t magnitude;
		if (card.GetCardEffect(cardEffect, magnitude))
		{
			if (indexToMagnitudePairs.size() == numCards)
			{
				auto iter = std::find_if(indexToMagnitudePairs.begin(), indexToMagnitudePairs.end(), [magnitude](const std::pair<int32_t, int32_t>& pair)
				{
					return pair.second < magnitude;
				});

				if (iter != indexToMagnitudePairs.end())
				{
					indexToMagnitudePairs.erase(iter);
					indexToMagnitudePairs.emplace_back(index, magnitude);
				}
			}
			else
			{
				indexToMagnitudePairs.emplace_back(index, magnitude);
			}
		}
	}

	int32_t totalMagnitude = 0;
	for (const std::pair<int32_t, int32_t>& pair : indexToMagnitudePairs)
	{
		inOutIndexesOfMax.push_back(pair.first);
		totalMagnitude += pair.second;
	}
	return totalMagnitude;
}

int32_t WPWorker::GetMaxLevelOfCardsInHand(int32_t numCards, std::vector<int32_t>& outIndexesOfMax) const
{
	outIndexesOfMax.clear();
	if (numCards == 0)
	{
		return 0;
	}
	std::vector<std::pair<int32_t, int32_t>> indexToLevelsPairs;
	for (int32_t index : mCurrentHandIndices)
	{
		const WPCard& card = mExecutionResources.GetCard(index);
		if (indexToLevelsPairs.size() == numCards)
		{
			auto iter = std::find_if(indexToLevelsPairs.begin(), indexToLevelsPairs.end(), [&card](const std::pair<int32_t, int32_t>& pair)
				{
					return pair.second < card.mCardLevel;
				});

			if (iter != indexToLevelsPairs.end())
			{
				indexToLevelsPairs.erase(iter);
				indexToLevelsPairs.emplace_back(index, card.mCardLevel);
			}
		}
		else
		{
			indexToLevelsPairs.emplace_back(index, card.mCardLevel);
		}
	}

	int32_t totalLevel = 0;
	for (const std::pair<int32_t, int32_t>& pair : indexToLevelsPairs)
	{
		outIndexesOfMax.push_back(pair.first);
		totalLevel += pair.second;
	}
	return totalLevel;
}

int32_t WPWorker::CountDiceWithMissingPotentialNOrHigher(int32_t threshold) const
{
	int32_t diceWithMissingPotential = 0;
	for (const DieRoll& dieRoll : mExecutionResources.GetRemainingDice())
	{
		diceWithMissingPotential += (dieRoll.mMissingPotential >= threshold);
	}
	return diceWithMissingPotential;
}

int32_t WPWorker::CountDiceWithMissingPotentialNOrLower(int32_t threshold) const
{
	int32_t diceWithMissingPotential = 0;
	for (const DieRoll& dieRoll : mExecutionResources.GetRemainingDice())
	{
		diceWithMissingPotential += (dieRoll.mMissingPotential <= threshold);
	}
	return diceWithMissingPotential;
}

int32_t WPWorker::FullRollResult::GetDieRollOnlyValue() const
{
	return (mTens.mValue * 10) + mOnes.mValue;
}

int32_t WPWorker::FullRollResult::CountEffects(DiceEffect diceEffect) const
{
	return (mTens.mDiceEffect == diceEffect) + (mOnes.mDiceEffect == diceEffect);
}

int32_t WPWorker::FullRollResult::GetTotalValueIfDiceSwapped() const
{
	const int32_t currentDieOnlyValue = GetDieRollOnlyValue();
	const int32_t postSwapDieOnlyValue = (mOnes.mValue * 10) + mTens.mValue;
	return mTotalValue + (postSwapDieOnlyValue - currentDieOnlyValue);
}

void WPWorker::FullRollResult::SwapDice()
{
	const int32_t oldDieOnlyValue = GetDieRollOnlyValue();
	const DieRoll oldOnesDieRoll = mOnes;
	mOnes = mTens;
	mTens = oldOnesDieRoll;
	mTotalValue += GetDieRollOnlyValue() - oldDieOnlyValue;
}

void WPWorker::FullRollResult::Print() const
{
	mTens.Print();
	mOnes.Print();
}
