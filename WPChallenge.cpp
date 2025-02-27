#include "WPChallenge.h"

WPChallenge::WPChallenge(const RNG& rng, Stats* stats /*= nullptr*/)
: mExecutionResources(rng)
, mRng(rng)
, mStats(stats)
{
}

ChallengeResult WPChallenge::Execute()
{
	ChallengeResult result;

	mExecutionResources.RollNOfEachDie(1);

	/**
	 * Figure out the best values we can use for all the possible rounds we can participate in
	 */
	const int32_t maxWins = std::min({ mExecutionResources.GetNumRemainingDice(), (int32_t)mWorkers.size() });

	const std::vector<int32_t> bestDieRollIndexes = mExecutionResources.GetNBestDieRollIndexes(maxWins);
	std::vector<int32_t> bestDieRollValues;
	for (int32_t dieIndex = 0; dieIndex < maxWins; ++dieIndex)
	{
		bestDieRollValues.push_back(mExecutionResources.GetDieRoll(dieIndex).mValue);
	}

	std::vector<int32_t> bestWorkerSkill;
	// Workers are presorted by skill (see: TryUpgradeWorkers)
	for (int32_t workerIndex = 0; workerIndex < maxWins; ++workerIndex)
	{
		bestWorkerSkill.push_back(mWorkers.at(workerIndex).GetWorkerSkill() / 10);
	}

	/**
	 * Find all matches that doesn't spend any cards
	 */
	std::vector<int32_t> unmatchedDieRollValues;
	for (int32_t dieRollValue : bestDieRollValues)
	{
		if (dieRollValue >= mChallengeDifficulty)
		{
			// Pair with worst worker
			result.mWins++;
			bestWorkerSkill.erase(std::prev(bestWorkerSkill.end()));
		}
		else
		{
			// Search for a cardless match, prioritizing the worst workers first
			for (std::vector<int32_t>::iterator workerIt = bestWorkerSkill.end(); workerIt != bestWorkerSkill.begin(); --workerIt)
			{
				const int32_t totalValue = (*std::prev(workerIt)) + dieRollValue;
				if (totalValue >= mChallengeDifficulty)
				{
					bestWorkerSkill.erase(std::prev(workerIt));
					result.mWins++;
					goto endcardlessworkerloop;
				}
			}

			unmatchedDieRollValues.push_back(dieRollValue);
		endcardlessworkerloop:
			(void)0;
		}
	}

	// Use cards for unfinished matches
	std::vector<int32_t> bestCardLevels = mExecutionResources.GetNHighestCardLevels(mExecutionResources.GetNumAllCards());
	for (int32_t dieRollValue : unmatchedDieRollValues)
	{
		for (std::vector<int32_t>::iterator workerIt = bestWorkerSkill.begin(); workerIt != bestWorkerSkill.end(); ++workerIt)
		{
			// Find an exact match with a single card
			const int32_t cardlessValue = dieRollValue + (*workerIt);
			for (std::vector<int32_t>::iterator cardIt = bestCardLevels.begin(); cardIt != bestCardLevels.end(); ++cardIt)
			{
				const int32_t totalValue = cardlessValue + (*cardIt);
				if (totalValue == mChallengeDifficulty)
				{
					bestWorkerSkill.erase(workerIt);
					bestCardLevels.erase(cardIt);
					result.mWins++;
					goto endcardedworkerloop;
				}
			}

			// Fill in multiple cards to make a match
			int32_t totalValue = cardlessValue;
			for (std::vector<int32_t>::iterator cardIt = bestCardLevels.end(); cardIt != bestCardLevels.begin(); --cardIt)
			{
				totalValue += *std::prev(cardIt);
				if (totalValue >= mChallengeDifficulty)
				{
					bestWorkerSkill.erase(workerIt);
					bestCardLevels.erase(std::prev(cardIt), std::prev(bestCardLevels.end()));
					result.mWins++;
					goto endcardedworkerloop;
				}
			}
		}
	endcardedworkerloop:
		(void)0;
	}

	return result;
}

void WPChallenge::SetupChallengeToRoundDefaults(int32_t roundNumber)
{
	mChallengeDifficulty = 6 + ((roundNumber - 1) / 2);

	ResetWorkers(3);
	TryUpgradeNWorkers(1, WorkerType::Basic, WorkerType::Trained);
	mExecutionResources.ResetCardsToDefault();
}

void WPChallenge::ResetWorkers(int32_t numWorkers)
{
	mWorkers.clear();
	for (int32_t workerIndex = 0; workerIndex < numWorkers; ++workerIndex)
	{
		mWorkers.emplace_back(mRng, mExecutionResources, mStats);
		mWorkers.back().SetupStatsFromWorkerType();
	}
}

void WPChallenge::TryUpgradeNWorkers(int32_t numWorkers, WorkerType fromType, WorkerType toType)
{
	int32_t workersUpgraded = 0;
	for (WPWorker& worker : mWorkers)
	{
		if (worker.GetWorkerType() == fromType)
		{
			worker.SetWorkerType(toType);
			worker.SetupStatsFromWorkerType();
			workersUpgraded++;
			if (workersUpgraded == numWorkers)
			{
				return;
			}
		}
	}
}
