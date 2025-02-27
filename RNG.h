#pragma once
#include "MathCommon.h"

class RNG
{
public:
	RNG();

	uint32_t RandomNumber() const;
	size_t RandomIndex(size_t size) const;

private:
	mutable std::mt19937 mRandomDist;
};

