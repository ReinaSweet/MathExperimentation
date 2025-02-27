#pragma once
#include "stdafx.h"
#include "SquareContainmentGlobalData.h"

class ConsoleMenu;
class SquareContainment;

namespace SquareContainmentMenu
{
	void SetupMenu(ConsoleMenu& inMenu, ConsoleMenu& analysisMenu);
	void AddPointsFromSettedPointsSpcIndex(SetType setType, int32_t index, int32_t numSections, std::vector<NamedVector2>& inOutPoints);
	void AddPointsFromSettedPoints(SetType setType, int32_t numSections, std::vector<NamedVector2>& inOutPoints);
	void AddPointsFromSettedPointsByName(std::vector<std::string> names, std::vector<NamedVector2>& inOutPoints);

	void LoadGlobalData();
	void PrintPoint(const NamedVector2& point);
	void PrintPointPair(const NamedVector2& pointA, const NamedVector2& pointB);
	void PrintSpecificSetOfPoints(const std::vector<NamedVector2>& points, bool includeIndexes, const NamedVector2* optionalOffset = nullptr, bool includeNames = false);
	void PrintSpecificSetOfPointsWithNames(const std::vector<NamedVector2>& points);
	void PrintPoints();

	void ReloadPredefinedSets();
	void PreOpenMenu();


	void AddPoint(uint64_t x, uint64_t y);
	void RemovePointByIndex(uint64_t index);
	void ClearAllPoints(uint64_t confirmation);
	void AddLineOfPoints(const char* const points);
	void ClearAndAddLineOfPoints(const char* const points);
	void ClearAndAddNamedLineOfPoints(const char* const points);

	void SetPointsToPredefinedSet_PreArgs();
	void SetPointsToPredefinedSet(uint64_t setIndex);

	void Callback_PostRotate(const std::vector<NamedVector2>& rotatingHull, double accumulatedRotation, const NamedVector2& xExtremesVec, const NamedVector2& yExtremesVec);
	void TestCurrentSetOfPoints(uint64_t squareSideLength);
	void TestCurrentSetOfPoints_Forced10000();

	void SeekingTest_InternalMeasure(
		const NamedVector2& safePoint, double xExponentDirection, double yExponentDirection,
		double squareSideLength, std::vector<NamedVector2>& testPoints, NamedVector2& outFinalFitsPoint, NamedVector2& outFinalFailPoint);
	void TwoPointSeekingTest_InternalMeasure(
		const NamedVector2& safePointX, double xExponentDirectionX, double yExponentDirectionX,
		const NamedVector2& safePointY, double xExponentDirectionY, double yExponentDirectionY,
		double squareSideLength, std::vector<NamedVector2>& testPoints, NamedVector2& outFinalFitsPointX, NamedVector2& outFinalFailPointX, NamedVector2& outFinalFitsPointY, NamedVector2& outFinalFailPointY);

	void SeekingTest_InternalFullTest(double squareSideLength, uint64_t index, double xMult, double yMult);
	void TwoPointSeekingTest_InternalFullTest(double squareSideLength, uint64_t indexX, double xMultX, double yMultX, uint64_t indexY, double xMultY, double yMultY);

	void SeekingTestX_Forced10000(uint64_t index);
	void SeekingTestY_Forced10000(uint64_t index);
	void SeekingTestXY_Forced10000(uint64_t index);
	void TwoPointSeekingTestXY_Forced10000(uint64_t indexX, uint64_t indexY);

	void Analyze_PointExclusions();
	void Analyze_CheckExpectedFails();
	void Analyze_PrintAllPoints();

	void AssertFail(int32_t& passedTracker, int32_t& totalTracker, std::vector<std::string> names);
	void AssertMaxCountOfSet(int32_t& passedTracker, int32_t& totalTracker, std::vector<std::string> names, SetType ofSet, int32_t maxCount);

	struct MaxInclusions
	{
		int32_t T = 0;
		int32_t D = 0;
		int32_t I = 0;
		int32_t P = 0;
		int32_t Z = 0;

		std::vector<NamedVector2> mExampleMaxT;
		std::vector<NamedVector2> mExampleMaxD;
		std::vector<NamedVector2> mExampleMaxI;
		std::vector<NamedVector2> mExampleMaxP;
		std::vector<NamedVector2> mExampleMaxZ;

		int32_t GetMax(const std::vector<NamedVector2>& fixedPoints, SetType ofSet, int32_t maxSections, std::vector<NamedVector2>& outLargestSetOfPoints);
		void FillAllMax(const std::vector<NamedVector2>& fixedPoints);
		void FillAllMaxWithFixedSet(SetType fixedSet, int32_t fixedSetMaxSections);

		void PrintMaxes();

		static void BuildAndPrintInclusions(std::vector<SetType> fixedSets, int32_t fixedSetMaxSections);
		static void BuildByNameAndPrintInclusions(std::vector<std::string> names);
	};
}