// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Console/Cartridge.h"
#include "BullCowCartridge.generated.h"

enum class EGameState : uint8 {
    Menu,
    Instructions,
    Difficulty,
    Game,
    EndGame
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BULLCOWGAME_API UBullCowCartridge : public UCartridge {
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;

    virtual void OnInput(const FString& Input) override;

private:
    void MainMenu();
    void SetupGame();
    void ProcessGuess(const FString& Guess);
    void EndGame(const FString& EndMessage);

    static bool IsIsogram(const FString& Input);
    void GetBullCows(const FString& Guess, int32& BullCount, int32& CowCount) const;
private:
    EGameState GameState;

    FString HiddenWord;
    int32 Lives;
    int32 Difficulty;
    TSet<FString> PreviousGuesses;

    TArray<FString> Words;
};