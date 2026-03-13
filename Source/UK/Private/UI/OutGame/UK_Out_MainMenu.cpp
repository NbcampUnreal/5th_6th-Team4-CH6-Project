// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_MainMenu.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/UK_PlayerController_Title.h"
#include "UI/OutGame/UK_Out_CharacterSelect.h"
#include "Systems/Data/UK_SaveGame.h"
#include "Kismet/GameplayStatics.h"

UUK_Out_MainMenu::UUK_Out_MainMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUK_Out_MainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
	}
}

void UUK_Out_MainMenu::OnPlayButtonClicked()
{

	AUK_PlayerController_Title* PlayerController = GetOwningPlayer<AUK_PlayerController_Title>();

	UUK_Out_CharacterSelect* CharacterSelect = CreateWidget<UUK_Out_CharacterSelect>(PlayerController, CharacterSelectWidgetClass);
	if ( !CharacterSelect ) return;

	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());

	if ( UGameplayStatics::DoesSaveGameExist(TEXT("CharacterData"), 0) )
	{
		UUK_SaveGame* LoadedGame = Cast<UUK_SaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("CharacterData"), 0));
		if ( LoadedGame && LoadedGame->SavedCharacterClass )
		{
			GI->CharacterSelected = LoadedGame->SavedCharacterClass;
			PlayerController->StartGame();
		}
	}
	else 
	{
		CharacterSelect->AddToViewport();

		PlayerController->bShowMouseCursor = true;

		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(CharacterSelect->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetHideCursorDuringCapture(false);

		PlayerController->SetInputMode(InputModeData);

		RemoveFromParent();
	}
}

void UUK_Out_MainMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
