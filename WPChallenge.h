#pragma once
#include "WPWorker.h"

class RNG;
class Stats;

struct ChallengeResult
{
	int32_t mWins = 0;
	int32_t mWinningHold = 0;
	int32_t mWinningDamage = 0;
	int32_t mWinningFlee = 0;
};

class WPChallenge
{
public:
	WPChallenge(const RNG& rng, Stats* stats = nullptr);

	ChallengeResult Execute();

	void SetupChallengeToRoundDefaults(int32_t roundNumber);
	void ResetWorkers(int32_t numWorkers);
	void TryUpgradeNWorkers(int32_t numWorkers, WorkerType fromType, WorkerType toType);

	WPExecutionResources& GetExecutionResources() { return mExecutionResources; }

private:
	WPExecutionResources mExecutionResources;
	const RNG& mRng;
	Stats* mStats = nullptr;
	std::vector<WPWorker> mWorkers;

	int32_t mChallengeDifficulty = 0;
};

