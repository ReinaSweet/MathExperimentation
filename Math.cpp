#include "Math.h"
#include "NamedVector2.h"

const double Math::kEpsilon = 1.0e-6;

/*static */double Math::RadiansToDegrees(double rads)
{
	return rads * 180 / std::numbers::pi_v<double>;
}


/*static */bool Math::IsPointInTriangle(const NamedVector2& point, const NamedVector2& A, const NamedVector2& B, const NamedVector2& C)
{
	const NamedVector2 AP = point - A;
	const NamedVector2 BP = point - B;
	const NamedVector2 CP = point - C;

	const NamedVector2 AB = B - A;
	const NamedVector2 BC = C - B;
	const NamedVector2 CA = A - C;

	const double APcrossAB = AP.CrossProduct(AB);
	const double BPcrossBC = BP.CrossProduct(BC);
	const double CPcrossCA = CP.CrossProduct(CA);

	const ZeroExclusiveSign signAPcrossAB = DetermineSign(APcrossAB);
	const ZeroExclusiveSign signBPcrossBC = DetermineSign(BPcrossBC);
	const ZeroExclusiveSign signCPcrossCA = DetermineSign(CPcrossCA);

	const int32_t zeroes = (signAPcrossAB == ZeroExclusiveSign::Zero) + (signBPcrossBC == ZeroExclusiveSign::Zero) + (signCPcrossCA == ZeroExclusiveSign::Zero);

	switch (zeroes)
	{
	case 0:
		return (signAPcrossAB == signBPcrossBC) && (signBPcrossBC == signCPcrossCA);

	case 1:
	{
		const int32_t positives = (signAPcrossAB == ZeroExclusiveSign::Positive) + (signBPcrossBC == ZeroExclusiveSign::Positive) + (signCPcrossCA == ZeroExclusiveSign::Positive);
		const int32_t negatives = (signAPcrossAB == ZeroExclusiveSign::Negative) + (signBPcrossBC == ZeroExclusiveSign::Negative) + (signCPcrossCA == ZeroExclusiveSign::Negative);
		return (positives != negatives);
	}

	case 2:
		return true;

		// undefined cases, just return false for now?
	case 3:
	default:
		return false;
	}
}

/*static */Math::ZeroExclusiveSign Math::DetermineSign(double value, double epsilon)
{
	if (std::abs(value) < epsilon)
	{
		return ZeroExclusiveSign::Zero;
	}
	if (std::signbit(value))
	{
		return ZeroExclusiveSign::Negative;
	}
	return ZeroExclusiveSign::Positive;
}

/*static */double Math::SignValue(double value, double epsilon)
{
	return DetermineSign(value, epsilon) == ZeroExclusiveSign::Negative ? -1.0 : 1.0;
}

/*static */bool Math::AbsValueFitsContainer(double value, double container, FittingTolerance fittingTolerance, double epsilon)
{
	value = std::abs(value);

	switch (fittingTolerance)
	{
	default:
	case Math::FittingTolerance::kAcceptInaccuraciesOfDoubleAsIs:
		return value < container;

	case Math::FittingTolerance::kFavorFitting:
		return value < (container + kEpsilon);

	case Math::FittingTolerance::kFavorFailing:
		return value < (container - kEpsilon);
	}
}

/*static */bool Math::LineSegLineSegIntersection(const NamedVector2& A, const NamedVector2& B, const NamedVector2& C, const NamedVector2& D, NamedVector2* OutIntersection)
{
	const NamedVector2 Seg1 = B - A;
	const NamedVector2 Seg2 = D - C;
	const NamedVector2 CA = A - C;
	const double Seg1crossSeg2 = Seg1.CrossProduct(Seg2);

	if (DetermineSign(Seg1crossSeg2) == ZeroExclusiveSign::Zero)
	{
		return false;
	}

	const double Seg1crossCA = Seg1.CrossProduct(CA);
	const double Seg2crossCA = Seg2.CrossProduct(CA);

	// Lines have crossed by this point. Now determine if they crossed within the bounds of the segments
	const double s = Seg1crossCA / Seg1crossSeg2;
	const double t = Seg2crossCA / Seg1crossSeg2;

	if ((s >= 0.0) && (s <= 1.0) && (t >= 0.0) && (t <= 1.0))
	{
		if (OutIntersection)
		{
			const double x = A.X() + (t * Seg1.X());
			const double y = A.Y() + (t * Seg1.Y());
			*OutIntersection = NamedVector2(x, y, "intersect");
		}
		return true;
	}
	return false;
}

/*static */bool Math::LineLineIntersection(const NamedVector2& A, const NamedVector2& B, const NamedVector2& C, const NamedVector2& D, NamedVector2* OutIntersection /*= nullptr*/)
{
	const NamedVector2 Line1 = B - A;
	const NamedVector2 Line2 = D - C;
	const NamedVector2 CA = A - C;
	const double Line1crossLine2 = Line1.CrossProduct(Line2);

	if (DetermineSign(Line1crossLine2) == ZeroExclusiveSign::Zero)
	{
		return false;
	}

	if (OutIntersection)
	{
		const double Seg1crossCA = Line1.CrossProduct(CA);
		const double Seg2crossCA = Line2.CrossProduct(CA);

		const double s = Seg1crossCA / Line1crossLine2;
		const double t = Seg2crossCA / Line1crossLine2;

		const double x = A.X() + (t * Line1.X());
		const double y = A.Y() + (t * Line1.Y());
		*OutIntersection = NamedVector2(x, y, "intersect");
	}
	return true;
}
