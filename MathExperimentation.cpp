// Booper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include "ConsoleInfo.h"
#include "ConsoleMenu.h"

#include <ctime>
#include <windows.h>

#include "RNG.h"
#include "Stats.h"
#include "WPChallenge.h"
#include "WPWorker.h"
#include "WPScenario.h"
#include "SmallestSquare.h"
#include "SquareContainmentMenu.h"

RNG* gRng = nullptr;

WorkerType gAttackerWorkerType = WorkerType::Basic;
WorkerType gDefenderWorkerType = WorkerType::Basic;
int32_t gAttackerCardVariation = 0;
int32_t gDefenderCardVariation = 0;

void AttackerNextWorkerType()
{
	gAttackerWorkerType++;
	if (gAttackerWorkerType == WorkerType::Count)
	{
		gAttackerWorkerType = WorkerType::Basic;
	}
	printf("Attacker is now a %s\n", WPWorker::GetWorkerTypeName(gAttackerWorkerType));
}

void DefenderNextWorkerType()
{
	gDefenderWorkerType++;
	if (gDefenderWorkerType == WorkerType::Count)
	{
		gDefenderWorkerType = WorkerType::Basic;
	}
	printf("Defender is now a %s\n", WPWorker::GetWorkerTypeName(gDefenderWorkerType));
}

void AttackerNextCardVariation()
{
	gAttackerCardVariation++;

	WPExecutionResources workerResources(*gRng);
	WPWorker worker(*gRng, workerResources);
	worker.SetArbitraryCardVariation(gAttackerCardVariation);

	workerResources.PrintCardVariations("Attacker");
	printf("\nCard Variation: %d\n", gAttackerCardVariation);
}

void DefenderNextCardVariation()
{
	gDefenderCardVariation++;

	WPExecutionResources workerResources(*gRng);
	WPWorker worker(*gRng, workerResources);
	worker.SetArbitraryCardVariation(gDefenderCardVariation);

	workerResources.PrintCardVariations("Defender");
	printf("\nCard Variation: %d\n", gDefenderCardVariation);
}

void WorkerPlacementDiceTest()
{
	WPScenario testScenario(*gRng);
	testScenario.GetDefender().SetDiceStrategy(
	{
		DicePlayStrategy::IfExtraEvalThrowBestTens
	});

	testScenario.GetAttacker().SetWorkerType(gAttackerWorkerType);
	testScenario.GetDefender().SetWorkerType(gDefenderWorkerType);

	testScenario.GetAttacker().SetArbitraryCardVariation(gAttackerCardVariation);
	testScenario.GetDefender().SetArbitraryCardVariation(gDefenderCardVariation);

	testScenario.SetPrintType(ScenarioPrintType::EachExecute);
	testScenario.ExecuteWithEndPrint();
}

void WorkerPlacementMultipleRuns(uint64_t numRuns)
{
	printf("\nRunning...\n");
	Stats stats;
	WPScenario testScenario(*gRng, &stats);
	// testScenario.GetDefender().SetDiceStrategy(
	// {
	// 	DicePlayStrategy::IfExtraEvalThrowBestTens,
	// 	DicePlayStrategy::TensTopHalfRandom,
	// 	DicePlayStrategy::OnesBottomHalfRandom
	// });

	testScenario.GetAttacker().SetWorkerType(gAttackerWorkerType);
	testScenario.GetDefender().SetWorkerType(gDefenderWorkerType);

	testScenario.GetAttacker().SetArbitraryCardVariation(gAttackerCardVariation);
	testScenario.GetDefender().SetArbitraryCardVariation(gDefenderCardVariation);
	//testScenario.GetAttacker().SetForceCommand(PlayCommand::Attack);
	//testScenario.GetDefender().SetForceCommand(PlayCommand::Flee);

	testScenario.SetPrintType(ScenarioPrintType::MultiExecute);
	testScenario.MultiExecute(numRuns);
}

void WorkerPlacementMultipleChallenges(uint64_t numRuns)
{
	printf("\nRunning...\n");
	Stats stats;
	WPChallenge testChallenge(*gRng, &stats);
	testChallenge.SetupChallengeToRoundDefaults(1);

	int32_t wins = 0;

	for (int32_t run = 0; run < numRuns; ++run)
	{
		wins += testChallenge.Execute().mWins;
	}

	printf("\nWins: %d\n", wins);
}

void WorkerPlacementMultipleChallengesTTT(uint64_t numRuns)
{
	printf("\nRunning...\n");
	Stats stats;
	WPChallenge testChallenge(*gRng, &stats);
	testChallenge.SetupChallengeToRoundDefaults(1);
	testChallenge.TryUpgradeNWorkers(1, WorkerType::Basic, WorkerType::Trained);

	int32_t wins = 0;

	for (int32_t run = 0; run < numRuns; ++run)
	{
		wins += testChallenge.Execute().mWins;
	}

	printf("\nWins: %d\n", wins);
}

void WorkerPlacementMultipleChallengesWar(uint64_t numRuns)
{
	printf("\nRunning...\n");
	Stats stats;
	WPChallenge testChallenge(*gRng, &stats);
	testChallenge.SetupChallengeToRoundDefaults(1);
	testChallenge.TryUpgradeNWorkers(1, WorkerType::Trained, WorkerType::Warrior);

	int32_t wins = 0;

	for (int32_t run = 0; run < numRuns; ++run)
	{
		wins += testChallenge.Execute().mWins;
	}

	printf("\nWins: %d\n", wins);
}

