// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UK_PlayerController_Title.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AUK_PlayerController_Title::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어에서만 UI 생성
	if ( !IsLocalController() )
		return;

	if ( UIWidgetClass )
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		if ( UIWidgetInstance )
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(UIWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void AUK_PlayerController_Title::StartGame(FString Text)
{
	if ( HasAuthority() )
	{
		UWorld* World = GetWorld();
		if ( World )
		{
			World->ServerTravel(TEXT("/Game/Level/Maps/BK_Level?listen"), true);
			UE_LOG(LogTemp, Error, TEXT("Seamless Travel"));
		}
	}
	else
	{
		//멀티
		ClientTravel(Text, TRAVEL_Absolute);
	}
}

void AUK_PlayerController_Title::Server_StartGame_Implementation()
{
	StartGame(TEXT(""));
}