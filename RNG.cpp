#include "RNG.h"

RNG::RNG()
{
	mRandomDist.seed(static_cast<uint32_t>(std::time(nullptr)) + 0xC0135BAB);
}

uint32_t RNG::RandomNumber() const
{
	return mRandomDist();
}

size_t RNG::RandomIndex(size_t size) const
{
	return RandomNumber() % size;
}
