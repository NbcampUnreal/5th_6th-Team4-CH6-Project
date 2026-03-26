// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_MainMenu.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/UK_PlayerController_Title.h"
#include "UI/OutGame/UK_Out_CharacterSelect.h"
#include "Systems/Data/UK_SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Systems/UK_GameInstance.h"
#include "UI/OutGame/UK_Out_Loading.h"

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
		StartButton->OnHovered.AddDynamic(this, &ThisClass::OnHoveredButtonClicked);
		UE_LOG(LogTemp, Warning, TEXT("StartButton Bound"));
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
		ExitButton->OnHovered.AddDynamic(this, &ThisClass::OnHoveredButtonClicked);
	}
}

void UUK_Out_MainMenu::OnPlayButtonClicked()
{
	AUK_PlayerController_Title* PlayerController = GetOwningPlayer<AUK_PlayerController_Title>();
	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( !GI || !PlayerController ) return;

	if ( UGameplayStatics::DoesSaveGameExist(TEXT("CharacterData"), 0) )
	{
		UUK_SaveGame* LoadedGame = Cast<UUK_SaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("CharacterData"), 0));
		if ( LoadedGame && LoadedGame->SavedCharacterClass )
		{
			GI->CharacterSelected = LoadedGame->SavedCharacterClass;

			GI->ShowLoading(0.7f);

			RemoveFromParent();

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ PlayerController ] ()
				{
					if ( IsValid(PlayerController) ) PlayerController->StartGame();
				}, 0.1f, false);
		}
	}
	else 
	{
		UUK_Out_CharacterSelect* CharacterSelect = CreateWidget<UUK_Out_CharacterSelect>(PlayerController, CharacterSelectWidgetClass);
		if ( CharacterSelect )
		{
			CharacterSelect->AddToViewport(0);
			PlayerController->bShowMouseCursor = true;

			FInputModeGameAndUI Mode;
			Mode.SetWidgetToFocus(CharacterSelect->TakeWidget());
			PlayerController->SetInputMode(Mode);

			RemoveFromParent();
		}
	}
	
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}

//void UUK_Out_MainMenu::OnPlayButtonClicked()
//{
//
//	AUK_PlayerController_Title* PlayerController = GetOwningPlayer<AUK_PlayerController_Title>();
//
//	UUK_Out_CharacterSelect* CharacterSelect = CreateWidget<UUK_Out_CharacterSelect>(PlayerController, CharacterSelectWidgetClass);
//	if ( !CharacterSelect ) return;
//
//	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
//
//	if ( UGameplayStatics::DoesSaveGameExist(TEXT("CharacterData"), 0) )
//	{
//		UUK_SaveGame* LoadedGame = Cast<UUK_SaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("CharacterData"), 0));
//		if ( LoadedGame && LoadedGame->SavedCharacterClass )
//		{
//			GI->CharacterSelected = LoadedGame->SavedCharacterClass;
//
//			if ( LoadingWidgetClass )
//			{
//				UUK_Out_Loading* Loading = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
//				if ( Loading )
//				{
//					Loading->TargetValue = 0.7f;
//
//					Loading->AddToViewport(999);
//
//					if ( GI )
//					{
//						GI->ShowLoading(0.7f);
//					}
//				}
//			}
//
//			RemoveFromParent();
//
//			if ( IsValid(PlayerController) == true )
//			{
//				FTimerHandle TimerHandle;
//				GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ PlayerController ] ()
//					{
//						if ( IsValid(PlayerController) )
//						{
//							PlayerController->StartGame();
//						}
//					}, 0.1f, false);
//			}
//		}
//	}
//	else 
//	{
//		CharacterSelect->AddToViewport();
//
//		PlayerController->bShowMouseCursor = true;
//
//		FInputModeGameAndUI InputModeData;
//		InputModeData.SetWidgetToFocus(CharacterSelect->TakeWidget());
//		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
//		InputModeData.SetHideCursorDuringCapture(false);
//
//		PlayerController->SetInputMode(InputModeData);
//
//		RemoveFromParent();
//	}
//}

void UUK_Out_MainMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
	
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
}

void UUK_Out_MainMenu::OnHoveredButtonClicked()
{
	if (HoveredSound)
	{
		UGameplayStatics::PlaySound2D(this, HoveredSound);
	}
}
