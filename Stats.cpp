#include "Stats.h"
#include "MathPrint.h"

void Stats::ClearIntDistribution()
{
	mIntDistribution.clear();
	mIntDistributionValueMin = INT_MAX;
	mIntDistributionValueMax = 0;
	mIntDistributionAmountMax = 0;
}

void Stats::AddToIntDistribution(int32_t distributionId, int32_t value, int32_t amount /*= 1*/)
{
	IntDistribution& distribution = mIntDistributions[distributionId];

	int32_t& currentAmount = distribution.mValueToAmount.try_emplace(value, 0).first->second;
	currentAmount += amount;

	if (value < distribution.mValueMin)
	{
		distribution.mValueMin = value;
	}
	if (value > distribution.mValueMax)
	{
		distribution.mValueMax = value;
	}
	if (currentAmount > distribution.mAmountMax)
	{
		distribution.mAmountMax = currentAmount;
	}
}

void Stats::SetIntDistributionAxisNames(int32_t distributionId, const char* const xName, const char* const yName)
{
	IntDistribution& distribution = mIntDistributions[distributionId];
	distribution.mXName = xName;
	distribution.mYName = yName;
}

void Stats::PrintIntDistribution(int32_t distributionId, int32_t width, int32_t height)
{
	if (mIntDistributions.find(distributionId) == mIntDistributions.end())
	{
		printf("\n\n==Empty==\n\n");
		return;
	}

	IntDistribution& distribution = mIntDistributions[distributionId];
	if (distribution.mValueToAmount.empty())
	{
		printf("\n\n==Empty==\n\n");
		return;
	}

	const int32_t xDelta = distribution.mValueMax - distribution.mValueMin;
	const int32_t xSpacePer = std::max(1, xDelta / width);
	const int32_t ySpacePer = std::max(1, distribution.mAmountMax / height);
	const int32_t yHalfSpace = ySpacePer / 2;

	const int32_t xPrintSpace = (int32_t)distribution.mXName.length() + 1;

	int32_t xSubspacePre = 0;
	int32_t xSubspacePost = 0;
	if (xSpacePer > 1)
	{
		xSubspacePre = xSpacePer / 2;
		xSubspacePost = (xSpacePer - 1) / 2;
	}

	for (int32_t y = distribution.mAmountMax; y > 0; y -= ySpacePer)
	{
		printf("\n%*d|", xPrintSpace, y);
		
		for (int32_t x = distribution.mValueMin; x <= distribution.mValueMax; x += xSpacePer)
		{
			putchar(' ');

			// Combine nearby numbers if they are hidden
			int32_t accumulatedAmount = distribution.mValueToAmount.try_emplace(x, 0).first->second;
			for (int32_t subspaceOffset = 1; subspaceOffset <= xSubspacePre; ++subspaceOffset)
			{
				accumulatedAmount += distribution.mValueToAmount.try_emplace(x - subspaceOffset, 0).first->second;
			}
			for (int32_t subspaceOffset = 1; subspaceOffset <= xSubspacePost; ++subspaceOffset)
			{
				accumulatedAmount += distribution.mValueToAmount.try_emplace(x + subspaceOffset, 0).first->second;
			}
			accumulatedAmount /= (1 + xSubspacePre + xSubspacePost);


			if (accumulatedAmount >= y)
			{
				MathPrint::PrintCharacter(MathPrint::Character::BlockFull);
			}
			else if (accumulatedAmount >= (y - yHalfSpace))
			{
				MathPrint::PrintCharacter(MathPrint::Character::BlockBottom);
			}
			else
			{
				putchar(' ');
			}
		}
	}

	printf("\n%*s|", xPrintSpace, distribution.mXName.c_str());

	int32_t digit = 2;
	while (true)
	{
		const int32_t within = (int32_t)std::pow(10.0, (double)digit);
		const int32_t place = (int32_t)std::pow(10.0, (double)(digit - 1));

		for (int32_t x = distribution.mValueMin; x <= distribution.mValueMax; x += xSpacePer)
		{
			const int32_t xThisDigit = (x % within) / place;
			if (xThisDigit > 0)
			{
				printf(" %d", xThisDigit);
			}
			else if (digit == 1)
			{
				printf(" 0");
			}
			else if ((x / within) > 0)
			{
				printf(" 0");
			}
			else
			{
				printf("  ");
			}
		}
		
		--digit;
		if (digit > 0)
		{
			printf("\n%*s ", xPrintSpace, "");
		}
		else
		{
			break;
		}
	}
}
