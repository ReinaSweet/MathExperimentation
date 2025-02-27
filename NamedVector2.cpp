#include "NamedVector2.h"

#include <format>

double NamedVector2::Magnitude() const
{
	if (!mValidCachedMagnitude) [[unlikely]]
	{
		mCachedMagnitude = std::sqrt(MagnitudeSq());
		mValidCachedMagnitude = true;
	}
	return mCachedMagnitude;
}

double NamedVector2::Angle() const
{
	return std::atan2(mY, mX);
}

NamedVector2 NamedVector2::operator-(const NamedVector2& other) const
{
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
	std::string newName;
	if (mName.size() > 0 && other.mName.size() > 0)
	{
		// If both has a 1 letter name, we form a 2 letter pair. e.g.
		// B - A becomes AB
		if (mName.size() == 0 && other.mName.size() == 0)
		{
			newName.append(other.mName);
			newName.append(mName);
		}
		// If either has longer than a 1 letter name, than we clarify the direction with an arrow in the middle and wrap with parens
		// Back - Front becomes (Front->Back)
		else
		{
			newName.append("(");
			newName.append(other.mName);
			newName.append("->");
			newName.append(mName);
			newName.append(")");
		}
	}
#endif

	return NamedVector2(mX - other.mX, mY - other.mY
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, newName.c_str()
#endif
	);
}

bool NamedVector2::operator==(const NamedVector2& other) const
{
	return mX == other.mX && mY == other.mY;
}

NamedVector2 NamedVector2::GetXReflectedVector() const
{
	return NamedVector2(mX * -1.0, mY
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName
#endif
	);
}

NamedVector2 NamedVector2::GetYReflectedVector() const
{
	return NamedVector2(mX, mY * -1.0
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName
#endif
	);
}

NamedVector2 NamedVector2::GetXYReflectedVector() const
{
	return NamedVector2(mX * -1.0, mY * -1.0
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName
#endif
	);
}

NamedVector2 NamedVector2::Rotate(double angleRads) const
{
	const double c = std::cos(angleRads);
	const double s = std::sin(angleRads);
	const double newX = mX * c - mY * s;
	const double newY = mX * s + mY * c;

#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
	std::string newName;
	if (mName.size() > 0)
	{
		std::format_to(std::back_inserter(newName),
			"({0} rot {1})", mName, Math::RadiansToDegrees(angleRads));
	}
#endif

	return NamedVector2(newX, newY
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, newName.c_str()
#endif
	);
}

void NamedVector2::RotateInPlace(double angleRads)
{
	const double c = std::cos(angleRads);
	const double s = std::sin(angleRads);
	const double newX = mX * c - mY * s;
	const double newY = mX * s + mY * c;

	mX = newX;
	mY = newY;
}

Math::AngularOrientation NamedVector2::GetAngularOrientation(const NamedVector2& prev, const NamedVector2& next) const
{
	const double value = (mY - prev.mY) * (next.mX - mX) - (mX - prev.mX) * (next.mY - mY);
	switch (Math::DetermineSign(value))
	{
	case Math::ZeroExclusiveSign::Zero:
		return Math::AngularOrientation::Collinear;

	case Math::ZeroExclusiveSign::Positive:
		return Math::AngularOrientation::Clockwise;

	case Math::ZeroExclusiveSign::Negative:
		return Math::AngularOrientation::Counterclockwise;

	default:
		return Math::AngularOrientation::Undefined;
	}
}
