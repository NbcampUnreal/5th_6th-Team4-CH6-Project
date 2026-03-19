// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/UK_GameInstance.h"
#include "Systems/Data/UK_SaveInterface.h"
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

void UUK_GameInstance::SetLoadingInputMode(APlayerController* PC)
{
	if (!PC) return;

	PC->bShowMouseCursor = true;

	FInputModeGameAndUI InputModeData;
	if (PersistentLoadingWidget)
	{
		InputModeData.SetWidgetToFocus(PersistentLoadingWidget->TakeWidget());
	}
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	PC->SetInputMode(InputModeData);
}

void UUK_GameInstance::SaveEntireGame()
{
	UUK_InGameSave* SaveInstance = Cast<UUK_InGameSave>(UGameplayStatics::CreateSaveGameObject(UUK_InGameSave::StaticClass()));
	if (!SaveInstance) return;
	
	TArray<AActor*> SaveAbleActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(),UUK_GameInstance::StaticClass(),SaveAbleActors);
	
	for (AActor* Actor : SaveAbleActors)
	{
		IUK_SaveInterface* SaveIntf = Cast<IUK_SaveInterface>(Actor);
		if (SaveIntf)
		{
			SaveIntf->OnSaveGame(SaveInstance);
		}
	}
	
	UGameplayStatics::SaveGameToSlot(SaveInstance, MainSaveSlotName, 0);
}

void UUK_GameInstance::LoadEntireGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(MainSaveSlotName,0))
	{
		return;
	}
	
	UUK_InGameSave* LoadedInstance = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(MainSaveSlotName,0));
	if (!LoadedInstance) return;
	
	TArray<AActor*> SaveAbleActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(),UUK_GameInstance::StaticClass(),SaveAbleActors);
	
	for (AActor* Actor : SaveAbleActors)
	{
		IUK_SaveInterface* SaveIntf = Cast<IUK_SaveInterface>(Actor);
		if (SaveIntf)
		{
			SaveIntf->OnLoadGame(LoadedInstance);
		}
	}
}
