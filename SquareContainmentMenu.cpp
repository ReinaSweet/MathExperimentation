#include "SquareContainmentMenu.h"
#include "ConsoleMenu.h"
#include "MathCommon.h"
#include "SquareContainment.h"

#include <sstream>

namespace SquareContainmentMenu
{
	SquareContainmentMenu::GlobalData gGlobalData;

	int32_t IncrementalTestForMax(const SquareContainment& prevSquareContainment, const std::vector<NamedVector2>& addablePoints, const std::vector<NamedVector2>& fixedPoints, std::vector<NamedVector2>& outLargestSetOfPoints);

	template<int32_t DEBUG_FIXED_MAX>
	int32_t IncrementalTestForMax_DbgT(const SquareContainment& prevSquareContainment, const std::vector<NamedVector2>& addablePoints, const std::vector<NamedVector2>& fixedPoints, std::vector<NamedVector2>& outLargestSetOfPoints)
	{
		int32_t max = (int32_t)fixedPoints.size();
		for (size_t i = 0; i < addablePoints.size(); ++i)
		{
			std::vector<NamedVector2> testPoints = fixedPoints;
			testPoints.emplace_back(addablePoints[i]);
			std::vector<NamedVector2> newAddablePoints;
			newAddablePoints.reserve(addablePoints.size() - (i + 1));
			for (size_t copyI = i + 1; copyI < addablePoints.size(); ++copyI)
			{
				newAddablePoints.emplace_back(addablePoints[copyI]);
			}

			if (prevSquareContainment.PointIsWithinHull(addablePoints[i]))
			{
				const int32_t newMax = IncrementalTestForMax(prevSquareContainment, newAddablePoints, testPoints, outLargestSetOfPoints);
				if (newMax > max)
				{
					max = newMax;
				}
				if (testPoints.size() > outLargestSetOfPoints.size())
				{
					outLargestSetOfPoints = testPoints;
				}
			}
			else
			{
				SquareContainment squareContainment(testPoints);
				if (squareContainment.Test(SquareContainment::kDefaultSideLength) < SquareContainmentResult::kBELOWFits_ABOVEFails)
				{
					const int32_t newMax = std::max(max, IncrementalTestForMax(squareContainment, newAddablePoints, testPoints, outLargestSetOfPoints));
					if (newMax > max)
					{
						max = newMax;
					}
					if (testPoints.size() > outLargestSetOfPoints.size())
					{
						outLargestSetOfPoints = testPoints;
					}
				}
			}
		}
		return max;
	}

	int32_t IncrementalTestForMax(const SquareContainment& prevSquareContainment, const std::vector<NamedVector2>& addablePoints, const std::vector<NamedVector2>& fixedPoints, std::vector<NamedVector2>& outLargestSetOfPoints)
	{
		switch (fixedPoints.size())
		{
		REPEAT_CASE_TEMPLATE_DEBUG_64(return IncrementalTestForMax_DbgT, prevSquareContainment, addablePoints, fixedPoints, outLargestSetOfPoints);
		default: return IncrementalTestForMax_DbgT<-1>(prevSquareContainment, addablePoints, fixedPoints, outLargestSetOfPoints);
		}
	}

	int32_t MaxInclusions::GetMax(const std::vector<NamedVector2>& fixedPoints, SetType ofSet, int32_t maxSections, std::vector<NamedVector2>& outLargestSetOfPoints)
	{
		std::vector<NamedVector2> removablePoints;
		AddPointsFromSettedPoints(ofSet, maxSections, removablePoints);

		int32_t preExcludedMax = 0;

		// Remove all points that by their own fail with the fixedPoints or already exist within fixedPoints
		std::erase_if(removablePoints, [&fixedPoints, &preExcludedMax](const NamedVector2& point) -> bool
			{
				auto iter = std::find_if(fixedPoints.begin(), fixedPoints.end(), [&point](const NamedVector2& fixedPoint)
					{
						return point == fixedPoint;
					});
				if (iter != fixedPoints.end())
				{
					preExcludedMax++;
					return true;
				}
				std::vector<NamedVector2> removeTestPoints = fixedPoints;
				removeTestPoints.emplace_back(point);
				return !SquareContainment::SimpleTest(removeTestPoints, SquareContainment::kDefaultSideLength);
			});

		// Check all
		std::vector<NamedVector2> allPoints = fixedPoints;
		allPoints.insert(allPoints.end(), removablePoints.begin(), removablePoints.end());
		if (SquareContainment::SimpleTest(allPoints, SquareContainment::kDefaultSideLength))
		{
			outLargestSetOfPoints = allPoints;
			return (int32_t)removablePoints.size() + preExcludedMax;
		}

		SquareContainment baseSquareContainment(fixedPoints);

		return IncrementalTestForMax(baseSquareContainment, removablePoints, fixedPoints, outLargestSetOfPoints) - (int32_t)fixedPoints.size() + preExcludedMax;
	}

