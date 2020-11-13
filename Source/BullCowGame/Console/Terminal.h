// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Terminal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTextUpdateSignature, FString, Text);

struct FKey;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BULLCOWGAME_API UTerminal : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Terminal")
	FTextUpdateSignature TextUpdated;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScreenText() const;

	UFUNCTION(BlueprintCallable)
	void ActivateTerminal();

	UFUNCTION(BlueprintCallable)
	void DeactivateTerminal() const;

	void PrintLine(const FString& Line);
	void ClearScreen();

	void OptionMenu(const TArray<FString>& Options, int32 DefaultOption);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void OnKeyDown(FKey Key);
	TArray<FString> WrapLines(const TArray<FString>&  Lines) const;
	void Truncate(TArray<FString>& Lines) const;
	FString JoinWithNewline(const TArray<FString>& Lines) const;
	void AcceptInputLine();
	void Backspace();
	static FString GetKeyString(FKey Key, bool bIsShift) ;
	void UpdateText();


	UPROPERTY(EditAnywhere)
	int32 MaxLines = 20;

	UPROPERTY(EditAnywhere)
	int32 MaxColumns = 70;

	TArray<FString> Buffer;
	FString InputLine;

	int32 PressedBindingIndex;
	int32 RepeatBindingIndex;

	// Menu system
	bool bInMenu = false;
	TArray<FString> MenuItems;
	int32 MenuOption;
};