void WorkerPlacementMultipleChallengesCard(uint64_t numRuns)
{
	printf("\nRunning...\n");
	Stats stats;
	WPChallenge testChallenge(*gRng, &stats);
	testChallenge.SetupChallengeToRoundDefaults(1);
	testChallenge.GetExecutionResources().AddCardVariation(1);

	int32_t wins = 0;

	for (int32_t run = 0; run < numRuns; ++run)
	{
		wins += testChallenge.Execute().mWins;
	}

	printf("\nWins: %d\n", wins);
}

void TriangleSmallestSquare(uint64_t aX, uint64_t aY, uint64_t bX, uint64_t bY, uint64_t cX, uint64_t cY)
{
	const NamedVector2 posA(aX, aY);
	const NamedVector2 posB(bX, bY);
	const NamedVector2 posC(cX, cY);
	const SmallestSquare smallestSquare(posA, posB, posC);

	printf("\n{%f, %f}, {%f, %f}, {%f, %f}", posA.X(), posA.Y(), posB.X(), posB.Y(), posC.X(), posC.Y());

	printf("\nBig Side: {%f, %f} (length: %f)", smallestSquare.GetBigSide().X(), smallestSquare.GetBigSide().Y(), smallestSquare.GetBigSide().Magnitude());
	printf("\nMed Side: {%f, %f} (length: %f)", smallestSquare.GetMediumSide().X(), smallestSquare.GetMediumSide().Y(), smallestSquare.GetMediumSide().Magnitude());
	printf("\nSml Side: {%f, %f} (length: %f)", smallestSquare.GetSmallSide().X(), smallestSquare.GetSmallSide().Y(), smallestSquare.GetSmallSide().Magnitude());

	printf("\n\nSmall Info, Angle: %f, Cosine: %f, Sine: %f", smallestSquare.GetAngleSmallDegrees(), smallestSquare.GetCosSmall(), smallestSquare.GetSinSmall());
	printf("\nBig Angle: %f, Medium Angle %f", smallestSquare.GetAngleBigDegrees(), smallestSquare.GetAngleMediumDegrees());

	const double smallestSide = smallestSquare.CalculateSmallestSquareSide();
	printf("\n\nSmallest Side: %f\n", smallestSide);
}

void TriangleSmallestSquareAssumeOrigin(uint64_t bX, uint64_t bY, uint64_t cX, uint64_t cY)
{
	TriangleSmallestSquare(0, 0, bX, bY, cX, cY);
}


/**
 * Main
 */
int main()
{
	gRng = new RNG();

	ConsoleMenu fightMenu("Fight Menu");
	fightMenu.AddCommand("s", "Single Fight", WorkerPlacementDiceTest);
	fightMenu.AddCommand("m", "Multiple Fights", WorkerPlacementMultipleRuns);
	fightMenu.AddCommand("aw", "Attacker: Next Worker Type", AttackerNextWorkerType);
	fightMenu.AddCommand("dw", "Defender: Next Worker Type", DefenderNextWorkerType);
	fightMenu.AddCommand("ac", "Attacker: Next Card Variation", AttackerNextCardVariation);
	fightMenu.AddCommand("dc", "Defender: Next Card Variation", DefenderNextCardVariation);

	ConsoleMenu challengeMenu("Challenge Menu");
	challengeMenu.AddCommand("m", "Multiple Challenges", WorkerPlacementMultipleChallenges);
	challengeMenu.AddCommand("tt", "Test two trained", WorkerPlacementMultipleChallengesTTT);
	challengeMenu.AddCommand("tw", "Test one warrior", WorkerPlacementMultipleChallengesWar);
	challengeMenu.AddCommand("tc", "Test one 2 card", WorkerPlacementMultipleChallengesCard);

	ConsoleMenu triangleMenu("Triangle Menu");
	triangleMenu.AddCommand("a", "All sides: Smallest Square;dA.x;dA.y;dB.x;dB.y;dC.x;dC.y", TriangleSmallestSquare);
	triangleMenu.AddCommand("o", "Origin assumed (0,0): Smallest Square;dB.x;dB.y;dC.x;dC.y", TriangleSmallestSquareAssumeOrigin);
	triangleMenu.AddCommand("s", "4 Point Square Containment. Origin assumed (0,0);dB.x;dB.y;dC.x;dC.y;dD.x;dD.y", TriangleSmallestSquareAssumeOrigin);

	ConsoleMenu squareContainmentMenu("Square Containment Menu");
	ConsoleMenu squareContainmentAnalysisMenu("Analysis Menu");
	SquareContainmentMenu::SetupMenu(squareContainmentMenu, squareContainmentAnalysisMenu);

	ConsoleMenu mainMenu("Main Menu");
	mainMenu.AddSubmenu("f", fightMenu);
	mainMenu.AddSubmenu("c", challengeMenu);
	mainMenu.AddSubmenu("t", triangleMenu);
	mainMenu.AddSubmenu("s", squareContainmentMenu, SquareContainmentMenu::PreOpenMenu);
	mainMenu.RunMenu();

	delete gRng;
}