	void MaxInclusions::FillAllMax(const std::vector<NamedVector2>& fixedPoints)
	{
		T = GetMax(fixedPoints, SetType::T, 1, mExampleMaxT);
		D = GetMax(fixedPoints, SetType::D, 2, mExampleMaxD);
		I = GetMax(fixedPoints, SetType::I, 4, mExampleMaxI);
		P = GetMax(fixedPoints, SetType::P, 4, mExampleMaxP);
		Z = GetMax(fixedPoints, SetType::Z, 4, mExampleMaxZ);
	}

	void MaxInclusions::FillAllMaxWithFixedSet(SetType fixedSet, int32_t fixedSetMaxSections)
	{
		std::vector<NamedVector2> fixedPoints;
		AddPointsFromSettedPoints(fixedSet, fixedSetMaxSections, fixedPoints);
		FillAllMax(fixedPoints);
	}

	void MaxInclusions::PrintMaxes()
	{
		printf("\nT: %i  e.g. ", T);
		PrintSpecificSetOfPoints(mExampleMaxT, false);
		printf("\nD: %i  e.g. ", D);
		PrintSpecificSetOfPoints(mExampleMaxD, false);
		printf("\nI: %i  e.g. ", I);
		PrintSpecificSetOfPoints(mExampleMaxI, false);
		printf("\nP: %i  e.g. ", P);
		PrintSpecificSetOfPoints(mExampleMaxP, false);
		printf("\nZ: %i  e.g. ", Z);
		PrintSpecificSetOfPoints(mExampleMaxZ, false);
	}

	void MaxInclusions::BuildAndPrintInclusions(std::vector<SetType> fixedSets, int32_t fixedSetMaxSections)
	{
		MaxInclusions inclusions;
		std::vector<NamedVector2> fixedPoints;
		printf("\n\nFixed on ");
		for (SetType setType : fixedSets)
		{
			AddPointsFromSettedPoints(setType, fixedSetMaxSections, fixedPoints);
			const std::string& setName = ToString(setType);
			printf("%s ", setName.c_str());
		}

		printf("maximums:\n");
		inclusions.FillAllMax(fixedPoints);
		inclusions.PrintMaxes();
	}

	void MaxInclusions::BuildByNameAndPrintInclusions(std::vector<std::string> names)
	{
		MaxInclusions inclusions;
		std::vector<NamedVector2> fixedPoints;
		printf("\n\nFixed on ");
		AddPointsFromSettedPointsByName(names, fixedPoints);
		for (const std::string& name : names)
		{
			printf("%s ", name.c_str());
		}

		printf(":\n");
		inclusions.FillAllMax(fixedPoints);
		inclusions.PrintMaxes();
	}

}

