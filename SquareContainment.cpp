#include "SquareContainment.h"
#include "Math.h"
#include <format>

static const std::string kSquareContainmentResultNames[] =
{
	"SquareFitsSinglePoint",
	"SquareFitsLineLTEDiagonal",
	"SquareFitsSmallHull",
	"SquareFitsHull",

	"BELOWFits_ABOVEFails",

	"FailHullSegmentExceedsDiagonal",
	"FailHullDoesntFit"
};
ENUM_STRING_CONVERT_DEFINE(SquareContainmentResult, kCount, kSquareContainmentResultNames);

SquareContainment::SquareContainment(const std::vector<NamedVector2>& points)
	: mSmallestDistanceBetweenAnyPointSq(std::numeric_limits<double>::max())
{
	BuildConvexHull(points);
	ConvertConvexHullToRelativeToOrigin();
	MeasureExtremes();
}

// Graham Scan Algorithm
// Adapted from: https://www.geeksforgeeks.org/convex-hull-using-graham-scan/
void SquareContainment::BuildConvexHull(const std::vector<NamedVector2>& points)
{
	mConvexHull = points;
	if (points.size() < 3)
	{
		return;
	}

	// Step 1: Sort points
	size_t minIndex = 0;
	double yMin = points[minIndex].Y();

	for (size_t index = 0; index < points.size(); ++index)
	{
		const double y = points[index].Y();
		if ((y < yMin) ||
			(yMin == y && points[index].X() < points[minIndex].X()))
		{
			yMin = points[index].Y();
			minIndex = index;
		}
	}
	
	mConvexHull[0].Swap(mConvexHull[minIndex]);

	std::sort(mConvexHull.begin() + 1, mConvexHull.end(), [this](const NamedVector2& a, const NamedVector2& b) -> bool
		{
			const Math::AngularOrientation orientation = a.GetAngularOrientation(mConvexHull[0], b);
			if (orientation == Math::AngularOrientation::Collinear)
			{
				return mConvexHull[0].DistSq(b) > mConvexHull[0].DistSq(a);
			}
			return (orientation == Math::AngularOrientation::Counterclockwise);
		});

	size_t sizeOfNewConvexHull = 1;
	for (size_t index = 1; index < mConvexHull.size(); ++index)
	{
		while (index < (mConvexHull.size() - 1) &&
			mConvexHull[index].GetAngularOrientation(mConvexHull[0], mConvexHull[index + 1]) == Math::AngularOrientation::Collinear)
		{
			++index;
		}

		mConvexHull[sizeOfNewConvexHull].Assign(mConvexHull[index]);
		++sizeOfNewConvexHull;
	}

	mConvexHull.resize(sizeOfNewConvexHull);

	if (mConvexHull.size() < 3)
	{
		return;
	}

	// Step 2: Accept or deny points
	std::vector<NamedVector2> remainingAcceptedPoints;
	remainingAcceptedPoints.reserve(mConvexHull.size());
	remainingAcceptedPoints.push_back(mConvexHull[0]);
	remainingAcceptedPoints.push_back(mConvexHull[1]);
	remainingAcceptedPoints.push_back(mConvexHull[2]);

	for (size_t index = 3; index < sizeOfNewConvexHull; ++index)
	{
		while (remainingAcceptedPoints.size() > 1 &&
			remainingAcceptedPoints.back().GetAngularOrientation(remainingAcceptedPoints[remainingAcceptedPoints.size() - 2], mConvexHull[index]) != Math::AngularOrientation::Counterclockwise)
		{
			remainingAcceptedPoints.pop_back();
		}
		remainingAcceptedPoints.push_back(mConvexHull[index]);
	}

	mConvexHull = remainingAcceptedPoints;
}

void SquareContainment::ConvertConvexHullToRelativeToOrigin()
{
	if (mConvexHull.size() < 1)
	{
		return;
	}

	mOriginOffset = NamedVector2(mConvexHull[0], "Offset");
	for (NamedVector2& point : mConvexHull)
	{
		point.ReduceButRetainNames(mOriginOffset);
	}
}

