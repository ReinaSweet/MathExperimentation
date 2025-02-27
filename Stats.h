#pragma once
#include "MathCommon.h"

class Stats
{
public:
	Stats()
	{}

	void ClearIntDistribution();
	void AddToIntDistribution(int32_t distributionId, int32_t value, int32_t amount = 1);
	void SetIntDistributionAxisNames(int32_t distributionId, const char* const xName, const char* const yName);
	void PrintIntDistribution(int32_t distributionId, int32_t width, int32_t height);

private:
	struct IntDistribution
	{
		std::map<int32_t, int32_t> mValueToAmount;
		int32_t mValueMin = INT_MAX;
		int32_t mValueMax = 0;
		int32_t mAmountMax = 0;

		std::string mXName;
		std::string mYName;
	};

	std::map<int32_t, IntDistribution> mIntDistributions;

	std::map<int32_t, int32_t> mIntDistribution;
	int32_t mIntDistributionValueMin = INT_MAX;
	int32_t mIntDistributionValueMax = 0;
	int32_t mIntDistributionAmountMax = 0;
};

