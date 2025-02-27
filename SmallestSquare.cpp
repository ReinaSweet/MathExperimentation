#include "SmallestSquare.h"

// https://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf

SmallestSquare::SmallestSquare(const NamedVector2& pos1, const NamedVector2& pos2, const NamedVector2& pos3)
{
	mSideBig = pos2 - pos1; // a->b
	mSideMedium = pos3 - pos1; // a->c
	mSideSmall = pos3 - pos2; // b->c

	// manual sort since we may have to reorient positions anyways
	// Always reidentify small first, since its missing position will imply which position must go second on the remaining two side
	// Also, there's only 6 possible outcomes so the number of checks is pretty straightforward
	if (mSideMedium.Magnitude() > mSideBig.Magnitude())
	{
		// We're exactly inverted (big is small, small is big)
		if (mSideSmall.Magnitude() > mSideMedium.Magnitude())
		{
			mSideSmall = pos2 - pos1;
			mSideMedium = pos1 - pos3;
			mSideBig = pos2 - pos3;
		}
		// Assumed big is actually the smallest. Assumed small is actually medium and assumed medium is actually big
		else if (mSideSmall.Magnitude() > mSideBig.Magnitude())
		{
			mSideSmall = pos2 - pos1;
			mSideMedium = pos2 - pos3;
			mSideBig = pos1 - pos3;
		}
		// Small was correct, but big and medium were flipped
		else
		{
			mSideMedium = pos2 - pos1;
			mSideBig = pos3 - pos1;
		}
	}
	else if (mSideSmall.Magnitude() > mSideMedium.Magnitude())
	{
		// The assumed small is actually the biggest. ABig is medium. AMedium is small.
		if (mSideSmall.Magnitude() > mSideBig.Magnitude())
		{
			mSideSmall = pos3 - pos1;
			mSideMedium = pos1 - pos2;
			mSideBig = pos3 - pos2;
		}
		// The assumed small is actually medium. ABig is still big. AMedium is small.
		else
		{
			mSideSmall = pos3 - pos1;
			mSideMedium = pos3 - pos2;
			mSideBig = pos1 - pos2;
		}
	}

	mCosSmall = CalculateCosine(mSideBig, mSideMedium, mSideSmall);
	mAngleSmall = std::acos(mCosSmall);
	mSinSmall = std::sin(mSinSmall);
}

double SmallestSquare::CalculateSmallestSquareSide() const
{
// 	double smallestSide = std::numeric_limits<double>::max();
// 	for (double piMult = -4.0; piMult <= 40.0; piMult += 1.0)
// 	{
// 		smallestSide = std::min(CalculateSmallestSquareSideOfPiMult(piMult), smallestSide);
// 	}
// 	return smallestSide;
 	return CalculateSmallestSquareSideOfPiMult(0.0);
}

double SmallestSquare::CalculateSmallestSquareSideOfPiMult(double piMult) const
{
	const size_t kTestedAngles = 5;
	const double testedAngles[kTestedAngles] =
	{
		std::numbers::pi_v<double> / 4.0, // x1,y1 intersect

		// x1,y2 intersect
		std::atan(
			(mSideBig.Magnitude() - mSinSmall * mSideMedium.Magnitude())
			/ (mCosSmall * mSideMedium.Magnitude())
			)
			+ std::numbers::pi_v<double> *piMult,

		// x2,y1 intersect
		std::atan(
			(mSideBig.Magnitude() -  mCosSmall * mSideMedium.Magnitude())
			/ (mSideBig.Magnitude() - mSinSmall * mSideMedium.Magnitude())
			)
			+ std::numbers::pi_v<double> *piMult,

		// x2, y2 intersect
		std::atan(
			(mSideBig.Magnitude() -  mSideMedium.Magnitude() * (mCosSmall + mSinSmall))
			/ (mSideMedium.Magnitude() * (mCosSmall - mSinSmall))
			)
			+ std::numbers::pi_v<double> *piMult,

		// y1,y2 intersect
		std::atan(
			(mSinSmall * mSideMedium.Magnitude())
			/ (mSideBig.Magnitude() - mCosSmall * mSideMedium.Magnitude())
			)
			+ std::numbers::pi_v<double> *piMult
	};

	// Mostly for debugging
	double resultSides[kTestedAngles] = {};
	for (size_t index = 0; index < kTestedAngles; ++index)
	{
		resultSides[index] = SmallestSideAtAngle(testedAngles[index]);
	}

	double smallestSide = resultSides[0];
	for (size_t index = 1; index < kTestedAngles; ++index)
	{
		if (resultSides[index] < smallestSide)
		{
			smallestSide = resultSides[index];
		}
	}
	return smallestSide;
}

double SmallestSquare::GetAngleSmallDegrees() const
{
	return RadiansToDegrees(mAngleSmall);
}

double SmallestSquare::GetAngleMediumDegrees() const
{
	return RadiansToDegrees(std::acos(CalculateCosine(mSideBig, mSideSmall, mSideMedium)));
}

double SmallestSquare::GetAngleBigDegrees() const
{
	return RadiansToDegrees(std::acos(CalculateCosine(mSideMedium, mSideSmall, mSideBig)));
}

double SmallestSquare::CalculateCosine(const NamedVector2& s1, const NamedVector2& s2, const NamedVector2& oppositeSide) const
{
	return (s1.MagnitudeSq() + s2.MagnitudeSq() - oppositeSide.MagnitudeSq()) / (2 * s1.Magnitude() * s2.Magnitude());
}

double SmallestSquare::SmallestSideAtAngle(double angle) const
{
	const double x2 = mSideBig.Magnitude() * std::cos(angle) - mSideMedium.Magnitude() * std::cos(angle + mAngleSmall);
	const double y2 = mSideMedium.Magnitude() * std::sin(angle + mAngleSmall);

	if ((angle < 0) || (angle > (std::numbers::pi_v<double> / 2)))
	{
		return std::max(x2, y2);
	}

	const double x1 = mSideBig.Magnitude() * std::cos(angle);
	const double y1 = mSideBig.Magnitude() * std::sin(angle);

	return std::max(std::max(x1, x2), std::max(y1, y2));
}

double SmallestSquare::RadiansToDegrees(double rads) const
{
	return rads * 180 / std::numbers::pi_v<double>;
}
