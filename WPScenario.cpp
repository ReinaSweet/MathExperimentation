#include "WPScenario.h"

#include "Stats.h"

WPScenario::WPScenario(const RNG& rng, Stats* stats)
: mAttackerResources(rng)
, mAttacker(rng, mAttackerResources, stats)
, mDefenderResources(rng)
, mDefender(rng, mDefenderResources, stats)
, mStats(stats)
{
	mAttacker.SetStrategyAsDefault();
	mAttacker.SetRole(WorkerRole::Attacker);
	mAttackerResources.ResetCardsToDefault();
	mDefender.SetStrategyAsDefault();
	mDefender.SetRole(WorkerRole::Defender);
	mDefenderResources.ResetCardsToDefault();
}

ScenarioResult WPScenario::Execute()
{
	mAttacker.PrepForCombat();
	mDefender.PrepForCombat();
	mAttacker.SetOpponent(mDefender);
	mDefender.SetOpponent(mAttacker);

	if (mPrintType == ScenarioPrintType::EachExecute)
	{
		printf("\n");
		PrintWorkerTypeVSLine();
		printf("\n\n");
		PrintRemainingDice();
		printf("\n          ");
		PrintVSLine(6);
		printf("\n");
	}

	int32_t attackerSadnessScore = 0;
	int32_t attackerMissingPotential = 0;
	int32_t attackerHighestValue = 0;
	int32_t attacker8plus = 0;
	for (const DieRoll& roll : mAttackerResources.GetRemainingDice())
	{
		attackerMissingPotential += roll.mMissingPotential;
		attackerHighestValue = std::max(attackerHighestValue, roll.mValue);

		attackerSadnessScore += roll.mMissingPotential + (roll.mMissingPotential >= 4);
		attacker8plus += (roll.mValue >= 8);
	}
	attackerSadnessScore += 3 - (attackerHighestValue - 6);

	int32_t defenderSadnessScore = 0;
	int32_t defenderMissingPotential = 0;
	int32_t defenderHighestValue = 0;
	int32_t defender8plus = 0;
	for (const DieRoll& roll : mDefenderResources.GetRemainingDice())
	{
		defenderMissingPotential += roll.mMissingPotential;
		defenderHighestValue = std::max(defenderHighestValue, roll.mValue);

		defenderSadnessScore += roll.mMissingPotential + (roll.mMissingPotential >= 4);
		defender8plus += (roll.mValue >= 8);
	}
	defenderSadnessScore += 3 - (defenderHighestValue - 6);

	const ScenarioResult result = ExecuteInnerLoop();
	if (mStats)
	{
		if (result == ScenarioResult::AttackerWin)
		{
			mStats->AddToIntDistribution(+WPStatsIds::WinMissingPotential, attackerMissingPotential);
			mStats->AddToIntDistribution(+WPStatsIds::LossMissingPotential, defenderMissingPotential);
			mStats->AddToIntDistribution(+WPStatsIds::LossHighestValue, defenderHighestValue);
			mStats->AddToIntDistribution(+WPStatsIds::Loss8Plus, defender8plus);
			mStats->AddToIntDistribution(+WPStatsIds::LossSadnessScore, defenderSadnessScore);
		}
		else if (result == ScenarioResult::DefenderWin)
		{
			mStats->AddToIntDistribution(+WPStatsIds::WinMissingPotential, defenderMissingPotential);
			mStats->AddToIntDistribution(+WPStatsIds::LossMissingPotential, attackerMissingPotential);
			mStats->AddToIntDistribution(+WPStatsIds::LossHighestValue, attackerHighestValue);
			mStats->AddToIntDistribution(+WPStatsIds::Loss8Plus, attacker8plus);
			mStats->AddToIntDistribution(+WPStatsIds::LossSadnessScore, attackerSadnessScore);
		}
	}
	return result;
}

ScenarioResult WPScenario::ExecuteWithEndPrint()
{
	const ScenarioResult result = Execute();
	if (mPrintType == ScenarioPrintType::EachExecute)
	{
		printf("\n\n       ");
		PrintVSLine(3);
		printf("\n");

		switch (result)
		{
		case ScenarioResult::Stall: printf("==Stalled=="); break;
		case ScenarioResult::DefenderWin: printf("Defender Wins!"); break;
		case ScenarioResult::AttackerWin:  printf("Attacker Wins!"); break;
		default: break;
		}
		printf("\n");
	}
	return result;
}

