#pragma once
#include "WPWorker.h"

class RNG;
class Stats;

enum class WPStatsIds : int32_t
{
	RollValues,
	WinMissingPotential,
	LossMissingPotential,
	LossHighestValue,
	Loss8Plus,
	LossSadnessScore
};
ENUM_OPS(WPStatsIds);

enum class ScenarioResult : uint8_t
{
	Stall,
	DefenderWin,
	AttackerWin,
	DefenderFlee,
	AttackerFlee,
	BothFlee,
	Count
};
ENUM_OPS(ScenarioResult);

enum class ScenarioPrintType : uint8_t
{
	None,
	EachExecute,
	MultiExecute,
	MultiExecuteAndStats
};
ENUM_OPS(ScenarioPrintType);

class WPScenario
{
public:
	WPScenario(const RNG& rng, Stats* stats = nullptr);

	ScenarioResult Execute();

	ScenarioResult ExecuteWithEndPrint();
	float MultiExecute(uint64_t numberToExecute); // return is Attacker advantage (e.g. winrate)

	void SetPrintType(ScenarioPrintType printType) { mPrintType = printType; }

	WPWorker& GetAttacker() { return mAttacker; }
	WPWorker& GetDefender() { return mDefender; }

private:
	ScenarioResult ExecuteInnerLoop();

	void PrintRemainingDice();
	void PrintWorkerTypeVSLine();
	void PrintVSLine(int32_t extraPadding);

	WPExecutionResources mAttackerResources;
	WPWorker mAttacker;
	WPExecutionResources mDefenderResources;
	WPWorker mDefender;
	Stats* mStats = nullptr;

	ScenarioPrintType mPrintType = ScenarioPrintType::None;
};

