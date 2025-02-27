#include "SquareContainmentGlobalData.h"

#include "SquareContainment.h"

#include <fstream>
#include <sstream>
#include <format>

namespace SquareContainmentMenu
{
static const std::string kSquareContainmentSetNames[] =
{
	"C",
	"K",
	"P",
	"H",
	"V",
	"T",
	"O",
	"D",
	"I",
	"Z",
	"U",
	"S"
};
ENUM_STRING_CONVERT_DEFINE(SetType, kCount, kSquareContainmentSetNames);

static const std::string kSquareContainmentDataLoadReadModeNames[] =
{
	"None",
	"Config",
	"SettedPoint",
	"Assertions",
	"PredefinedSets"
};
ENUM_STRING_CONVERT_DEFINE(DataLoadReadMode, kCount, kSquareContainmentDataLoadReadModeNames);

static const std::string kSquareContainmentAssertionFunctionNames[] =
{
	"Fail",
	"MaxCountOfSet"
};
ENUM_STRING_CONVERT_DEFINE(AssertionFunction, kCount, kSquareContainmentAssertionFunctionNames);

void AssertionData::SetFromLine(const std::string& line)
{
	// Examples:
	// Fail:K,T,D,PH3,PV1_4
	// MaxCountOfSet:C,O,O_2;@P;0

	const size_t colonPos = line.find(":");
	if (colonPos != std::string::npos)
	{
		mFunc = ToEnum<AssertionFunction>(line.substr(0, colonPos));
		if (mFunc != AssertionFunction::kCount)
		{
			size_t paramStartPos = colonPos + 1;
			size_t paramEndPos;
			std::string param;
			bool isFirstParam = true;
			do 
			{
				paramEndPos = line.find(";", paramStartPos);
				param = line.substr(paramStartPos, paramEndPos - paramStartPos);

				// First param is always a list of point names
				if (isFirstParam)
				{
					isFirstParam = false;
					size_t pointNameStartPos = 0;
					size_t pointNameEndPos;
					do 
					{
						pointNameEndPos = param.find(",", pointNameStartPos);
						mPointNames.emplace_back(param.substr(pointNameStartPos, pointNameEndPos - pointNameStartPos));
						pointNameStartPos = pointNameEndPos + 1;

					} while (pointNameEndPos != std::string::npos);
				}
				// SetType
				else if (param.at(0) == '@')
				{
					const SetType setType = ToEnum<SetType>(param.substr(1));
					if (setType != SetType::kCount)
					{
						mSetTypes.push_back(setType);
					}
				}
				// Count
				else
				{
					mCount.push_back(std::atoi(param.c_str()));
				}
				paramStartPos = paramEndPos + 1;

			} while (paramEndPos != std::string::npos);
		}
	}
}
}

void SquareContainmentMenu::GlobalData::LoadData()
{
	std::ifstream setsFile("Data/SquareContainmentSets.txt");
	if (setsFile.is_open())
	{
		mCurrentReadMode = DataLoadReadMode::None;
		mSettings.clear();
		mPredefinedSets.clear();
		mSettedPoints.clear();
		mAssertions.clear();

		NamedSet* activeSet = nullptr;

		std::string line;
		while (std::getline(setsFile, line))
		{
			if (line.size() > 1)
			{
				if (line.at(0) == '#')
				{
					const DataLoadReadMode newReadMode = ToEnum<DataLoadReadMode>(line.substr(1));
					if (newReadMode != DataLoadReadMode::kCount)
					{
						mCurrentReadMode = newReadMode;
					}
				}
				else
				{
					switch (mCurrentReadMode)
					{
					case DataLoadReadMode::Config: ReadConfigLine(line); break;
					case DataLoadReadMode::SettedPoint: ReadSettedPointLine(line); break;
					case DataLoadReadMode::Assertions: ReadAssertionLine(line); break;
					case DataLoadReadMode::PredefinedSets: ReadPredefinedSetsLine(line, activeSet); break;
					default: break;
					}
				}
			}
		}

		PostProcessConfig_Early();
		PostProcessSettedPoints();
		PostProcessAssertions();
		PostProcessPredefinedSets();
		PostProcessConfig_Late();
	}
}

void SquareContainmentMenu::GlobalData::AddActivePoint(double x, double y)
{
	mActivePointsName.clear();
	mActivePoints.emplace_back(x, y);
}

