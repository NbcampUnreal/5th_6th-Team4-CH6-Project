// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_Loading.h"
#include "Character/UK_PlayerController.h"
#include "Systems/UK_GameInstance.h"

void UUK_Out_Loading::LoadingLogoFunc(const FGeometry& MyGeometry, float InDeltaTime)
{
	CurrentPercentage = FMath::FInterpTo(CurrentPercentage, TargetValue, InDeltaTime, InterpSpeed);

	if ( TargetLogoWidget )
	{
		TargetLogoWidget->UpdatePercentage(CurrentPercentage);
	}

	if ( CurrentPercentage >= 0.99f && TargetValue >= 1.0f )
	{
		
		GetWorld()->GetTimerManager().SetTimer(FinishTimerHandle, this, &UUK_Out_Loading::HandleLoadingComplete, 0.5f, false);
	}
}

void UUK_Out_Loading::HandleLoadingComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Complete! Removing Widget..."));

	GetWorld()->GetTimerManager().ClearTimer(FinishTimerHandle);

	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( GI )
	{
		GI->PersistentLoadingWidget = nullptr;
	}

	AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetWorld()->GetFirstPlayerController());
	if ( PC )
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
		PC->ApplyInputState(EInputState::Game);
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}

	RemoveFromParent();
}
