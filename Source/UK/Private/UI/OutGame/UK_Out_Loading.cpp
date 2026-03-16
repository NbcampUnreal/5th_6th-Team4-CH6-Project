// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_Loading.h"
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
		// 0.5초 뒤에 꺼지도록 타이머 설정
		FTimerHandle FinishTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(FinishTimerHandle, this, &UUK_Out_Loading::HandleLoadingComplete, 0.5f, false);
	}
}

void UUK_Out_Loading::HandleLoadingComplete()
{
	UUK_GameInstance* GI = Cast<UUK_GameInstance>(GetGameInstance());
	if ( GI )
	{
		GI->PersistentLoadingWidget = nullptr;
	}

	APlayerController* PC = GetOwningPlayer();
	if ( PC )
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}
