#pragma once
#include "MathCommon.h"
#include "Math.h"
#include "NamedVector2.h"

class PolarCoord2
{
public:
	PolarCoord2(const NamedVector2& source)
	{
		mRadiusSq = source.MagnitudeSq();
		mAngle = std::atan2(source.Y(), source.X());
	}

	void Rotate(double angleRadians)
	{
		mAngle += angleRadians;
	}

	double Angle() const { return mAngle; }
	double RadiusSq() const { return mRadiusSq; }

private:
	double mAngle;
	double mRadiusSq;
};

enum class SquareContainmentResult : uint8_t
{
	kSquareFitsSinglePoint,
	kSquareFitsLineLTEDiagonal,
	kSquareFitsSmallHull,
	kSquareFitsHull,

	kBELOWFits_ABOVEFails, // Check if less than / equal

	kFailHullSegmentExceedsDiagonal,
	kFailHullDoesntFit,
	
	kCount
};
ENUM_OPS(SquareContainmentResult);
ENUM_STRING_CONVERT_DECLARE(SquareContainmentResult);

class SquareContainment
{
public:
	using FuncPtr = void(*)();
	using FuncPtrRotatingHull = void(*)(const std::vector<NamedVector2>&, double, const NamedVector2&, const NamedVector2&);

	SquareContainment(const std::vector<NamedVector2>& points);

	SquareContainmentResult Test(double squareSideLength, std::string* optionalResultContext = nullptr, FuncPtrRotatingHull postRotateCallback = nullptr, Math::FittingTolerance fittingTolerance = Math::FittingTolerance::kFavorFitting) const;
	bool PointIsWithinHull(NamedVector2 point) const;

	// false = fails, true = fits
	static bool SimpleTest(const std::vector<NamedVector2>& points, double squareSideLength, Math::FittingTolerance fittingTolerance = Math::FittingTolerance::kFavorFitting);

	const std::vector<NamedVector2>& GetConvexHull() const { return mConvexHull; }
	const NamedVector2& GetOriginOffset() const { return mOriginOffset; }

	double GetMidpointXInOriginalExtremes() const;
	double GetMidpointYInOriginalExtremes() const;

	static constexpr double kDefaultSideLength = 10000.0;
	static constexpr double kFullSquareSideLength = (kDefaultSideLength * 2.0) + (kDefaultSideLength / 100.f);

private:
	struct ExtremeIndexes
	{
		ExtremeIndexes()
		 : mMinXIndex(0)
		 , mMaxXIndex(0)
		 , mMinYIndex(0)
		 , mMaxYIndex(0)
		{}
		ExtremeIndexes(const std::vector<NamedVector2>& convexHull);
		void SetFromHull(const std::vector<NamedVector2>& convexHull);

		size_t mMinXIndex;
		size_t mMaxXIndex;
		size_t mMinYIndex;
		size_t mMaxYIndex;
	};

	class RotatingHull
	{
	public:
		RotatingHull(const SquareContainment& parent, const std::vector<NamedVector2>& convexHull, double squareSideLength, Math::FittingTolerance fittingTolerance);

		bool FitsInSquare() const;
		bool ReachedMaxRotation() const;
		double GetAccumulatedRotation() const { return mAccumulatedRotation; }

		void RotateToNextSignificantAngle();

		const std::vector<NamedVector2>& GetInternalHull() const { return mRotatingConvexHull; }
		NamedVector2 GetCurrentXExtremesVec() const;
		NamedVector2 GetCurrentYExtremesVec() const;

	private:
		void SetMinXIndexTo(size_t index);
		void SetMaxXIndexTo(size_t index);
		void SetMinYIndexTo(size_t index);
		void SetMaxYIndexTo(size_t index);

		void SkipMeaninglessAngles();

		void UpdateNextAnglesOfFit();
		double GetSmallestPositiveAnglePastStartingVector(NamedVector2 idealVector, const NamedVector2& startingVector) const;

		size_t PrevIndex(size_t index) const;

		std::vector<NamedVector2> mRotatingConvexHull;
		double mSquareSideLength;
		double mSquareSideLengthSq;
		Math::FittingTolerance mFittingTolerance;
		NamedVector2 mScaledXAxis;
		NamedVector2 mScaledYAxis;

		ExtremeIndexes mExtremeIndexes;

		size_t mCurrentMinXIndex;
		size_t mCurrentMaxXIndex;
		size_t mCurrentMinYIndex;
		size_t mCurrentMaxYIndex;

		double mNextMinXAngle;
		double mNextMaxXAngle;
		double mNextMinYAngle;
		double mNextMaxYAngle;

		double mNextFitAngle;

		double mAccumulatedRotation;

		bool mFitsSquareWidth = false;
		bool mFitsSquareHeight = false;
	};

	void BuildConvexHull(const std::vector<NamedVector2>& points);
	void ConvertConvexHullToRelativeToOrigin();
	void MeasureExtremes();

	SquareContainmentResult Test(double squareSideLength, std::string& resultContext, FuncPtrRotatingHull postRotateCallback, Math::FittingTolerance fittingTolerance) const;

	std::vector<NamedVector2> mConvexHull;
	NamedVector2 mOriginOffset;
	double mSmallestDistanceBetweenAnyPointSq;
	double mLargestDistanceBetweenAnyPointSq = 0.0;

	ExtremeIndexes mExtremeIndexes;
};