void SquareContainmentMenu::GlobalData::AddNamedActivePoint(const std::string& name, double x, double y)
{
	mActivePointsName.clear();
	mActivePoints.emplace_back(x, y, name.c_str());
}

void SquareContainmentMenu::GlobalData::AddActivePointsByNamedSettedPoints(const std::vector<std::string>& names)
{
	mActivePointsName.clear();
	FillListWithSettedPoints(names, mActivePoints);
}

void SquareContainmentMenu::GlobalData::AddActivePointsByNamedSettedPoints(const std::string& tokenizedNameStr)
{
	mActivePointsName.clear();
	FillListWithSettedPoints(tokenizedNameStr, mActivePoints);
}

void SquareContainmentMenu::GlobalData::RemovePointByIndex(size_t index)
{
	if (index < mActivePoints.size())
	{
		mActivePointsName.clear();
		mActivePoints.erase(mActivePoints.begin() + index);
	}
}

void SquareContainmentMenu::GlobalData::ClearAllPoints()
{
	mActivePointsName.clear();
	mActivePoints.clear();
}

void SquareContainmentMenu::GlobalData::SetActivePointsToPredefinedSet(size_t setIndex)
{
	if (setIndex < mPredefinedSets.size())
	{
		mActivePoints = mPredefinedSets[setIndex].mPoints;
		mActivePointsName = mPredefinedSets[setIndex].mName;
	}
}

void SquareContainmentMenu::GlobalData::FillListWithSettedPoints(const std::vector<std::string>& names, std::vector<NamedVector2>& inOutPoints) const
{
	for (const std::string& name : names)
	{
		auto iter = mSettedPoints.find(name);
		if (iter != mSettedPoints.end())
		{
			inOutPoints.emplace_back((*iter).second.mPoint, name);
		}
	}
}

void SquareContainmentMenu::GlobalData::FillListWithSettedPoints(const std::string& tokenizedNameStr, std::vector<NamedVector2>& inOutPoints) const
{
	std::vector<std::string> names;

	size_t namePos = 0;
	do 
	{
		std::string name;
		for (;namePos < tokenizedNameStr.size() && tokenizedNameStr[namePos] != ','; ++namePos)
		{
			if (tokenizedNameStr[namePos] == ' ')
			{ }
			else if (tokenizedNameStr[namePos] >= 'a' && tokenizedNameStr[namePos] <= 'z')
			{
				// uppercase
				name.push_back(tokenizedNameStr[namePos] - 0x20);
			}
			else
			{
				name.push_back(tokenizedNameStr[namePos]);
			}
		}

		names.emplace_back(name);

		++namePos;
	} while (namePos < tokenizedNameStr.size());

	FillListWithSettedPoints(names, inOutPoints);
}

void SquareContainmentMenu::GlobalData::ReadConfigLine(const std::string& line)
{
	const size_t colonPos = line.find(":");
	if (colonPos != std::string::npos)
	{
		mSettings.emplace_back(line.substr(0, colonPos), line.substr(colonPos + 1));
	}
}

void SquareContainmentMenu::GlobalData::PostProcessConfig_Early()
{
}

void SquareContainmentMenu::GlobalData::PostProcessConfig_Late()
{
	for (const std::pair<std::string, std::string>& settingPair : mSettings)
	{
		if (settingPair.first == "DefaultSet")
		{
			for (const NamedSet& namedSet : mPredefinedSets)
			{
				if (namedSet.mName == settingPair.second)
				{
					mActivePointsName = namedSet.mName;
					mActivePoints = namedSet.mPoints;
					break;
				}
			}
		}
	}
}

void SquareContainmentMenu::GlobalData::ReadSettedPointLine(const std::string& line)
{
	const size_t colonPos = line.find(":");
	if (colonPos != std::string::npos)
	{
		const std::string sets = line.substr(0, colonPos);
		const std::string number = line.substr(colonPos + 1);
		std::istringstream iss(number);
		int32_t x, y;
		char delim;
		if ((iss >> x >> delim >> y) && (delim == ','))
		{
			std::string name;

			const size_t plusPos = sets.find("+");
			if (plusPos != std::string::npos)
			{
				name = sets.substr(0, plusPos);
			}
			else
			{
				name = sets;
			}
			SettedPoint& newPoint = (*mSettedPoints.emplace(name, SettedPoint((uint64_t)x, (uint64_t)y, sets)).first).second;
			newPoint.mPoint.SetName(name);
		}
	}
}