void SquareContainmentMenu::SetupMenu(ConsoleMenu& inMenu, ConsoleMenu& analysisMenu)
{
	SquareContainmentMenu::LoadGlobalData();
	inMenu.AddCommand("as", "Add Single Point;dx;dy", AddPoint);
	inMenu.AddCommand("af", "Add Full line of Points (doesn't clear points);Points", AddLineOfPoints);
	inMenu.AddCommand("acf", "Add Full line of Points (clearing previous points);Points", ClearAndAddLineOfPoints);
	inMenu.AddCommand("anf", "Add Named line of Points (clearing previous points);Points", ClearAndAddNamedLineOfPoints);
	inMenu.AddCommand(SetPointsToPredefinedSet_PreArgs, "ap", "Assign Points to Predefined Sets;dSet Index", SetPointsToPredefinedSet);

	inMenu.AddCommand("rs", "Remove Point (by index);dIndex", RemovePointByIndex);
	inMenu.AddCommand("rc", "Clear All Points;dEnter 101 to confirm", ClearAllPoints);
	inMenu.AddCommand("l", "Reload all Predefined Sets", ReloadPredefinedSets);

	inMenu.AddCommand("t", "Test Current Set of Points;dSquare Side Length", TestCurrentSetOfPoints);
	inMenu.AddCommand("mt", "10000 length side, Test Current Set of Points", TestCurrentSetOfPoints_Forced10000);
	inMenu.AddCommand("mx", "10000 length side, Seek Failure on X (fixed Y) Seeking Test;dExisting Point Index", SeekingTestX_Forced10000);
	inMenu.AddCommand("my", "10000 length side, Seek Failure on Y (fixed X) Seeking Test;dExisting Point Index", SeekingTestY_Forced10000);
	inMenu.AddCommand("md", "10000 length side, Seek Failure on Diagonal (X,Y equiv) Seeking Test;dExisting Point Index", SeekingTestXY_Forced10000);
	inMenu.AddCommand("mbd", "10000 length side, TwoPoint Seek Failure;dExisting Point Index to mX;dExisting Point Index to mY", TwoPointSeekingTestXY_Forced10000);
	
	inMenu.AddCommand("pa", "Print All Points", Analyze_PrintAllPoints);
	inMenu.AddCommand("pe", "Point Exclusion", Analyze_PointExclusions);
	inMenu.AddCommand("pc", "Print Checked Assertions", Analyze_CheckExpectedFails);
}

void SquareContainmentMenu::AddPointsFromSettedPointsSpcIndex(SetType setType, int32_t index, int32_t numSections, std::vector<NamedVector2>& inOutPoints)
{
	for (const std::pair<std::string, SettedPoint>& pair : gGlobalData.GetSettedPoints())
	{
		if (pair.second.mSection > numSections)
		{
			break;
		}
		if (pair.second.IsPartOfSet(setType) && pair.second.mIndex == index)
		{
			inOutPoints.emplace_back(pair.second.mPoint);
		}
	}
}

void SquareContainmentMenu::AddPointsFromSettedPoints(SetType setType, int32_t numSections, std::vector<NamedVector2>& inOutPoints)
{
	for (const std::pair<std::string, SettedPoint>& pair : gGlobalData.GetSettedPoints())
	{
		if (pair.second.mSection > numSections)
		{
			continue;
		}
		if (pair.second.IsPartOfSet(setType))
		{
			inOutPoints.emplace_back(pair.second.mPoint);
		}
	}
}

void SquareContainmentMenu::AddPointsFromSettedPointsByName(std::vector<std::string> names, std::vector<NamedVector2>& inOutPoints)
{
	for (const std::string& name : names)
	{
		auto iter = gGlobalData.GetSettedPoints().find(name);
		if (iter != gGlobalData.GetSettedPoints().end())
		{
			inOutPoints.emplace_back((*iter).second.mPoint, name);
		}
	}
}

void SquareContainmentMenu::LoadGlobalData()
{
	gGlobalData.LoadData();
}

void SquareContainmentMenu::PrintPoint(const NamedVector2& point)
{
	printf("{%.*f,%.*f} ",
		Math::GetRecommendedPrecisionOfFloat(static_cast<float>(point.X())), point.X(),
		Math::GetRecommendedPrecisionOfFloat(static_cast<float>(point.Y())), point.Y());
}

void SquareContainmentMenu::PrintPointPair(const NamedVector2& pointA, const NamedVector2& pointB)
{
	PrintPoint(pointA);
	printf("  ");
	PrintPoint(pointB);
}

void SquareContainmentMenu::PrintSpecificSetOfPoints(const std::vector<NamedVector2>& points, bool includeIndexes, const NamedVector2* optionalOffset /*= nullptr*/, bool includeNames /*= false*/)
{
	for (size_t index = 0; index < points.size(); ++index)
	{
		NamedVector2 point = points.at(index);
		if (optionalOffset)
		{
			point.IncreaseButRetainNames(*optionalOffset);
		}
		if (includeIndexes)
		{
			printf(" %i", static_cast<int32_t>(index));
			if (includeNames && point.Name().size() > 0)
			{
				printf(",%s", point.Name().c_str());
			}
			printf(": ");
		}
		else if (includeNames && point.Name().size() > 0)
		{
			printf(" %s: ", point.Name().c_str());
		}
		PrintPoint(point);
	}
}

