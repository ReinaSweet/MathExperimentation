#pragma once
#include "stdafx.h"
#include "NamedVector2.h"

class SmallestSquare
{
public:
	SmallestSquare(const NamedVector2& pos1, const NamedVector2& pos2, const NamedVector2& pos3);

	double CalculateSmallestSquareSide() const;

	const NamedVector2& GetBigSide() const { return mSideBig; }
	const NamedVector2& GetMediumSide() const { return mSideMedium; }
	const NamedVector2& GetSmallSide() const { return mSideSmall; }

	double GetCosSmall() const { return mCosSmall; }
	double GetSinSmall() const { return mSinSmall; }
	double GetAngleSmallDegrees() const;

	double GetAngleMediumDegrees() const;
	double GetAngleBigDegrees() const;

private:
	double CalculateSmallestSquareSideOfPiMult(double piMult) const;
	double CalculateCosine(const NamedVector2& s1, const NamedVector2& s2, const NamedVector2& oppositeSide) const;
	double SmallestSideAtAngle(double angle) const;
	double RadiansToDegrees(double rads) const;

	NamedVector2 mSideBig;
	NamedVector2 mSideMedium;
	NamedVector2 mSideSmall;

	double mCosSmall;
	double mSinSmall;
	double mAngleSmall;
};

