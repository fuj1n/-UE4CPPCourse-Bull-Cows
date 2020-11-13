#include "BullCowCartridge.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define DEBUG_CARTRIDGE 1
#define FMT(text, ...) FString::Format(TEXT(text), {__VA_ARGS__})


void UBullCowCartridge::BeginPlay() { // When the game starts
    Super::BeginPlay();

    const FString WordListPath = FPaths::ProjectContentDir() / TEXT("WordLists/HiddenWordList.txt");
    FFileHelper::LoadFileToStringArray(Words, *WordListPath);
    Words.RemoveAll([](const FString& Str) { return !(Str.Len() >= 4 && Str.Len() <= 8 && IsIsogram(Str)); });

#if DEBUG_CARTRIDGE
    PrintLine(FMT("<RichText.Debug>[DBG] Loaded {0} valid words.</>", Words.Num()));
#endif

    MainMenu();
}

void UBullCowCartridge::OnInput(const FString& Input) { // When the player hits enter
    switch(GameState) {
        case EGameState::Game:
            ProcessGuess(Input.TrimStartAndEnd().ToLower());
            break;
        case EGameState::Menu:
            ClearScreen();

            if(Input.Equals(TEXT("Play"))) {
                GameState = EGameState::Difficulty;

                PrintLine(TEXT("Select difficulty:\n"));
                OptionMenu({TEXT("Easy"), TEXT("Medium"), TEXT("Hard")}, 1);
            } else if (Input.Equals(TEXT("Instructions"))) {
                GameState = EGameState::Instructions;

                PrintLine(TEXT("<RichText.Small>%s%s%s%s%s%s</>"),
                          TEXT("The game is rather simple, at the beginning, an isogram will be chosen as a hidden word "),
                          TEXT("and you will be given the number of letters in it, you then have to type in your guess,"),
                          TEXT("if you guess wrong, the game will give you a count of bulls and cows, every bull is a "),
                          TEXT("right letter in the right place, and every cow is a right letter in the wrong place. "),
                          TEXT("Wrong guesses deduct lives.</>\n\n<RichText.Small>"),
                          TEXT("An isogram is a word with no repeating letters."));

                PrintLine(TEXT("Press enter to continue..."));
            } else if (Input.Equals(TEXT("Exit"))) {
                FGenericPlatformMisc::RequestExit(false);
            }
            break;
        case EGameState::Instructions:
            ClearScreen();
            MainMenu();
            break;
        case EGameState::Difficulty:
            ClearScreen();
            if(Input.Equals(TEXT("Easy"))) {
                Difficulty = 3;
            } else if(Input.Equals(TEXT("Medium"))) {
                Difficulty = 2;
            } else {
                Difficulty = 1;
            }

            SetupGame();
            break;
        case EGameState::EndGame:
            ClearScreen();

            if(Input.Equals(TEXT("Yes"))) {
                SetupGame();
            } else {
                MainMenu();
            }
            break;
    }
}

void UBullCowCartridge::MainMenu() {
    GameState = EGameState::Menu;

    PrintLine(TEXT("\n\n\n\n<RichText.Hidden><logo></>"));
    OptionMenu({TEXT("Play"), TEXT("Instructions"), TEXT("Exit")});
}

void UBullCowCartridge::SetupGame() {
    GameState = EGameState::Game;

    HiddenWord = Words[FMath::RandHelper(Words.Num())];
    Lives = HiddenWord.Len() * Difficulty;
    PreviousGuesses.Empty();

    PrintLine(FMT("The hidden word is {0} letters long.", HiddenWord.Len()));
    PrintLine(FMT("You have {0} lives remaining.", Lives));

#if DEBUG_CARTRIDGE
    PrintLine(FMT("<RichText.Debug>[DBG] The hidden word is '{0}'.</>", HiddenWord));
#endif

    PrintLine(TEXT("\nType in your guess and press enter to continue..."));
}

void UBullCowCartridge::ProcessGuess(const FString& Guess) {
    // Comparing two ints is faster than comparing strings
    if (Guess.Len() != HiddenWord.Len()) {
        PrintLine(FMT("The hidden word is {0} letters long, try again!", HiddenWord.Len()));
        return;
    }

    if (!IsIsogram(Guess)) {
        PrintLine(TEXT("The entered word is not an isogram."));
        PrintLine(TEXT("An isogram is a word that has no repeating letters.\nTry again!"));
        return;
    }

    if (Guess.Equals(HiddenWord, ESearchCase::IgnoreCase)) {
        ClearScreen();
        EndGame(TEXT("You win!"));
    } else {
        if(PreviousGuesses.Contains(Guess)) {
            int32 Bulls, Cows;
            GetBullCows(Guess, Bulls, Cows);

            PrintLine(FMT("You have already tried {0}. It resulted in {1} bulls and {2} cows.", Guess, Bulls, Cows));
            return;
        }

        PreviousGuesses.Emplace(Guess);
        --Lives;

        if (Lives <= 0) {
            ClearScreen();
            PrintLine(FMT("Incorrect, the correct answer was \"{0}\".", HiddenWord));
            EndGame(TEXT("Game over!"));
        } else {
            int32 Bulls, Cows;
            GetBullCows(Guess, Bulls, Cows);

            PrintLine(FMT("Incorrect, you have {0} bulls and {1} cows.\nPlease try again.", Bulls, Cows));
            PrintLine(FMT("You have {0} lives remaining.", Lives));
        }
    }
}

void UBullCowCartridge::EndGame(const FString& EndMessage) {
    GameState = EGameState::EndGame;
    PrintLine(EndMessage);
    PrintLine(TEXT("\nWould you like to play again?"));
    OptionMenu({TEXT("Yes"), TEXT("No")});
}

bool UBullCowCartridge::IsIsogram(const FString& Input) {
    TSet<TCHAR> UsedChars;
    UsedChars.Reserve(Input.Len());

    for (TCHAR c : Input) {
        if (UsedChars.Contains(c)) {
            return false;
        }

        UsedChars.Add(c);
    }

    return true;
}

void UBullCowCartridge::GetBullCows(const FString& Guess, int32& Bulls, int32& Cows) const {
    Bulls = Cows = 0;

    TSet<TCHAR> AllChars;
    for(TCHAR c : HiddenWord) {
        AllChars.Add(c);
    }

    for(int Index = 0; Index < Guess.Len(); Index++) {
        if(HiddenWord[Index] == Guess[Index]) {
            Bulls++;
        } else if (AllChars.Contains(Guess[Index])) {
            Cows++;
        }
    }
}

#undef DEBUG_CARTRIDGE
#undef FMT