void SquareContainmentMenu::PrintSpecificSetOfPointsWithNames(const std::vector<NamedVector2>& points)
{
	for (const NamedVector2& point : points)
	{
		printf(" %s: ", point.Name().c_str());
		PrintPoint(point);
	}
}

void SquareContainmentMenu::PrintPoints()
{
	printf("\n\nCurrent Points");
	if (gGlobalData.GetActivePointsName().size() > 0)
	{
		printf(" (%s)", gGlobalData.GetActivePointsName().c_str());
	}
	printf(":\n");
	PrintSpecificSetOfPoints(gGlobalData.GetActivePoints(), true, nullptr, true);
	printf("\n");
}

void SquareContainmentMenu::ReloadPredefinedSets()
{
	LoadGlobalData();
	PrintPoints();
}

void SquareContainmentMenu::PreOpenMenu()
{
	if (gGlobalData.GetActivePoints().size() > 0)
	{
		PrintPoints();
	}
}

void SquareContainmentMenu::AddPoint(uint64_t x, uint64_t y)
{
	gGlobalData.AddActivePoint((double)x, (double)y);
	PrintPoints();
}

void SquareContainmentMenu::RemovePointByIndex(uint64_t index)
{
	gGlobalData.RemovePointByIndex(index);
	PrintPoints();
}

void SquareContainmentMenu::ClearAllPoints(uint64_t confirmation)
{
	if (confirmation == 101)
	{
		gGlobalData.ClearAllPoints();
	}
	PrintPoints();
}

void SquareContainmentMenu::AddLineOfPoints(const char* const points)
{
	// Example:  C: {10050,10050}  T: {0,10050}  PV1: {0,9050}  PV2: {0,6430}  PV3: {0,5860}
	// Or: {10050,10050} {0,10050} {0,9050} {0,6430} {0,5860}
	const std::string pointsStr(points);

	for (size_t leftCurlyPos = pointsStr.find('{'); leftCurlyPos != std::string::npos; leftCurlyPos = pointsStr.find('{', leftCurlyPos + 1))
	{
		std::istringstream iss(pointsStr.substr(leftCurlyPos + 1));
		int32_t x, y;
		char delim;
		if ((iss >> x >> delim >> y) && (delim == ','))
		{
			size_t colonPos = leftCurlyPos;
			while (colonPos > 0 && pointsStr[colonPos] != '}' && pointsStr[colonPos] != ':')
			{
				--colonPos;
			}

			if (colonPos > 0 && pointsStr[colonPos] == ':')
			{
				size_t nameStartPos = colonPos - 1;
				while (nameStartPos > 0 && pointsStr[nameStartPos - 1] != ' ' && pointsStr[nameStartPos - 1] != '}' && pointsStr[nameStartPos - 1] != ',')
				{
					--nameStartPos;
				}

				const std::string extractedName = pointsStr.substr(nameStartPos, colonPos - nameStartPos);
				std::string name;
				for (char c : extractedName)
				{
					if (c >= 'a' && c <= 'z')
					{
						// uppercase
						name.push_back(c - 0x20);
					}
					else
					{
						name.push_back(c);
					}
				}

				gGlobalData.AddNamedActivePoint(name, (double)x, (double)y);
			}
			else
			{
				gGlobalData.AddActivePoint((double)x, (double)y);
			}
		}
	}
	PrintPoints();
}

void SquareContainmentMenu::ClearAndAddLineOfPoints(const char* const points)
{
	gGlobalData.ClearAllPoints();
	AddLineOfPoints(points);
}

void SquareContainmentMenu::ClearAndAddNamedLineOfPoints(const char* const points)
{
	gGlobalData.ClearAllPoints();
	const std::string tokenizedNameStr = points;
	gGlobalData.AddActivePointsByNamedSettedPoints(tokenizedNameStr);
	PrintPoints();
}

