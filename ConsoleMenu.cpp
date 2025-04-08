#include "ConsoleMenu.h"

InputMode GetInputModeFromChar(const char c)
{
    switch (c)
    {
        case 'x': return InputMode::kHex;
        case 'd': return InputMode::kDecimal;
        case 'b': return InputMode::kBinary;

        default: return InputMode::kInvalid;
    }
}

int GetBaseFromInputMode(const InputMode inputMode)
{
    switch (inputMode)
    {
        case InputMode::kHex: return 16;
        case InputMode::kDecimal: return 10;
        case InputMode::kBinary: return 2;

        default: return 10;
    }
}

CONSOLE_MENU_COMMAND_DEFINE(1, mArgs[0]);
CONSOLE_MENU_COMMAND_DEFINE(2, mArgs[0], mArgs[1]);
CONSOLE_MENU_COMMAND_DEFINE(3, mArgs[0], mArgs[1], mArgs[2]);
CONSOLE_MENU_COMMAND_DEFINE(4, mArgs[0], mArgs[1], mArgs[2], mArgs[3]);
CONSOLE_MENU_COMMAND_DEFINE(5, mArgs[0], mArgs[1], mArgs[2], mArgs[3], mArgs[4]);
CONSOLE_MENU_COMMAND_DEFINE(6, mArgs[0], mArgs[1], mArgs[2], mArgs[3], mArgs[4], mArgs[5]);

