#pragma once
#include "stdafx.h"
#include "Math.h"

#define NAMEDVECTOR2_ENABLESTRINGNAMES 1

class NamedVector2
{
public:
	NamedVector2(const char* name = nullptr)
		: mX(0.0)
		, mY(0.0)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName{name ? name : ""}
#endif
	{}

	NamedVector2(uint64_t x, uint64_t y, const char* name = nullptr)
		: mX(static_cast<double>(x))
		, mY(static_cast<double>(y))
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName{ name ? name : "" }
#endif
	{}

	NamedVector2(uint64_t x, uint64_t y, const std::string& name)
		: mX(static_cast<double>(x))
		, mY(static_cast<double>(y))
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName(name)
#endif
	{}

	NamedVector2(double x, double y, const char* name = nullptr)
		: mX(x)
		, mY(y)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName{ name ? name : "" }
#endif
	{}

	NamedVector2(double x, double y, const std::string& name)
		: mX(x)
		, mY(y)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName(name)
#endif
	{}

	NamedVector2(const NamedVector2& other)
		: mX(other.mX)
		, mY(other.mY)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName(other.mName)
#endif
	{}

	NamedVector2(const NamedVector2& other, const char* name)
		: mX(other.mX)
		, mY(other.mY)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName{ name ? name : "" }
#endif
	{}

	NamedVector2(const NamedVector2& other, const std::string& name)
		: mX(other.mX)
		, mY(other.mY)
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		, mName(name)
#endif
	{}

	void SetName(const std::string& name)
	{
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		mName = name;
#endif
	}

	void AssignButRetainName(const NamedVector2& other)
	{
		mX = other.mX;
		mY = other.mY;
	}

	void AssignButRetainName(double x, double y)
	{
		mX = x;
		mY = y;
	}

	void Assign(const NamedVector2& other)
	{
		mX = other.mX;
		mY = other.mY;
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		mName = other.mName;
#endif
	}

	void SwapButRetainNames(NamedVector2& other)
	{
		const NamedVector2 temp(mX, mY);
		AssignButRetainName(other);
		other.AssignButRetainName(temp);
	}

	void Swap(NamedVector2& other)
	{
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
		const NamedVector2 temp(mX, mY, mName);
#else
		const NamedVector2 temp(mX, mY);
#endif
		Assign(other);
		other.Assign(temp);
	}

	void ReduceButRetainNames(const NamedVector2& reduceBy)
	{
		mX -= reduceBy.mX;
		mY -= reduceBy.mY;
	}

	void IncreaseButRetainNames(const NamedVector2& increaseBy)
	{
		mX += increaseBy.mX;
		mY += increaseBy.mY;
	}

	double MagnitudeSq() const
	{
		return mX * mX + mY * mY;
	}

	double Magnitude() const;

	double Angle() const;

	double X() const
	{
		return mX;
	}

	double Y() const
	{
		return mY;
	}

#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
	const std::string& Name() const { return mName; }
#else
	const std::string& Name() const { static std::string kEmpty; return kEmpty; }
#endif

	NamedVector2 operator-(const NamedVector2& other) const;
	bool operator==(const NamedVector2& other) const;

	NamedVector2 GetXReflectedVector() const;
	NamedVector2 GetYReflectedVector() const;
	NamedVector2 GetXYReflectedVector() const;

	double CrossProduct(const NamedVector2& other) const
	{
		return (mX * other.mY) - (other.mX * mY);
	}

	double DotProduct(const NamedVector2& other) const
	{
		return (mX * other.mX) + (mY * other.mY);
	}

	NamedVector2 Rotate(double angleRads) const;

	void RotateInPlace(double angleRads);

	double DistSq(const NamedVector2& other) const
	{
		return (mX - other.mX) * (mX - other.mX) + (mY - other.mY) * (mY - other.mY);
	}

	Math::AngularOrientation GetAngularOrientation(const NamedVector2& prev, const NamedVector2& next) const;

private:
	double mX;
	double mY;
#ifdef NAMEDVECTOR2_ENABLESTRINGNAMES
	std::string mName;
#endif

	mutable double mCachedMagnitude = 0.0;
	mutable bool mValidCachedMagnitude = false;
};