void SquareContainment::MeasureExtremes()
{
	mSmallestDistanceBetweenAnyPointSq = std::numeric_limits<double>::max();
	mLargestDistanceBetweenAnyPointSq = 0.0;
	mExtremeIndexes.SetFromHull(mConvexHull);

	if (mConvexHull.size() < 2)
	{
		return;
	}

	for (size_t indexA = 0; indexA < (mConvexHull.size() - 1); ++indexA)
	{
		const NamedVector2& pA = mConvexHull[indexA];
		for (size_t indexB = (indexA + 1); indexB < mConvexHull.size(); ++indexB)
		{
			const NamedVector2& pB = mConvexHull[indexB];
			const double dist = pA.DistSq(pB);
			if (dist > mLargestDistanceBetweenAnyPointSq)
			{
				mLargestDistanceBetweenAnyPointSq = dist;
			}

			if (dist < mSmallestDistanceBetweenAnyPointSq)
			{
				mSmallestDistanceBetweenAnyPointSq = dist;
			}
		}
	}
}


SquareContainmentResult SquareContainment::Test(double squareSideLength, std::string& resultContext, FuncPtrRotatingHull postRotateCallback, Math::FittingTolerance fittingTolerance) const
{
	const double squareSideLengthSq = squareSideLength * squareSideLength;
	const double squareDiagonalLengthSq = squareSideLengthSq + squareSideLengthSq;

	if (mConvexHull.size() < 2)
	{
		return SquareContainmentResult::kSquareFitsSinglePoint;
	}

	if (mLargestDistanceBetweenAnyPointSq > squareDiagonalLengthSq)
	{
		return SquareContainmentResult::kFailHullSegmentExceedsDiagonal;
	}

	if (mConvexHull.size() < 3)
	{
		return SquareContainmentResult::kSquareFitsLineLTEDiagonal;
	}
	
	if (mLargestDistanceBetweenAnyPointSq <= squareSideLengthSq)
	{
		return SquareContainmentResult::kSquareFitsSmallHull;
	}

	RotatingHull rotatingHull(*this, mConvexHull, squareSideLength, fittingTolerance);
	
	bool hullFitsSquare = rotatingHull.FitsInSquare();
	while (!(hullFitsSquare || rotatingHull.ReachedMaxRotation()))
	{
		rotatingHull.RotateToNextSignificantAngle();
		if (postRotateCallback)
		{
			postRotateCallback(rotatingHull.GetInternalHull(), rotatingHull.GetAccumulatedRotation(), rotatingHull.GetCurrentXExtremesVec(), rotatingHull.GetCurrentYExtremesVec());
		}
		hullFitsSquare = rotatingHull.FitsInSquare();
	}

	if (hullFitsSquare)
	{
		std::format_to(std::back_inserter(resultContext), "Fits at {}deg. ", Math::RadiansToDegrees(rotatingHull.GetAccumulatedRotation()));
	}

	return hullFitsSquare ? SquareContainmentResult::kSquareFitsHull : SquareContainmentResult::kFailHullDoesntFit;
}


SquareContainmentResult SquareContainment::Test(double squareSideLength, std::string* optionalResultContext, FuncPtrRotatingHull postRotateCallback, Math::FittingTolerance fittingTolerance) const
{
	if (optionalResultContext)
	{
		return Test(squareSideLength, *optionalResultContext, postRotateCallback, fittingTolerance);
	}
	std::string placeholder;
	return Test(squareSideLength, placeholder, nullptr, fittingTolerance);
}

bool SquareContainment::PointIsWithinHull(NamedVector2 point) const
{
	if (mConvexHull.size() < 2)
	{
		return false;
	}

	point.ReduceButRetainNames(mOriginOffset);

	if (mConvexHull.size() == 2)
	{
		return point.GetAngularOrientation(mConvexHull[0], mConvexHull[1]) == Math::AngularOrientation::Collinear;
	}

	for (size_t prevIndex = 0; prevIndex < mConvexHull.size(); ++prevIndex)
	{
		const size_t nextIndex = prevIndex == (mConvexHull.size() - 1) ? 0 : prevIndex + 1;
		const Math::AngularOrientation angularOrientation = point.GetAngularOrientation(mConvexHull[prevIndex], mConvexHull[nextIndex]);
		if (angularOrientation == Math::AngularOrientation::Collinear)
		{
			return (std::min(mConvexHull[prevIndex].X(), mConvexHull[nextIndex].X()) <= point.X()) &&
				(std::max(mConvexHull[prevIndex].X(), mConvexHull[nextIndex].X()) >= point.X()) &&
				(std::min(mConvexHull[prevIndex].Y(), mConvexHull[nextIndex].Y()) <= point.Y()) &&
				(std::max(mConvexHull[prevIndex].Y(), mConvexHull[nextIndex].Y()) >= point.Y());
		}
		if (angularOrientation == Math::AngularOrientation::Clockwise)
		{
			return false;
		}
	}
	return true;
}

