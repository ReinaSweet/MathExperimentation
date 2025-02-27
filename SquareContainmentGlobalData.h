#pragma once
#include "stdafx.h"
#include "NamedVector2.h"
#include "MathCommon.h"

namespace SquareContainmentMenu
{
enum class SetType : uint8_t
{
	C, // Center
	K, // K-Corner
	P, // Perimeter (non-T, non-K)
	H, // Horizontal Perimeter
	V, // Vertical Perimeter
	T, // T Edge
	O, // Outer containment
	D, // Diamond (center square containment points)
	I, // Inner (non-Center, non-Diamond)
	Z, // Would be inner points, but are 10 away from C
	U, // Underside (extra Set)
	S, // Something (extra Set)

	kCount
};
ENUM_OPS(SetType);
ENUM_STRING_CONVERT_DECLARE(SetType);

enum class DataLoadReadMode : uint8_t
{
	None,
	Config,
	SettedPoint,
	Assertions,
	PredefinedSets,

	kCount
};
ENUM_OPS(DataLoadReadMode);
ENUM_STRING_CONVERT_DECLARE(DataLoadReadMode);

enum class AssertionFunction : uint8_t
{
	Fail,
	MaxCountOfSet,

	kCount
};
ENUM_OPS(AssertionFunction);
ENUM_STRING_CONVERT_DECLARE(AssertionFunction);

struct NamedSet
{
	std::string mName;
	std::vector<NamedVector2> mPoints;
};

struct SettedPoint
{
	SettedPoint(uint64_t x, uint64_t y, const std::string& setNames);
	SettedPoint(const SettedPoint& set1Version, int32_t newSection);

	bool IsPartOfSet(SetType setType) const;

	NamedVector2 mPoint;
	std::vector<SetType> mSets;
	int32_t mIndex = 0;
	int32_t mSection = 0;
};

struct AssertionData
{
	AssertionFunction mFunc = AssertionFunction::kCount;

	// Function Specific Data
	// Each function is allowed to use each data point as it sees fit
	std::vector<std::string> mPointNames;
	std::vector<SetType> mSetTypes;
	std::vector<int32_t> mCount;

	void SetFromLine(const std::string& line);
};

class GlobalData
{
public:
	GlobalData(){}

	void LoadData();

	void AddActivePoint(double x, double y);
	void AddNamedActivePoint(const std::string& name, double x, double y);
	void AddActivePointsByNamedSettedPoints(const std::vector<std::string>& names);
	void AddActivePointsByNamedSettedPoints(const std::string& tokenizedNameStr);
	void RemovePointByIndex(size_t index);
	void ClearAllPoints();
	void SetActivePointsToPredefinedSet(size_t setIndex);
	
	void FillListWithSettedPoints(const std::vector<std::string>& names, std::vector<NamedVector2>& inOutPoints) const;
	void FillListWithSettedPoints(const std::string& tokenizedNameStr, std::vector<NamedVector2>& inOutPoints) const;

	const std::vector<NamedVector2>& GetActivePoints() const { return mActivePoints; }
	const std::string& GetActivePointsName() const { return mActivePointsName; }
	const std::vector<NamedSet>& GetPredefinedSets() const { return mPredefinedSets; }
	const std::map<std::string, SettedPoint>& GetSettedPoints() const { return mSettedPoints; }
	const std::vector<AssertionData>& GetAssertions() const { return mAssertions; }
private:
	void ReadConfigLine(const std::string& line);
	void PostProcessConfig_Early();
	void PostProcessConfig_Late();
	void ReadSettedPointLine(const std::string& line);
	void PostProcessSettedPoints();
	void ReadAssertionLine(const std::string& line);
	void PostProcessAssertions();
	void ReadPredefinedSetsLine(const std::string& line, NamedSet*& activeSet);
	void PostProcessPredefinedSets();

	DataLoadReadMode mCurrentReadMode = DataLoadReadMode::None;

	std::vector<std::pair<std::string, std::string>> mSettings;
	std::vector<NamedVector2> mActivePoints;
	std::string mActivePointsName;
	std::vector<NamedSet> mPredefinedSets; // Data/SquareContainmentSets.txt
	std::map<std::string, SettedPoint> mSettedPoints;
	std::vector<AssertionData> mAssertions;
};
}