void SquareContainmentMenu::GlobalData::PostProcessSettedPoints()
{

}

void SquareContainmentMenu::GlobalData::ReadAssertionLine(const std::string& line)
{
	mAssertions.emplace_back();
	mAssertions.back().SetFromLine(line);
	if (mAssertions.back().mFunc == AssertionFunction::kCount)
	{
		mAssertions.pop_back();
	}
}

void SquareContainmentMenu::GlobalData::PostProcessAssertions()
{

}

void SquareContainmentMenu::GlobalData::ReadPredefinedSetsLine(const std::string& line, NamedSet*& activeSet)
{
	if (line.at(0) == '[' && line.at(line.size() - 1) == ']')
	{
		mPredefinedSets.emplace_back();
		activeSet = &mPredefinedSets.back();
		activeSet->mName = line.substr(1, line.size() - 2);
	}
	else if (activeSet)
	{
		std::istringstream iss(line);
		int32_t x, y;
		char delim;
		if ((iss >> x >> delim >> y) && (delim == ','))
		{
			activeSet->mPoints.emplace_back(static_cast<double>(x), static_cast<double>(y));
		}
	}
}

void SquareContainmentMenu::GlobalData::PostProcessPredefinedSets()
{
	std::map<std::string, SettedPoint> copyOfSettedPoints;

	// Same as base section points, but Horizontal vs Vertical variations
	copyOfSettedPoints = mSettedPoints;
	for (const std::pair<std::string, SettedPoint>& settedPoint : copyOfSettedPoints)
	{
		if (settedPoint.second.IsPartOfSet(SetType::H))
		{
			const SettedPoint& oldPoint = settedPoint.second;
			std::string pointName;
			for (char c : settedPoint.first)
			{
				if (c == 'H')
				{
					pointName += 'V';
				}
				else
				{
					pointName += c;
				}
			}

			SettedPoint& newPoint = (*mSettedPoints.emplace(pointName, SettedPoint(oldPoint, 1)).first).second;
			std::replace(newPoint.mSets.begin(), newPoint.mSets.end(), SetType::H, SetType::V);
			newPoint.mPoint.SetName(pointName);
			const double oldX = newPoint.mPoint.X();
			const double oldY = newPoint.mPoint.Y();
			newPoint.mPoint.AssignButRetainName(oldY, oldX);
		}
	}

	// Point Rotations / slides into other sections
	copyOfSettedPoints = mSettedPoints;
	for (const std::pair<std::string, SettedPoint>& settedPoint : copyOfSettedPoints)
	{
		if (settedPoint.second.IsPartOfSet(SetType::C))
		{
			continue;
		}

		const bool bRotate = !((settedPoint.second.IsPartOfSet(SetType::H) || settedPoint.second.IsPartOfSet(SetType::V)));

		for (int32_t newSection = 2; newSection <= 4; ++newSection)
		{
			std::string pointName;
			std::format_to(std::back_inserter(pointName), "{}_{}", settedPoint.first, newSection);

			const SettedPoint& oldPoint = settedPoint.second;
			SettedPoint& newPoint = (*mSettedPoints.emplace(pointName, SettedPoint(oldPoint, newSection)).first).second;
			newPoint.mPoint.SetName(pointName);

			if (bRotate)
			{
				if (oldPoint.mSection > 0)
				{
					int32_t sectionDifference = newSection - oldPoint.mSection;
					while (sectionDifference > 0)
					{
						newPoint.mPoint.AssignButRetainName(SquareContainment::kFullSquareSideLength - newPoint.mPoint.Y(), newPoint.mPoint.X());
						--sectionDifference;
					}
				}
			}
			else // Slide, retaining horizontal and vertical forms
			{
				if (oldPoint.mSection == 1)
				{
					if (newSection > 2)
					{
						newPoint.mPoint.AssignButRetainName(newPoint.mPoint.X(), SquareContainment::kFullSquareSideLength - newPoint.mPoint.Y());
					}

					if (newSection < 4)
					{
						newPoint.mPoint.AssignButRetainName(SquareContainment::kFullSquareSideLength - newPoint.mPoint.X(), newPoint.mPoint.Y());
					}
				}
			}
		}
	}
}