/*static */bool SquareContainment::SimpleTest(const std::vector<NamedVector2>& points, double squareSideLength, Math::FittingTolerance fittingTolerance)
{
	SquareContainment squareContainment(points);
	return squareContainment.Test(squareSideLength, nullptr, nullptr, fittingTolerance) < SquareContainmentResult::kBELOWFits_ABOVEFails;
}

double SquareContainment::GetMidpointXInOriginalExtremes() const
{
	return mConvexHull[mExtremeIndexes.mMinXIndex].X() + 
		(mConvexHull[mExtremeIndexes.mMaxXIndex].X() - mConvexHull[mExtremeIndexes.mMinXIndex].X()) / 2.0;
}

double SquareContainment::GetMidpointYInOriginalExtremes() const
{
	return mConvexHull[mExtremeIndexes.mMinYIndex].X() +
		(mConvexHull[mExtremeIndexes.mMaxYIndex].X() - mConvexHull[mExtremeIndexes.mMinYIndex].X()) / 2.0;
}

SquareContainment::ExtremeIndexes::ExtremeIndexes(const std::vector<NamedVector2>& convexHull)
{
	SetFromHull(convexHull);
}

void SquareContainment::ExtremeIndexes::SetFromHull(const std::vector<NamedVector2>& convexHull)
{
	mMinXIndex = 0;
	mMaxXIndex = 0;
	mMinYIndex = 0;
	mMaxYIndex = 0;

	if (convexHull.size() < 2)
	{
		return;
	}

	for (size_t index = 1; index < convexHull.size(); ++index)
	{
		const NamedVector2& point = convexHull[index];
		if (point.X() < convexHull[mMinXIndex].X())
		{
			mMinXIndex = index;
		}
		if (point.X() > convexHull[mMaxXIndex].X())
		{
			mMaxXIndex = index;
		}
		if (point.Y() < convexHull[mMinYIndex].Y())
		{
			mMinYIndex = index;
		}
		if (point.Y() > convexHull[mMaxYIndex].Y())
		{
			mMaxYIndex = index;
		}
	}
}

SquareContainment::RotatingHull::RotatingHull(const SquareContainment& parent, const std::vector<NamedVector2>& convexHull, double squareSideLength, Math::FittingTolerance fittingTolerance)
: mRotatingConvexHull(convexHull)
, mSquareSideLength(squareSideLength)
, mSquareSideLengthSq(squareSideLength * squareSideLength)
, mFittingTolerance(fittingTolerance)
, mScaledXAxis(squareSideLength, 0.0)
, mScaledYAxis(0.0, squareSideLength)
, mExtremeIndexes(convexHull)
, mAccumulatedRotation(0.0)
{
	SetMinXIndexTo(mExtremeIndexes.mMinXIndex);
	SetMaxXIndexTo(mExtremeIndexes.mMaxXIndex);
	SetMinYIndexTo(mExtremeIndexes.mMinYIndex);
	SetMaxYIndexTo(mExtremeIndexes.mMaxYIndex);
	SkipMeaninglessAngles();
	UpdateNextAnglesOfFit();
}

bool SquareContainment::RotatingHull::FitsInSquare() const
{
	return mFitsSquareWidth && mFitsSquareHeight;
}

bool SquareContainment::RotatingHull::ReachedMaxRotation() const
{
	return mAccumulatedRotation >= (std::numbers::pi / 1.0);
}