void SquareContainmentMenu::SetPointsToPredefinedSet_PreArgs()
{
	printf("\nSelect a Set:\n");
	for (size_t index = 0; index < gGlobalData.GetPredefinedSets().size(); ++index)
	{
		printf("%i: %s : ", static_cast<int32_t>(index), gGlobalData.GetPredefinedSets()[index].mName.c_str());
		PrintSpecificSetOfPoints(gGlobalData.GetPredefinedSets()[index].mPoints, false);
		printf("\n");
	}
}

void SquareContainmentMenu::SetPointsToPredefinedSet(uint64_t setIndex)
{
	gGlobalData.SetActivePointsToPredefinedSet(setIndex);
	PrintPoints();
}

void SquareContainmentMenu::Callback_PostRotate(const std::vector<NamedVector2>& rotatingHull, double accumulatedRotation, const NamedVector2& xExtremesVec, const NamedVector2& yExtremesVec)
{
	printf("\n\nRotatingHull Hull (post %f rads, %f deg):\n", accumulatedRotation, Math::RadiansToDegrees(accumulatedRotation));
	PrintSpecificSetOfPoints(rotatingHull, true);
	printf("\n X Extremes: ");
	PrintPoint(xExtremesVec);
	printf("   Y Extremes: ");
	PrintPoint(yExtremesVec);
	printf("\n");
}

void SquareContainmentMenu::TestCurrentSetOfPoints(uint64_t squareSideLength)
{
	PrintPoints();
	SquareContainment squareContainment(gGlobalData.GetActivePoints());

	const std::vector<NamedVector2>& convexHull = squareContainment.GetConvexHull();
	printf("\n\nConvex Hull:\n");
	PrintSpecificSetOfPoints(convexHull, true, &squareContainment.GetOriginOffset());

	std::string resultContext;
	const SquareContainmentResult result = squareContainment.Test(static_cast<double>(squareSideLength), &resultContext, Callback_PostRotate);

	const std::string resultName = ToString(result);
	printf("\n\n%s\n", resultName.c_str());

	if (result < SquareContainmentResult::kBELOWFits_ABOVEFails)
	{
		printf("%s\n", resultContext.c_str());
	}
}

void SquareContainmentMenu::TestCurrentSetOfPoints_Forced10000()
{
	TestCurrentSetOfPoints((uint64_t)SquareContainment::kDefaultSideLength);
}

void SquareContainmentMenu::SeekingTest_InternalMeasure(
	const NamedVector2& safePoint, double xExponentDirection, double yExponentDirection,
	double squareSideLength, std::vector<NamedVector2>& testPoints, NamedVector2& outFinalFitsPoint, NamedVector2& outFinalFailPoint)
{
	const double diagonalLength = std::sqrt(squareSideLength * squareSideLength + squareSideLength * squareSideLength);
	double accumulatedValue = 0.0;

	double exponent = 1.0;
	double incrementBase = diagonalLength / 2.0;
	double increment = std::ceil(incrementBase);
	do
	{
		testPoints.back().AssignButRetainName(
			safePoint.X() + (accumulatedValue + increment) * xExponentDirection,
			safePoint.Y() + (accumulatedValue + increment) * yExponentDirection
		);

		if (SquareContainment::SimpleTest(testPoints, squareSideLength))
		{
			accumulatedValue += increment;
		}

		exponent += 1.0;
		incrementBase = diagonalLength / std::pow(2.0, exponent);
		increment = std::ceil(incrementBase);

	} while (incrementBase > (0.5 - Math::kEpsilon));


	testPoints.back().AssignButRetainName(
		safePoint.X() + accumulatedValue * xExponentDirection,
		safePoint.Y() + accumulatedValue * yExponentDirection
	);
	outFinalFitsPoint.AssignButRetainName(testPoints.back());
	outFinalFailPoint.AssignButRetainName(testPoints.back().X() + xExponentDirection, testPoints.back().Y() + yExponentDirection);
}

