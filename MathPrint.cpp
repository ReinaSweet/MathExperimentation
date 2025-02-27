#include "MathPrint.h"

/*static*/ void MathPrint::PrintResetCC()
{
	printf("\x1B[0m");
}

/*static*/ void MathPrint::PrintCCValue(PrintColor color)
{
	switch (color)
	{
	case PrintColor::LightGray: printf("5;250m"); break;
	case PrintColor::MidGray: printf("5;244m"); break;
	case PrintColor::DarkGray: printf("5;238m"); break;
	case PrintColor::Orange: printf("5;202m"); break;
	case PrintColor::Green: printf("5;28m"); break;
	case PrintColor::Brown: printf("5;94m"); break;
	case PrintColor::Purple: printf("5;55m"); break;
	case PrintColor::Red: printf("5;160m"); break;
	case PrintColor::DarkRed: printf("5;88m"); break;
	case PrintColor::DarkYellow: printf("5;220m"); break;
	case PrintColor::Blue: printf("5;33m"); break;
	default: break;
	}
}

/*static*/ void MathPrint::PrintBGColorCode(PrintColor color)
{
	printf("\x1B[48;");
	PrintCCValue(color);
}

/*static*/ void MathPrint::PrintFGColorCode(PrintColor color)
{
	printf("\x1B[38;");
	PrintCCValue(color);
}

/*static*/ void MathPrint::PrintCharacter(Character character)
{
	switch (character)
	{
	case MathPrint::Character::BlockFull: printf("\xDB"); return;
	case MathPrint::Character::BlockBottom: printf("\xDC"); return;
	case MathPrint::Character::BlockLeft: printf("\xDD"); return;
	case MathPrint::Character::BlockRight: printf("\xDE"); return;
	case MathPrint::Character::BlockTop: printf("\xDF"); return;
	default:
		break;
	}
}
