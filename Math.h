#pragma once

class NamedVector2;

class Math
{
public:
	static const double kEpsilon;

	enum class ZeroExclusiveSign : uint8_t
	{
		Zero,
		Positive,
		Negative
	};

	enum class AngularOrientation : uint8_t
	{
		Collinear,
		Clockwise,
		Counterclockwise,
		Undefined
	};

	enum class FittingTolerance : uint8_t
	{
		// Does a comparison with all of the float approximation issues in tact
		kAcceptInaccuraciesOfDoubleAsIs,

		// A ~10000.00000001 value FITs a container of 1000 (with default epsilon of kEpsilon)
		kFavorFitting,

		// A ~9999.999999991 value FAILs to fit a container of 1000 (with default epsilon of kEpsilon)
		kFavorFailing
	};

	static double RadiansToDegrees(const double radians);
	static bool IsPointInTriangle(const NamedVector2& point, const NamedVector2& A, const NamedVector2& B, const NamedVector2& C);
	static ZeroExclusiveSign DetermineSign(double value, double epsilon = kEpsilon);
	static double SignValue(double value, double epsilon = kEpsilon);
	static bool AbsValueFitsContainer(double value, double container, FittingTolerance fittingTolerance, double epsilon = kEpsilon);
	static bool LineSegLineSegIntersection(const NamedVector2& A, const NamedVector2& B, const NamedVector2& C, const NamedVector2& D, NamedVector2* OutIntersection = nullptr);
	static bool LineLineIntersection(const NamedVector2& A, const NamedVector2& B, const NamedVector2& C, const NamedVector2& D, NamedVector2* OutIntersection = nullptr);

	static int32_t GetRecommendedPrecisionOfFloat(float value, int32_t maxPrecision = 6)
	{
		// This function can be designated constexpr with C++23
		// Gets the next nearest to 1 in decimal precisions
		const int32_t inverseRoundingExponent = 3 + (3 * maxPrecision) + (int32_t)(maxPrecision / 3);
		const float roundingModifier = std::ldexp(1.f, -1 * inverseRoundingExponent);

		float valueIntegerPart = 0.f;
		value = std::modff(std::abs(value) + roundingModifier, &valueIntegerPart);
		int32_t precision = 0;
		int32_t trailingZeros = 0;
		do
		{
			value = std::modff(value * 10.f, &valueIntegerPart);
			trailingZeros += (int32_t)(valueIntegerPart < 0.0001f);
			++precision;
		} while ((trailingZeros < 2) && (precision < maxPrecision));

		precision -= trailingZeros;
		return precision;
	}

private:
};