void SquareContainmentMenu::TwoPointSeekingTest_InternalMeasure(
	const NamedVector2& safePointX, double xExponentDirectionX, double yExponentDirectionX,
	const NamedVector2& safePointY, double xExponentDirectionY, double yExponentDirectionY,
	double squareSideLength, std::vector<NamedVector2>& testPoints, NamedVector2& outFinalFitsPointX, NamedVector2& outFinalFailPointX, NamedVector2& outFinalFitsPointY, NamedVector2& outFinalFailPointY)
{
	const double diagonalLength = std::sqrt(squareSideLength * squareSideLength + squareSideLength * squareSideLength);
	double accumulatedValue = 0.0;

	double exponent = 1.0;
	double incrementBase = diagonalLength / 2.0;
	double increment = std::ceil(incrementBase);
	do
	{
		testPoints.at(testPoints.size() - 2).AssignButRetainName( // X
			safePointX.X() + (accumulatedValue + increment) * xExponentDirectionX,
			safePointX.Y() + (accumulatedValue + increment) * yExponentDirectionX
		);
		testPoints.back().AssignButRetainName( // Y
			safePointY.X() + (accumulatedValue + increment) * xExponentDirectionY,
			safePointY.Y() + (accumulatedValue + increment) * yExponentDirectionY
		);

		if (SquareContainment::SimpleTest(testPoints, squareSideLength))
		{
			accumulatedValue += increment;
		}

		exponent += 1.0;
		incrementBase = diagonalLength / std::pow(2.0, exponent);
		increment = std::ceil(incrementBase);

	} while (incrementBase > (0.5 - Math::kEpsilon));


	testPoints.at(testPoints.size() - 2).AssignButRetainName( // X
		safePointX.X() + accumulatedValue * xExponentDirectionX,
		safePointX.Y() + accumulatedValue * yExponentDirectionX
	);
	testPoints.back().AssignButRetainName( // Y
		safePointY.X() + accumulatedValue * xExponentDirectionY,
		safePointY.Y() + accumulatedValue * yExponentDirectionY
	);
	outFinalFitsPointX.AssignButRetainName(testPoints.at(testPoints.size() - 2));
	outFinalFailPointX.AssignButRetainName(testPoints.at(testPoints.size() - 2).X() + xExponentDirectionX, testPoints.at(testPoints.size() - 2).Y() + yExponentDirectionX);
	outFinalFitsPointY.AssignButRetainName(testPoints.back());
	outFinalFailPointY.AssignButRetainName(testPoints.back().X() + xExponentDirectionY, testPoints.back().Y() + yExponentDirectionY);
}

void SquareContainmentMenu::SeekingTest_InternalFullTest(double squareSideLength, uint64_t index, double xMult, double yMult)
{
	PrintPoints();
	if (!SquareContainment::SimpleTest(gGlobalData.GetActivePoints(), squareSideLength))
	{
		printf("\nCurrent set of points always fails without seeking\n");
		return;
	}

	if (index >= gGlobalData.GetActivePoints().size())
	{
		printf("\nIndex isn't valid\n");
		return;
	}

	const NamedVector2& safePoint = gGlobalData.GetActivePoints().at(index);

	std::vector<NamedVector2> testPoints = gGlobalData.GetActivePoints();
	testPoints.emplace_back(safePoint);

	NamedVector2 finalFitsPoint;
	NamedVector2 finalFailPoint;

	SeekingTest_InternalMeasure(safePoint, 1.0 * xMult, 1.0 * yMult, squareSideLength, testPoints, finalFitsPoint, finalFailPoint);
	printf("\nFail Point: ");
	PrintPoint(finalFailPoint);

	printf("\n\nFits Point: ");
	PrintPoint(finalFitsPoint);
	printf("\nStarting Point: ");
	PrintPoint(safePoint);
	SeekingTest_InternalMeasure(safePoint, -1.0 * xMult, -1.0 * yMult, squareSideLength, testPoints, finalFitsPoint, finalFailPoint);
	printf("\nFits Point: ");
	PrintPoint(finalFitsPoint);

	printf("\n\nFail Point: ");
	PrintPoint(finalFailPoint);
}

