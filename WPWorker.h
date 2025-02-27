#pragma once
#include "MathCommon.h"
#include "MathPrint.h"
#include "WPCard.h"
#include "WPExecutionResources.h"
class RNG;
class Stats;

enum class WorkerRole : uint8_t
{
	Unset,
	Attacker,
	Defender
};
ENUM_OPS(WorkerRole);

enum class WorkerType : uint8_t
{
	Basic,
	Trained,
	Warrior,
	Rogue,

	// WPWorker::SetupStatsFromWorkerType and WPWorker::GetWorkerTypeName needs to be updated if changed here
	Count
};
ENUM_OPS(WorkerType);

enum class PlayCommand : uint8_t
{
	Attack,
	Flee,

	UseDeterminedCommand
};

enum class DicePlayStrategy : uint8_t
{
	IfExtraEvalThrowBest,
	IfExtraEvalThrowBestTens,
	IfMaxHealthThrowWorst,
	PlayCards,
	TensTopHalfRandom,
	TensTopOrdered,
	OnesBottomHalfRandom,
	OnesBottomOrdered
};
ENUM_OPS(DicePlayStrategy);

enum class CardPlayStrategy : uint8_t
{
	First,

	UseGuaranteedKill = First,
	MatchVSDice,
	BurstToSurvive,
	BurstToKill,
	PermanentDamageIfDiceAreStrong,
	ExtraEvalIfExtremelyConfident,
	HealIfHurt,
	ReplaceWorstDice,
	RerollIfTooFewGoodTens,
	ExtraEvalIfVeryConfident,
	RerollInDesperation,
	DrawCardsIfDiceAreStrong,

	Count
};
ENUM_OPS(CardPlayStrategy);

class WPWorker
{
public:
	struct FullRollResult
	{
		DieRoll mTens;
		DieRoll mOnes;
		int32_t mTotalValue;

		int32_t GetDieRollOnlyValue() const;
		int32_t CountEffects(DiceEffect diceEffect) const;
		int32_t GetTotalValueIfDiceSwapped() const;
		void SwapDice();
		void Print() const;
	};


	WPWorker(const RNG& rng, WPExecutionResources& executionResources, Stats* stats = nullptr);

	void SetWorkerType(WorkerType workerType);
	void SetArbitraryCardVariation(int32_t arbitraryCardStrength);

	void SetupStatsFromWorkerType();
	void PrepForCombat();

	void ClearThisRoundStats();
	void MoveToNextRound();

	void SetOpponent(WPWorker& opponent);
	void SetRole(WorkerRole role) { mWorkerRole = role; }
	void SetForceCommand(PlayCommand command) { mForcedCommand = command; }
	void SetDiceStrategy(std::vector<DicePlayStrategy> dicePlayStrategy);
	void SetCardStrategy(std::vector<CardPlayStrategy> cardPlayStrategy);
	void SetStrategyAsDefault();

	void DetermineCommand();
	FullRollResult PullDice(); // returns numerical amount
	void AddSkillBonuses(FullRollResult& inOutRollResult);
	void PlayCards(int32_t numCards, FullRollResult& ourCurrentRoll, FullRollResult& opponentCurrentRoll);

	void DrawCards(int32_t numCards);
	void DealStatBasedDamage(const FullRollResult& ourRollResult);
	void DealDamage(int32_t amount);
	void BuildFlee(int32_t amount);
	void BuildStatBasedFlee(const FullRollResult& ourRollResult, bool wonValueCompare);

	void IncreaseSkill(int32_t amount);

	int32_t GetCurrentWorstPossibleTotal() const;
	int32_t GetCurrentBestPossibleTotal() const;

	void PrintHealth() const;
	void PrintSkill() const;
	void PrintCurrentRound() const;
	void PrintCardsPlayedThisRound() const;

	bool IsAlive() const;
	bool FleeMatchesOpposingHold() const;
	PlayCommand GetCommand() const { return mCurrentCommand; }
	WorkerType GetWorkerType() const { return mWorkerType; }
	WorkerRole GetWorkerRole() const { return mWorkerRole; }
	int32_t GetWorkerSkill() const { return mSkill; }
	float GetTotalAverageRollValue() const;
	int32_t GetTotalEvaluatesThisRound() const;
	bool HasPlayedCardForTheirEffectsThisRound() const;

	static std::vector<DiceEffect> MakeDiceEffectList(DiceEffect diceEffect);
	static const char* GetWorkerTypeName(WorkerType workerType);

	const WPExecutionResources& GetExecutionResources() const { return mExecutionResources; }
private:
	void ExecuteSingleCard(const WPCard& card, FullRollResult& ourCurrentRoll, FullRollResult& opponentCurrentRoll);
	void EvaluateDiceStrategy(DicePlayStrategy strategy, int32_t& inOutTensDigit, int32_t& inOutOnesDigit) const;
	void EvaluateCardStrategy(CardPlayStrategy cardPlayStrategy, int32_t numCards, const FullRollResult& ourCurrentRoll, const FullRollResult& opponentCurrentRoll, std::vector<int32_t>& outCardsToPlayAsIs, std::vector<int32_t>& outCardsToPlayAsSkillBonus);
	bool CanLikelyWinAgainst(DiceFace diceFaceComparison) const;

	int32_t GetRandomIndexFromList(std::vector<int32_t> list) const;
	int32_t GetMaxMagnitudeOfCardEffectInHand(CardEffect cardEffect, int32_t numCards, std::vector<int32_t>& inOutIndexesOfMax) const;
	int32_t GetMaxLevelOfCardsInHand(int32_t numCards, std::vector<int32_t>& outIndexesOfMax) const;
	int32_t CountDiceWithMissingPotentialNOrHigher(int32_t threshold) const;
	int32_t CountDiceWithMissingPotentialNOrLower(int32_t threshold) const;

	const RNG& mRng;
	Stats* mStats = nullptr;

	int32_t mArbitraryCardVariation = 0;
	PlayCommand mForcedCommand = PlayCommand::UseDeterminedCommand;
	WorkerType mWorkerType = WorkerType::Basic;

	WPExecutionResources& mExecutionResources;
	std::vector<WPCard> mAllAvailableCards;

	PlayCommand mCurrentCommand = PlayCommand::Attack;
	std::vector<int32_t> mCurrentDeckIndices;
	std::vector<int32_t> mCurrentHandIndices;

	/**
	 * Persists over Executions
	 */
	int32_t mVP = 100;
	int32_t mTotalRolls = 0;
	int32_t mTotalRollValues = 0;

	/**
	 * Stats
	 */
	int32_t mHealth_Current = 0;
	int32_t mHealth_Max = 0;
	int32_t mSkill = 0;
	int32_t mDamage = 0;
	int32_t mHold = 0;
	int32_t mFlee = 0;
	int32_t mFlee_Current = 0;
	int32_t mStartingHandSize = 0;

	/**
	 * Per Round Stats
	 */
	int32_t mCurrentRound = 0;

	std::vector<int32_t> mThisRound_CardsPlayedAsIs;
	int32_t mThisRound_Skill = 0;
	int32_t mThisRound_BonusResult = 0;
	int32_t mThisRound_ExtraEvaluate = 0;
	bool mThisRound_SwapDice = false;

	int32_t mNextRound_ExtraEvaluate = 0;

	/**
	 * Strategy
	 */
	WPWorker* mOpponent = nullptr;
	WorkerRole mWorkerRole = WorkerRole::Unset;
	std::vector<DicePlayStrategy> mDicePlayStrategy;
	std::vector<CardPlayStrategy> mCardPlayStrategy;
};