CONSOLE_MENU_TEXT_COMMAND_DEFINE(1, mArgs[0].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(2, mArgs[0].c_str(), mArgs[1].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(3, mArgs[0].c_str(), mArgs[1].c_str(), mArgs[2].c_str());
CONSOLE_MENU_TEXT_COMMAND_DEFINE(4, mArgs[0].c_str(), mArgs[1].c_str(), mArgs[2].c_str(), mArgs[3].c_str());

void ConsoleMenu::ConsoleMenuCommandSubMenu::Execute()
{
    printf("\n\n");
    if (mPreOpenMenuFunc)
    {
        mPreOpenMenuFunc();
    }
    mConsoleMenu.ResetMenu();
}

int32_t ConsoleMenuCommandI::Matches(const char* const command) const
{
    size_t pos = 0;
    char c = command[pos];
    while (c != '\0')
    {
        if (c != mCommand[pos])
        {
            return -1;
        }
        c = command[++pos];
    }

    return (int32_t)(mCommand.length()) - (int32_t)(pos);
}

const ConsoleMenuCommandInput& ConsoleMenuCommandI::GetInput(size_t pos) const
{
    if (pos < mInputs.size())
    {
        return mInputs.at(pos);
    }

    static const ConsoleMenuCommandInput skInvalidReturn;
    return skInvalidReturn;
}

ConsoleMenuCommandI::ConsoleMenuCommandI(const char* const command, const char* const description)
    : mCommand(command)
    , mDescription(description)
{
    SplitDescriptionAmongInputs();
}

ConsoleMenuCommandI::ConsoleMenuCommandI(const char* const command, const char* const description, InputMode forcedInputMode)
    : mCommand(command)
    , mDescription(description)
{
    SplitDescriptionAmongInputs(forcedInputMode);
}

void ConsoleMenuCommandI::SplitDescriptionAmongInputs(InputMode forcedInputMode)
{
    size_t semiColonLoc = mDescription.find(';');
    if (semiColonLoc != std::string::npos)
    {
        const size_t firstColonLoc = semiColonLoc;
        do
        {
            const char inputModeCharacter = mDescription.at(semiColonLoc + 1);
            const InputMode newMode = (forcedInputMode == InputMode::kInvalid ? GetInputModeFromChar(inputModeCharacter) : forcedInputMode);
            const size_t inputDescriptionStart = semiColonLoc + ((newMode == InputMode::kInvalid) || (newMode == InputMode::kText) ? 1 : 2);

            semiColonLoc = mDescription.find(';', semiColonLoc + 1);
            if (semiColonLoc == std::string::npos)
            {
                mInputs.emplace_back(newMode, mDescription.substr(inputDescriptionStart));
            }
            else
            {
                mInputs.emplace_back(newMode, mDescription.substr(inputDescriptionStart, semiColonLoc - inputDescriptionStart));
            }
        } while (semiColonLoc != std::string::npos);

        mDescription.resize(firstColonLoc);
    }
}

ConsoleMenu::~ConsoleMenu()
{
    for (ConsoleMenuCommandI* command : mCommands)
    {
        delete command;
    }
    mCommands.clear();
}

void ConsoleMenu::RunMenu()
{
    ResetMenu();

    char c;
    do
    {
        c = _getch();
    } while (ReceiveInput(c));
}

void ConsoleMenu::RunMenuDirectlyAtCommand(bool shouldExitAfterCommand, const char* const input)
{
    ClearInput();
    mInputMode = InputMode::kCommand;
    mExitAfterCommandExecution = shouldExitAfterCommand;

    mCurrentPos = 0;

    char c;
    do 
    {
        c = input[mCurrentPos];
        mInputBuffer[mCurrentPos] = c;
        ++mCurrentPos;
    } while (c != '\0');

    EvaluateCommandInput();
    do
    {
        c = _getch();
    } while (ReceiveInput(c));
}

void ConsoleMenu::ResetMenu()
{
    ClearInput();

    putchar('\n');
    PrintHorizontalBreak();
    putchar('\n');
    for (ConsoleMenuCommandI* command : mCommands)
    {
        printf(" %s = %s\n", command->GetCommand(), command->GetDescription());
    }

    if (mParentMenu)
    {
        printf(" x = Return to %s\n\n", mParentMenu->mDescription.c_str());
    }
    else
    {
        printf(" x = Exit\n\n");
    }

    mInputMode = InputMode::kCommand;
    mExitAfterCommandExecution = false;
}

void ConsoleMenu::PrintHorizontalBreak()
{
    putchar('\n');
    const size_t consoleWidth = ConsoleInfo::GetConsoleWidth();
    for (size_t i = 0; i < consoleWidth; ++i)
    {
        putchar('=');
    }
    putchar('\n');
}

bool ConsoleMenu::ReceiveInput(char c)
{
    if (mInputMode == InputMode::kSubMenu)
    {
        if (!mCurrentSubMenu->ReceiveInput(c))
        {
            ResetMenu();
        }
        return true;
    }

    // Lowercase
    if (c > 64 && c < 91)
    {
        c += 32;
    }

    if (c == '\b')
    {
        BackspaceLastInput();
    }
    else if (c == 27)
    {
        // Escape first clears, then "backs out" of the input mode (up to backing out of the program)
        if (mInputMode == InputMode::kHex || mInputMode == InputMode::kBinary)
        {
            ConvertModeTo(InputMode::kDecimal);
        }
        else if (mCurrentPos > 0u)
        {
            if (mInputMode == InputMode::kDecimal)
            {
                ClearInput();
                putchar('d');
            }
            else if (mInputMode == InputMode::kText)
            {
                ClearInput();
            }
        }
        else if (mInputMode != InputMode::kCommand)
        {
            ResetMenu();
        }
        else
        {
            return false;
        }
    }
    else if (mInputMode == InputMode::kCommand)
    {
        // Special command: eXit
        if ((mCurrentPos == 0u) && (c == 'x'))
        {
            return false;
        }

        ReceiveCommandInput(c);
    }
    else if (mInputMode == InputMode::kText)
    {
        ReceiveTextInput(c);
    }
    else if (mInputMode == InputMode::kDecimal)
    {
        const InputMode newMode = GetInputModeFromChar(c);
        if (newMode == InputMode::kInvalid)
        {
            ReceiveNumberInput(c);
        }
        else
        {
            ConvertModeTo(newMode);
        }
    }
    else
    {
        ReceiveNumberInput(c);
    }

    // Above commands can change us to this mode
    if (mInputMode == InputMode::kForceExit)
    {
        return false;
    }
    return true;
}

void ConsoleMenu::AddCommand(const char* const input, const char* const description, const FuncPtr0Num& func)
{
    mCommands.push_back(new ConsoleMenuCommand0Num(input, description, func));
}

void ConsoleMenu::AddSubmenu(const char* const input, ConsoleMenu& subMenu)
{
    mCommands.push_back(new ConsoleMenuCommandSubMenu(input, subMenu));
}

void ConsoleMenu::AddSubmenu(const char* const input, ConsoleMenu& subMenu, const char* const description)
{
    mCommands.push_back(new ConsoleMenuCommandSubMenu(input, subMenu, description));
}

void ConsoleMenu::AddSubmenu(const char* const input, ConsoleMenu& subMenu, const char* const description, FuncPtr0Num preOpenMenuFunc)
{
    ConsoleMenuCommandSubMenu* subMenuCommand = new ConsoleMenuCommandSubMenu(input, subMenu, description);
    subMenuCommand->SetPreOpenMenuFunc(preOpenMenuFunc);
    mCommands.push_back(subMenuCommand);
}

void ConsoleMenu::AddSubmenu(const char* const input, ConsoleMenu& subMenu, FuncPtr0Num preOpenMenuFunc)
{
    ConsoleMenuCommandSubMenu* subMenuCommand = new ConsoleMenuCommandSubMenu(input, subMenu);
    subMenuCommand->SetPreOpenMenuFunc(preOpenMenuFunc);
    mCommands.push_back(subMenuCommand);
}

void ConsoleMenu::ReceiveCommandInput(char c)
{
    PutInput(c);
    EvaluateCommandInput();
}

void ConsoleMenu::EvaluateCommandInput()
{
    size_t partialMatches = 0u;
    for (ConsoleMenuCommandI* command : mCommands)
    {
        const int32_t matchResults = command->Matches(mInputBuffer);
        if (matchResults == 0)
        {
            mCurrentCommand = command;

            StartNewInputOnNewLine();

            if (mCurrentCommand->IsSubMenu())
            {
                mCurrentSubMenu = &(static_cast<ConsoleMenuCommandSubMenu*>(mCurrentCommand)->GetMenu());
                mInputMode = InputMode::kSubMenu;
                ClearInput();
                mCurrentCommand->Execute();
            }
            else if (mCurrentCommand->GetNumArgs() > 0u)
            {
                mCurrentParam = 0u;
                mCurrentCommand->PreArgsExecute();
                StartParamInput();
            }
            else
            {
                ExecuteCurrentCommand();
            }
            return;
        }
        else if (matchResults > 0)
        {
            ++partialMatches;
        }
    }

    if (partialMatches == 0u)
    {
        ClearInput();
    }
}

void ConsoleMenu::ReceiveTextInput(char c)
{
    if (c == '\n' || c == '\r')
    {
        mCurrentCommand->SetArg(mCurrentParam - 1, mInputBuffer);
        CommitCurrentCommandParam();
    }
    else
    {
        PutInput(c);
    }
}

void ConsoleMenu::ReceiveNumberInput(char c)
{
    if (c == ' ' || c == '\n' || c == '\r')
    {
        const int64_t val = strtoull(mInputBuffer + 1, nullptr, GetBaseFromInputMode(mInputMode));
        mCurrentCommand->SetArg(mCurrentParam - 1, (uint64_t)val);
        CommitCurrentCommandParam();
    }
    else
    {
        if (mInputMode == InputMode::kBinary)
        {
            if (c == '0' || c == '1')
            {
                PutInput(c);
            }
        }
        else if (mInputMode == InputMode::kDecimal)
        {
            if (c >= '0' && c <= '9')
            {
                PutInput(c);
            }
        }
        else if (mInputMode == InputMode::kHex)
        {
            if ((c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f'))
            {
                PutInput(c);
            }
        }
    }
}

void ConsoleMenu::CommitCurrentCommandParam()
{
    StartNewInputOnNewLine();
    if (mCurrentCommand->GetNumArgs() > mCurrentParam)
    {
        StartParamInput();
    }
    else
    {
        switch (mCurrentCommand->ValidateArgs())
        {
        case ValidateResponse::kOk:
        {
            ExecuteCurrentCommand();
            break;
        }
        case ValidateResponse::kTryAgain:
        {
            mCurrentParam = 0u;
            mCurrentCommand->PreArgsExecute();
            StartParamInput();
            break;
        }
        case ValidateResponse::kExitMenu:
        {
            mInputMode = InputMode::kForceExit;
            break;
        }
        }
    }
}

void ConsoleMenu::ConvertModeTo(InputMode mode)
{
    ClearInput();
    mInputMode = mode;
    if (mInputMode == InputMode::kBinary)
    {
        PutInput('b');
    }
    else if (mInputMode == InputMode::kDecimal)
    {
        PutInput('d');
    }
    else if (mInputMode == InputMode::kHex)
    {
        PutInput('x');
    }
}

void ConsoleMenu::StartParamInput()
{
    const ConsoleMenuCommandInput& input = mCurrentCommand->GetInput(mCurrentParam);
    ++mCurrentParam;
    printf("%s (Param %zu):\n", input.mDescription.c_str(), mCurrentParam);

    ConvertModeTo((input.mInputMode == InputMode::kInvalid ? InputMode::kDecimal : input.mInputMode));
}

void ConsoleMenu::PutInput(char c)
{
    if ((mCurrentPos < sizeof(mInputBuffer)) && (c >= '!') && (c <= '~'))
    {
        putchar(c);
        mInputBuffer[mCurrentPos] = c;
        ++mCurrentPos;
        mInputBuffer[mCurrentPos] = '\0';
    }
}

void ConsoleMenu::BackspaceLastInput()
{
    if (mCurrentPos > 0u)
    {
        printf("\b \b");
        --mCurrentPos;
        mInputBuffer[mCurrentPos] = '\0';
    }
}

void ConsoleMenu::ClearInput()
{
    while (mCurrentPos > 0u)
    {
        printf("\b \b");
        --mCurrentPos;
    }
    mInputBuffer[0] = '\0';
}

void ConsoleMenu::StartNewInputOnNewLine()
{
    mInputBuffer[0] = '\0';
    mCurrentPos = 0;
    putchar('\n');
}

void ConsoleMenu::ExecuteCurrentCommand()
{
    printf("\n");
    mCurrentCommand->Execute();
    printf("\n");

    if (mExitAfterCommandExecution)
    {
        mInputMode = InputMode::kForceExit;
    }
    else
    {
        ResetMenu();
    }
}