void SquareContainmentMenu::TwoPointSeekingTest_InternalFullTest(double squareSideLength, uint64_t indexX, double xMultX, double yMultX, uint64_t indexY, double xMultY, double yMultY)
{
	PrintPoints();
	if (!SquareContainment::SimpleTest(gGlobalData.GetActivePoints(), squareSideLength))
	{
		printf("\nCurrent set of points always fails without seeking\n");
		return;
	}

	if (indexX >= gGlobalData.GetActivePoints().size())
	{
		printf("\nIndex X isn't valid\n");
		return;
	}

	if (indexY >= gGlobalData.GetActivePoints().size())
	{
		printf("\nIndex Y isn't valid\n");
		return;
	}

	const NamedVector2& safePointX = gGlobalData.GetActivePoints().at(indexX);
	const NamedVector2& safePointY = gGlobalData.GetActivePoints().at(indexY);

	std::vector<NamedVector2> testPoints = gGlobalData.GetActivePoints();
	testPoints.emplace_back(safePointX);
	testPoints.emplace_back(safePointY);

	NamedVector2 finalFitsPointX;
	NamedVector2 finalFailPointX;
	NamedVector2 finalFitsPointY;
	NamedVector2 finalFailPointY;

	TwoPointSeekingTest_InternalMeasure(
		safePointX, 1.0 * xMultX, 1.0 * yMultX,
		safePointY, 1.0 * xMultY, 1.0 * yMultY,
		squareSideLength, testPoints, finalFitsPointX, finalFailPointX, finalFitsPointY, finalFailPointY);
	printf("\nFail Points: ");
	PrintPointPair(finalFailPointX, finalFailPointY);

	printf("\n\nFits Points: ");
	PrintPointPair(finalFitsPointX, finalFitsPointY);
	printf("\nStarting Points: ");
	PrintPointPair(safePointX, safePointY);
	TwoPointSeekingTest_InternalMeasure(
		safePointX, -1.0 * xMultX, -1.0 * yMultX,
		safePointY, -1.0 * xMultY, -1.0 * yMultY,
		squareSideLength, testPoints, finalFitsPointX, finalFailPointX, finalFitsPointY, finalFailPointY);
	printf("\nFits Points: ");
	PrintPointPair(finalFitsPointX, finalFitsPointY);

	printf("\n\nFail Points: ");
	PrintPointPair(finalFailPointX, finalFailPointY);
}

void SquareContainmentMenu::SeekingTestX_Forced10000(uint64_t index)
{
	SeekingTest_InternalFullTest(SquareContainment::kDefaultSideLength, index, 1.0, 0.0);
}

void SquareContainmentMenu::SeekingTestY_Forced10000(uint64_t index)
{
	SeekingTest_InternalFullTest(SquareContainment::kDefaultSideLength, index, 0.0, 1.0);
}

void SquareContainmentMenu::SeekingTestXY_Forced10000(uint64_t index)
{
	SeekingTest_InternalFullTest(SquareContainment::kDefaultSideLength, index, 1.0, 1.0);
}

void SquareContainmentMenu::TwoPointSeekingTestXY_Forced10000(uint64_t indexX, uint64_t indexY)
{
	TwoPointSeekingTest_InternalFullTest(SquareContainment::kDefaultSideLength, indexX, 1.0, 0.0, indexY, 0.0, 1.0);
}

void SquareContainmentMenu::Analyze_PointExclusions()
{
	MaxInclusions::BuildAndPrintInclusions({ SetType::K }, 1);

	MaxInclusions::BuildAndPrintInclusions({ SetType::K, SetType::T }, 1);
	MaxInclusions::BuildAndPrintInclusions({ SetType::K, SetType::D }, 1);
	MaxInclusions::BuildAndPrintInclusions({ SetType::K, SetType::T, SetType::D }, 1);

	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "I1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "I2" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "I3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "I4" });

	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "I1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "I2" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "I3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "I4" });

	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "D", "I1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "D", "I2" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "D", "I3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "D", "I4" });

	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "D", "I1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "D", "I2" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "D", "I3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "K", "T", "D", "I4" });


	MaxInclusions::BuildAndPrintInclusions({ SetType::C }, 1);
	MaxInclusions::BuildAndPrintInclusions({ SetType::C, SetType::T }, 1);
	MaxInclusions::BuildAndPrintInclusions({ SetType::C, SetType::T, SetType::D }, 1);
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "D_2" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "D_2", "T" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PH1_4" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PV1_4" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PH1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PV1" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PH3_4" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PV3_4" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PH3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "D", "T", "PV3" });
	MaxInclusions::BuildByNameAndPrintInclusions({ "C", "O", "O_2" });

	MaxInclusions::BuildAndPrintInclusions({ SetType::D }, 1);
}