float WPScenario::MultiExecute(uint64_t numberToExecute)
{
	int32_t results[+ScenarioResult::Count] = {};

	for (uint64_t i = 0; i < numberToExecute; ++i)
	{
		results[+Execute()]++;
	}

	int32_t total = 0;
	for (ScenarioResult result = ScenarioResult::Stall; result < ScenarioResult::Count; ++result)
	{
		total += results[+result];
	}

	float resultPercents[+ScenarioResult::Count] = {};
	for (ScenarioResult result = ScenarioResult::Stall; result < ScenarioResult::Count; ++result)
	{
		resultPercents[+result] = ((float)results[+result] / (float)total);
	}

	if (mPrintType >= ScenarioPrintType::MultiExecute)
	{
		printf("\n");
		PrintWorkerTypeVSLine();
		mAttackerResources.PrintCardVariations("Attacker");
		mDefenderResources.PrintCardVariations("Defender");

		printf("\n"
			"Stalls: %i (%.2f%%)\n"
			"Attacker Flees: %i (%.2f%%)\n"
			"Defender Flees: %i (%.2f%%)\n"
			"Both Flees: %i (%.2f%%)\n"
			"Defender Wins: %i (%.2f%%)\n"
			"Attacker Wins: %i (%.2f%%)\n",
			results[+ScenarioResult::Stall], resultPercents[+ScenarioResult::Stall] * 100.f,
			results[+ScenarioResult::AttackerFlee], resultPercents[+ScenarioResult::AttackerFlee] * 100.f,
			results[+ScenarioResult::DefenderFlee], resultPercents[+ScenarioResult::DefenderFlee] * 100.f,
			results[+ScenarioResult::BothFlee], resultPercents[+ScenarioResult::BothFlee] * 100.f,
			results[+ScenarioResult::DefenderWin], resultPercents[+ScenarioResult::DefenderWin] * 100.f,
			results[+ScenarioResult::AttackerWin], resultPercents[+ScenarioResult::AttackerWin] * 100.f);

		if (mStats && mPrintType == ScenarioPrintType::MultiExecuteAndStats)
		{
			mStats->SetIntDistributionAxisNames(+WPStatsIds::RollValues, "Rolls", "Total Value");
			mStats->PrintIntDistribution(+WPStatsIds::RollValues, 40, 20);

			mStats->SetIntDistributionAxisNames(+WPStatsIds::WinMissingPotential, "+Potential", "Losses");
			mStats->PrintIntDistribution(+WPStatsIds::WinMissingPotential, 40, 20);

			mStats->SetIntDistributionAxisNames(+WPStatsIds::LossMissingPotential, "-Potential", "Losses");
			mStats->PrintIntDistribution(+WPStatsIds::LossMissingPotential, 40, 20);

			mStats->SetIntDistributionAxisNames(+WPStatsIds::LossHighestValue, "Value", "Losses");
			mStats->PrintIntDistribution(+WPStatsIds::LossHighestValue, 40, 20);

			mStats->SetIntDistributionAxisNames(+WPStatsIds::Loss8Plus, "8Plus", "Losses");
			mStats->PrintIntDistribution(+WPStatsIds::Loss8Plus, 40, 20);

			mStats->SetIntDistributionAxisNames(+WPStatsIds::LossSadnessScore, "Score", "Losses");
			mStats->PrintIntDistribution(+WPStatsIds::LossSadnessScore, 40, 20);
		}
	}
	return resultPercents[+ScenarioResult::AttackerWin];
}