void SquareContainment::RotatingHull::RotateToNextSignificantAngle()
{
	const std::array<double, 5> allPossibleRotations = { mNextFitAngle, mNextMinXAngle, mNextMaxXAngle, mNextMinYAngle, mNextMaxYAngle };
	double smallestRotation = std::numeric_limits<double>::max();
	for (double rotation : allPossibleRotations)
	{
		if (rotation > Math::kEpsilon && rotation < smallestRotation)
		{
			smallestRotation = rotation;
		}
	}

	mAccumulatedRotation += smallestRotation;

	for (NamedVector2& point : mRotatingConvexHull)
	{
		point.RotateInPlace(smallestRotation);
	}

	mNextMinXAngle -= smallestRotation;
	if (Math::DetermineSign(mNextMinXAngle) != Math::ZeroExclusiveSign::Positive)
	{
		SetMinXIndexTo(PrevIndex(mExtremeIndexes.mMinXIndex));
	}
	mNextMaxXAngle -= smallestRotation;
	if (Math::DetermineSign(mNextMaxXAngle) != Math::ZeroExclusiveSign::Positive)
	{
		SetMinXIndexTo(PrevIndex(mExtremeIndexes.mMaxXIndex));
	}
	mNextMinYAngle -= smallestRotation;
	if (Math::DetermineSign(mNextMinYAngle) != Math::ZeroExclusiveSign::Positive)
	{
		SetMinXIndexTo(PrevIndex(mExtremeIndexes.mMinYIndex));
	}
	mNextMaxYAngle -= smallestRotation;
	if (Math::DetermineSign(mNextMaxYAngle) != Math::ZeroExclusiveSign::Positive)
	{
		SetMinXIndexTo(PrevIndex(mExtremeIndexes.mMaxYIndex));
	}

	SkipMeaninglessAngles();
	UpdateNextAnglesOfFit();
}

NamedVector2 SquareContainment::RotatingHull::GetCurrentXExtremesVec() const
{
	return mRotatingConvexHull[mExtremeIndexes.mMaxXIndex] - mRotatingConvexHull[mExtremeIndexes.mMinXIndex];
}

NamedVector2 SquareContainment::RotatingHull::GetCurrentYExtremesVec() const
{
	return mRotatingConvexHull[mExtremeIndexes.mMaxYIndex] - mRotatingConvexHull[mExtremeIndexes.mMinYIndex];
}

void SquareContainment::RotatingHull::SetMinXIndexTo(size_t index)
{
	mExtremeIndexes.mMinXIndex = index;
	const double xDiff = mRotatingConvexHull[PrevIndex(index)].X() - mRotatingConvexHull[index].X();
	mNextMinXAngle = Math::DetermineSign(xDiff) == Math::ZeroExclusiveSign::Zero ? 0.0 :
		std::atan2(xDiff, mRotatingConvexHull[PrevIndex(index)].Y() - mRotatingConvexHull[index].Y());
}

void SquareContainment::RotatingHull::SetMaxXIndexTo(size_t index)
{
	mExtremeIndexes.mMaxXIndex = index;
	const double xDiff = mRotatingConvexHull[index].X() - mRotatingConvexHull[PrevIndex(index)].X();
	mNextMaxXAngle = Math::DetermineSign(xDiff) == Math::ZeroExclusiveSign::Zero ? 0.0 :
		std::atan2(xDiff, mRotatingConvexHull[index].Y() - mRotatingConvexHull[PrevIndex(index)].Y());
}

void SquareContainment::RotatingHull::SetMinYIndexTo(size_t index)
{
	mExtremeIndexes.mMinYIndex = index;
	const double yDiff = mRotatingConvexHull[PrevIndex(index)].Y() - mRotatingConvexHull[index].Y();
	mNextMinYAngle = Math::DetermineSign(yDiff) == Math::ZeroExclusiveSign::Zero ? 0.0 :
		std::atan2(yDiff, mRotatingConvexHull[PrevIndex(index)].X() - mRotatingConvexHull[index].X());
}

void SquareContainment::RotatingHull::SetMaxYIndexTo(size_t index)
{
	mExtremeIndexes.mMaxYIndex = index;
	const double yDiff = mRotatingConvexHull[index].Y() - mRotatingConvexHull[PrevIndex(index)].Y();
	mNextMaxYAngle = Math::DetermineSign(yDiff) == Math::ZeroExclusiveSign::Zero ? 0.0 :
		std::atan2(yDiff, mRotatingConvexHull[index].X() - mRotatingConvexHull[PrevIndex(index)].X());
}