void SquareContainmentMenu::Analyze_CheckExpectedFails()
{
	int32_t passed = 0;
	int32_t total = 0;

	const std::vector<AssertionData>& assertions = gGlobalData.GetAssertions();
	for (const AssertionData& assertion : assertions)
	{
		switch (assertion.mFunc)
		{
		case AssertionFunction::Fail:
		{
			AssertFail(passed, total, assertion.mPointNames);
		} break;

		case AssertionFunction::MaxCountOfSet:
		{
			if (assertion.mCount.size() > 0 && assertion.mSetTypes.size() > 0)
			{
				AssertMaxCountOfSet(passed, total, assertion.mPointNames, assertion.mSetTypes.at(0), assertion.mCount.at(0));
			}
		} break;

		default: break;
		}
	}
	printf("\n%i / %i tests succeeded", passed, total);
}

void SquareContainmentMenu::Analyze_PrintAllPoints()
{
	std::vector<NamedVector2> allSettedPoints;
	for (auto& settedPointPair : gGlobalData.GetSettedPoints())
	{
		allSettedPoints.emplace_back(settedPointPair.second.mPoint);
	}
	PrintSpecificSetOfPointsWithNames(allSettedPoints);
}

void SquareContainmentMenu::AssertFail(int32_t& passedTracker, int32_t& totalTracker, std::vector<std::string> names)
{
	totalTracker++;
	std::vector<NamedVector2> points;
	for (const std::string& name : names)
	{
		auto iter = gGlobalData.GetSettedPoints().find(name);
		if (iter != gGlobalData.GetSettedPoints().end())
		{
			points.emplace_back((*iter).second.mPoint, name);
		}
	}
	const bool fits = SquareContainment::SimpleTest(points, SquareContainment::kDefaultSideLength);
	if (fits)
	{
		printf("\n[%i] Expected to fail, but got fit!\n", totalTracker);
		PrintSpecificSetOfPointsWithNames(points);
	}
	else
	{
		passedTracker++;
	}
}

void SquareContainmentMenu::AssertMaxCountOfSet(int32_t& passedTracker, int32_t& totalTracker, std::vector<std::string> names, SetType ofSet, int32_t maxCount)
{
	totalTracker++;
	MaxInclusions inclusions;
	std::vector<NamedVector2> fixedPoints;
	std::vector<NamedVector2> examplePoints;
	AddPointsFromSettedPointsByName(names, fixedPoints);
	const int32_t actualCount = inclusions.GetMax(fixedPoints, ofSet, 4, examplePoints);

	if (actualCount != maxCount)
	{
		printf("\n[%i] Expected equal to %i points of set %s, but got %i points instead!\n", totalTracker, maxCount, ToString(ofSet).c_str(), actualCount);

		if (examplePoints.size() > 0)
		{
			printf("Example: ");
			PrintSpecificSetOfPointsWithNames(examplePoints);
		}
		else
		{
			printf("Original Points: ");
			PrintSpecificSetOfPointsWithNames(fixedPoints);
		}
		printf("\n");
	}
	else
	{
		passedTracker++;
	}
}

SquareContainmentMenu::SettedPoint::SettedPoint(uint64_t x, uint64_t y, const std::string& setNames)
: mPoint(x, y)
{
	mSection = 1;
	for (SetType setType = SetType::C; setType < SetType::kCount; ++setType)
	{
		const std::string& setTypeString = ToString(setType);
		if (setNames.find(setTypeString) != std::string::npos)
		{
			mSets.emplace_back(setType);

			if (setType == SetType::C)
			{
				mSection = 0;
			}
		}
	}

	for (char c : setNames)
	{
		if ((c >= '0') == (c <= '9'))
		{
			mIndex *= 10;
			mIndex += (int32_t)(c - '0');
		}
	}
}

bool SquareContainmentMenu::SettedPoint::IsPartOfSet(SetType setType) const
{
	return std::find(mSets.begin(), mSets.end(), setType) != mSets.end();
}

SquareContainmentMenu::SettedPoint::SettedPoint(const SettedPoint& set1Version, int32_t newSection)
: mPoint(set1Version.mPoint)
, mIndex(set1Version.mIndex)
, mSets(set1Version.mSets)
, mSection(set1Version.mSection > 0 ? newSection : 0)
{
}