ScenarioResult WPScenario::ExecuteInnerLoop()
{
	while (mAttackerResources.GetRemainingDice().size() > 1)
	{
		mAttacker.DetermineCommand();
		mDefender.DetermineCommand();

		WPWorker::FullRollResult attackerRollResult = mAttacker.PullDice();
		mAttacker.AddSkillBonuses(attackerRollResult);

		WPWorker::FullRollResult defenderRollResult = mDefender.PullDice();
		mDefender.AddSkillBonuses(defenderRollResult);

		const int32_t attackerCardsToPlay = attackerRollResult.CountEffects(DiceEffect::PlayCard);
		if (attackerCardsToPlay > 0)
		{
			mAttacker.PlayCards(attackerCardsToPlay, attackerRollResult, defenderRollResult);

			// Cards can lead to instant wins
			if (!mDefender.IsAlive())
			{
				return ScenarioResult::AttackerWin;
			}
		}

		const int32_t defenderCardsToPlay = defenderRollResult.CountEffects(DiceEffect::PlayCard);
		if (defenderCardsToPlay > 0)
		{
			mDefender.PlayCards(defenderCardsToPlay, defenderRollResult, attackerRollResult);

			// Cards can lead to instant wins
			if (!mAttacker.IsAlive())
			{
				return ScenarioResult::DefenderWin;
			}
		}

		if (mPrintType == ScenarioPrintType::EachExecute)
		{
			printf("\n");
			mAttacker.PrintCurrentRound();

			attackerRollResult.Print();
			mAttacker.PrintSkill();

			printf(" ");
			mAttacker.PrintHealth();

			printf(" vs ");
			mDefender.PrintHealth();
			printf(" ");

			defenderRollResult.Print();
			mDefender.PrintSkill();

			printf("   ");
			if (mAttacker.HasPlayedCardForTheirEffectsThisRound())
			{
				printf("Attacker Cards: ");
				mAttacker.PrintCardsPlayedThisRound();
			}
			if (mDefender.HasPlayedCardForTheirEffectsThisRound())
			{
				printf("Defender Cards: ");
				mDefender.PrintCardsPlayedThisRound();
			}
		}

		const bool attackerWonRollResult = (attackerRollResult.mTotalValue > defenderRollResult.mTotalValue);
		const bool defenderWonRollResult = (defenderRollResult.mTotalValue > attackerRollResult.mTotalValue);

		bool attackerFled = false;
		if (mAttacker.GetCommand() == PlayCommand::Flee)
		{
			mAttacker.BuildStatBasedFlee(attackerRollResult, attackerWonRollResult);
			attackerFled = mAttacker.FleeMatchesOpposingHold();
		}

		bool defenderFled = false;
		if (mDefender.GetCommand() == PlayCommand::Flee)
		{
			mDefender.BuildStatBasedFlee(defenderRollResult, defenderWonRollResult);
			defenderFled = mDefender.FleeMatchesOpposingHold();
		}

		if (attackerFled)
		{
			return defenderFled ? ScenarioResult::BothFlee : ScenarioResult::AttackerFlee;
		}
		else if (defenderFled)
		{
			return ScenarioResult::DefenderFlee;
		}

		if (attackerWonRollResult)
		{
			if (mAttacker.GetCommand() == PlayCommand::Attack)
			{
				for (int32_t evaluates = mAttacker.GetTotalEvaluatesThisRound(); evaluates > 0; --evaluates)
				{
					mAttacker.DealStatBasedDamage(attackerRollResult);
					if (!mDefender.IsAlive())
					{
						return ScenarioResult::AttackerWin;
					}
				}
			}
		}
		else if (defenderWonRollResult)
		{
			if (mDefender.GetCommand() == PlayCommand::Attack)
			{
				for (int32_t evaluates = mDefender.GetTotalEvaluatesThisRound(); evaluates > 0; --evaluates)
				{
					mDefender.DealStatBasedDamage(defenderRollResult);
					if (!mAttacker.IsAlive())
					{
						return ScenarioResult::DefenderWin;
					}
				}
			}
		}

		mAttacker.ClearThisRoundStats();
		mDefender.ClearThisRoundStats();
		mAttacker.MoveToNextRound();
		mDefender.MoveToNextRound();
	}
	return ScenarioResult::Stall;
}

void WPScenario::PrintRemainingDice()
{
	const std::vector<DieRoll>& attackerRolls = mAttackerResources.GetRemainingDice();
	for (const DieRoll& dieRoll : attackerRolls)
	{
		dieRoll.Print();
	}

	printf(" vs ");

	const std::vector<DieRoll>& defenderRolls = mDefenderResources.GetRemainingDice();
	for (const DieRoll& dieRoll : defenderRolls)
	{
		dieRoll.Print();
	}
}

void WPScenario::PrintWorkerTypeVSLine()
{
	printf("%s vs %s",
		WPWorker::GetWorkerTypeName(mAttacker.GetWorkerType()), WPWorker::GetWorkerTypeName(mDefender.GetWorkerType()));
}

void WPScenario::PrintVSLine(int32_t extraPadding)
{
	mAttacker.PrintSkill();
	printf("%*s", extraPadding, "");
	mAttacker.PrintHealth();
	printf(" vs ");
	mDefender.PrintHealth();
	printf("%*s", extraPadding, "");
	mDefender.PrintSkill();
}
