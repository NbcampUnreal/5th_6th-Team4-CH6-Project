// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/UK_GameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"


void UUK_GameInstance::ShowLoading(float Target)
{
	if ( PersistentLoadingWidget ) return;

	if ( LoadingWidgetClass && GetWorld() )
	{
		PersistentLoadingWidget = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);

		if ( PersistentLoadingWidget && GEngine && GEngine->GameViewport )
		{
			PersistentLoadingWidget->TargetValue = Target;

			GEngine->GameViewport->AddViewportWidgetContent(
				PersistentLoadingWidget->TakeWidget(),
				999
			);
		}
	}
}

void UUK_GameInstance::HideLoading()
{
	if ( PersistentLoadingWidget && GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(
			PersistentLoadingWidget->TakeWidget()
		);

		PersistentLoadingWidget = nullptr;
	}
}