void SquareContainment::RotatingHull::SkipMeaninglessAngles()
{
	for (int32_t maxIterations = 6; (Math::DetermineSign(mNextMinXAngle) != Math::ZeroExclusiveSign::Positive) && maxIterations > 0; --maxIterations)
	{
		SetMinXIndexTo(PrevIndex(mExtremeIndexes.mMinXIndex));
	}
	for (int32_t maxIterations = 6; (Math::DetermineSign(mNextMaxXAngle) != Math::ZeroExclusiveSign::Positive) && maxIterations > 0; --maxIterations)
	{
		SetMaxXIndexTo(PrevIndex(mExtremeIndexes.mMaxXIndex));
	}
	for (int32_t maxIterations = 6; (Math::DetermineSign(mNextMinYAngle) != Math::ZeroExclusiveSign::Positive) && maxIterations > 0; --maxIterations)
	{
		SetMinYIndexTo(PrevIndex(mExtremeIndexes.mMinYIndex));
	}
	for (int32_t maxIterations = 6; (Math::DetermineSign(mNextMaxYAngle) != Math::ZeroExclusiveSign::Positive) && maxIterations > 0; --maxIterations)
	{
		SetMaxYIndexTo(PrevIndex(mExtremeIndexes.mMaxYIndex));
	}
}

void SquareContainment::RotatingHull::UpdateNextAnglesOfFit()
{
	mExtremeIndexes.SetFromHull(mRotatingConvexHull);

	const double currentHullWidth = GetCurrentXExtremesVec().X();
	const double currentHullHeight = GetCurrentYExtremesVec().Y();

	mFitsSquareWidth = Math::AbsValueFitsContainer(currentHullWidth * currentHullWidth, mSquareSideLengthSq, mFittingTolerance);
	mFitsSquareHeight = Math::AbsValueFitsContainer(currentHullHeight * currentHullHeight, mSquareSideLengthSq, mFittingTolerance);

	mNextFitAngle = std::numeric_limits<double>::max();

	if (!mFitsSquareWidth)
	{
		const NamedVector2 minToMaxVector = GetCurrentXExtremesVec();
		if (minToMaxVector.MagnitudeSq() > 0.0)
		{
			const NamedVector2 idealVector(
				mSquareSideLength,
				(std::sqrt(minToMaxVector.MagnitudeSq() - mSquareSideLengthSq)));

			mNextFitAngle = std::min(mNextFitAngle, GetSmallestPositiveAnglePastStartingVector(idealVector, mRotatingConvexHull[mExtremeIndexes.mMaxXIndex] - mRotatingConvexHull[mExtremeIndexes.mMinXIndex]));
		}
	}

	if (!mFitsSquareHeight)
	{
		const NamedVector2 minToMaxVector = GetCurrentYExtremesVec();
		if (minToMaxVector.MagnitudeSq() > 0.0)
		{
			const NamedVector2 idealVector(
				(std::sqrt(minToMaxVector.MagnitudeSq() - mSquareSideLengthSq)),
				mSquareSideLength);
			mNextFitAngle = std::min(mNextFitAngle, GetSmallestPositiveAnglePastStartingVector(idealVector, mRotatingConvexHull[mExtremeIndexes.mMaxYIndex] - mRotatingConvexHull[mExtremeIndexes.mMinYIndex]));
		}
	}


	if (Math::DetermineSign(mNextFitAngle) != Math::ZeroExclusiveSign::Positive)
	{
		if (!(mFitsSquareHeight && mFitsSquareWidth))
		{
			mNextFitAngle = std::numeric_limits<double>::max();
		}
	}
}

double SquareContainment::RotatingHull::GetSmallestPositiveAnglePastStartingVector(NamedVector2 idealVector, const NamedVector2& startingVector) const
{
	const double startingAngle = startingVector.Angle();
	const double idealAngle = idealVector.Angle();
	std::array<double, 4> reflectionAngles =
	{
		idealAngle - startingAngle,
		std::numbers::pi - idealAngle - startingAngle,
		std::numbers::pi + idealAngle - startingAngle,
		std::numbers::pi * 2 - idealAngle - startingAngle
	};

	// std::array<double, 4> TestreflectionAngles =
	// {
	// 	idealVector.Angle() - startingAngle,
	// 	idealVector.GetXReflectedVector().Angle() - startingAngle,
	// 	idealVector.GetYReflectedVector().Angle() - startingAngle,
	// 	idealVector.GetXYReflectedVector().Angle() - startingAngle
	// };

	double smallestPositiveReflection = std::numeric_limits<double>::max();
	for (double reflectionAngle : reflectionAngles)
	{
		if (reflectionAngle > 0.0 && reflectionAngle < smallestPositiveReflection)
		{
			smallestPositiveReflection = reflectionAngle;
		}
	}

	return smallestPositiveReflection;
}

size_t SquareContainment::RotatingHull::PrevIndex(size_t index) const
{
	if (index == 0)
	{
		return mRotatingConvexHull.size() - 1;
	}
	return index - 1;
}
