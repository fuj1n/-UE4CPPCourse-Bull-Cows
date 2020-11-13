// Fill out your copyright notice in the Description page of Project Settings.


#include "Terminal.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"

#include "Cartridge.h"

constexpr TCHAR GPrompt[4] = TEXT("$> ");

// Called when the game starts
void UTerminal::BeginPlay()
{
	Super::BeginPlay();
	UpdateText();
}

void UTerminal::ActivateTerminal()
{
    FInputKeyBinding PressedBinding(EKeys::AnyKey, EInputEvent::IE_Pressed);
    PressedBinding.KeyDelegate.BindDelegate(this, &UTerminal::OnKeyDown);

    FInputKeyBinding RepeatBinding(EKeys::AnyKey, EInputEvent::IE_Repeat);
    RepeatBinding.KeyDelegate.BindDelegate(this, &UTerminal::OnKeyDown);

	if (GetOwner()->InputComponent == nullptr) return;

    PressedBindingIndex = GetOwner()->InputComponent->KeyBindings.Emplace(MoveTemp(PressedBinding));
    RepeatBindingIndex = GetOwner()->InputComponent->KeyBindings.Emplace(MoveTemp(RepeatBinding));
}

void UTerminal::DeactivateTerminal() const
{
	if (GetOwner()->InputComponent == nullptr) return;
	
	// Must do in this order as RepeatBindingIndex > PressedBindingIndex so would change when first is removed
	GetOwner()->InputComponent->KeyBindings.RemoveAt(RepeatBindingIndex);
	GetOwner()->InputComponent->KeyBindings.RemoveAt(PressedBindingIndex);
}

void UTerminal::PrintLine(const FString& Line)
{
	FString Input = Line;
	FString Left, Right;
	while (Input.Split(TEXT("\n"), &Left, &Right))
	{
		Buffer.Emplace(Left);
		Input = Right;
	} 
	Buffer.Emplace(Input);
	UpdateText();
}

void UTerminal::OptionMenu(const TArray<FString>& Options, int32 DefaultOption)
{
    if(Options.Num() == 0)
        return;

    bInMenu = true;
    MenuItems = Options;
    MenuOption = DefaultOption;

    UpdateText();
}

void UTerminal::ClearScreen()
{
	Buffer.Empty();
	UpdateText();
}

FString UTerminal::GetScreenText() const
{
	TArray<FString> FullTerminal = Buffer;

	if(bInMenu) {
	    FullTerminal.Emplace("");

	    for(int32 Index = 0; Index < MenuItems.Num(); Index++) {
	        FString Format = Index == MenuOption ? TEXT("\t<RichText.MenuItem.Selected>[ {0} ]</>") : TEXT("\t<RichText.MenuItem>  {0}  </>");

	        FullTerminal.Emplace(FString::Format(*Format, {MenuItems[Index]}));
	    }

	    Truncate(FullTerminal);
	    return JoinWithNewline(FullTerminal);
	}

	FullTerminal.Add(GPrompt + InputLine);

	// WrapLines
	// Removed in favour of built-in text wrapping that takes words into account rather than slicing mid-word
//	TArray<FString> WrappedLines(WrapLines(FullTerminal));
	Truncate(FullTerminal);

	return JoinWithNewline(FullTerminal);
}

TArray<FString> UTerminal::WrapLines(const TArray<FString>& Lines) const
{
	TArray<FString> WrappedLines;
	for (auto &&Line : Lines)
	{
		FString CurrentLine = Line;
		do
		{
			WrappedLines.Add(CurrentLine.Left(MaxColumns));
			CurrentLine = CurrentLine.RightChop(MaxColumns);
		}
		while (CurrentLine.Len() > 0);
	}
	return WrappedLines;
}

void UTerminal::Truncate(TArray<FString>& Lines) const
{
	while (Lines.Num() > MaxLines)
	{
		Lines.RemoveAt(0);
	}
}

FString UTerminal::JoinWithNewline(const TArray<FString>& Lines) const
{
	FString Result;
	for (auto &&Line : Lines)
	{
		Result += Line + TEXT("\n");
	}
	return Result;
}

void UTerminal::OnKeyDown(FKey Key)
{
    if(bInMenu) {
        if(Key == EKeys::Enter) {
            bInMenu = false;

            InputLine = MenuItems[MenuOption];
            AcceptInputLine();
        } else if(Key == EKeys::Up || Key == EKeys::W) {
            MenuOption--;
        } else if(Key == EKeys::Down || Key == EKeys::S) {
            MenuOption++;
        }

        MenuOption = FMath::Clamp(MenuOption, 0, MenuItems.Num() - 1);
        UpdateText();
        return;
    }

	if (Key == EKeys::Enter)
	{
		AcceptInputLine();
	}

	if (Key == EKeys::BackSpace)
	{
		Backspace();
	}

    const FModifierKeysState KeyState = FSlateApplication::Get().GetModifierKeys();
    const FString KeyString = GetKeyString(Key, KeyState.IsShiftDown());

	if (KeyState.IsShiftDown() || KeyState.AreCapsLocked())
	{
		InputLine += KeyString.ToUpper();
	}
	else
	{
		InputLine += KeyString.ToLower();
	}

	UpdateText();
}


void UTerminal::AcceptInputLine()
{
	Buffer.Emplace(GPrompt + InputLine);
	auto Cartridge = GetOwner()->FindComponentByClass<UCartridge>();
	if (Cartridge != nullptr)
	{
		Cartridge->OnInput(InputLine);
	}
	InputLine = TEXT("");

}

void UTerminal::Backspace()
{
	if (InputLine.Len() > 0)
	{
		InputLine.RemoveAt(InputLine.Len()-1);
	}
}

FString UTerminal::GetKeyString(FKey Key, bool bIsShift)
{
	 const uint32* KeyCode = nullptr;
	 const uint32* CharCode = nullptr;

    FInputKeyManager::Get().GetCodesFromKey(Key, KeyCode, CharCode);

    if (CharCode != nullptr)
	{
        uint32 code = *CharCode;

        if(bIsShift) {
	        switch(code) {
	            case '1':
	                code = '!';
	                break;
	            case '2':
	                code = '@';
	                break;
	            case '3':
	                code = '#';
	                break;
	            case '4':
	                code = '$';
	                break;
	            case '5':
	                code = '%';
	                break;
	            case '6':
	                code = '^';
	                break;
	            case '7':
	                code = '&';
	                break;
	            case '8':
	                code = '*';
	                break;
	            case '9':
	                code = '(';
	                break;
	            case '0':
	                code = ')';
	                break;
	            case '-':
	                code = '_';
	                break;
	            case '=':
	                code = '+';
	                break;
	            default:
	                break;
	        }
	    }

		ANSICHAR Char[2] = {static_cast<ANSICHAR>(code), '\0'};
		return ANSI_TO_TCHAR(Char);
	}

	return TEXT("");
}

void UTerminal::UpdateText()
{
	TextUpdated.Broadcast(GetScreenText());
}