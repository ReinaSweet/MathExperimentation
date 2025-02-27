#pragma once
#include "MathCommon.h"

class MathPrint
{
public:
	enum class PrintColor : uint8_t
	{
		Orange,
		LightGray,
		MidGray,
		DarkGray,
		Green,
		Brown,
		Purple,
		Red,
		DarkRed,
		DarkYellow,
		Blue
	};

	enum class Character : uint8_t
	{
		BlockFull,
		BlockBottom,
		BlockLeft,
		BlockRight,
		BlockTop
	};

	static void PrintResetCC();
	static void PrintBGColorCode(PrintColor color);
	static void PrintFGColorCode(PrintColor color);

	static void PrintCharacter(Character character);

private:
	static void PrintCCValue(PrintColor color);
};

