// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/UK_GameInstance.h"
#include "Systems/Data/UK_SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/InGame/Quest/UK_QuestMain.h"

void UUK_GameInstance::Init()
{
	Super::Init();

	/*FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UUK_GameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UUK_GameInstance::EndLoadingScreen);*/
}

void UUK_GameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	Super::Shutdown();
}

void UUK_GameInstance::ShowLoading(float Target)
{
	// 1. 이미 위젯이 존재하는 경우 값만 업데이트

	if ( auto PC = Cast<AUK_PlayerController>(GetFirstLocalPlayerController()) )
	{
		PC->ClearAllWidgets(); // 기존 UI 모두 제거
		PC->ApplyInputState(EInputState::UI); // 입력 UI 모드
	}

	if ( PersistentLoadingWidget && PersistentLoadingWidget->IsInViewport() )
	{
		PersistentLoadingWidget->TargetValue = Target;
		return;
	}

	// 2. 위젯이 없거나 뷰포트에 없는 경우 생성 및 설정
	if ( LoadingWidgetClass )
	{
		//// 이전에 남은 위젯이 있다면 정리 (안전장치)
		//if ( PersistentLoadingWidget )
		//{
		//	HideLoading();
		//}

		PersistentLoadingWidget = CreateWidget<UUK_Out_Loading>(this, LoadingWidgetClass);

		if ( PersistentLoadingWidget )
		{

			PersistentLoadingWidget->TargetValue = Target;

			if ( GEngine && GEngine->GameViewport )
			{
				GEngine->GameViewport->AddViewportWidgetContent(
					PersistentLoadingWidget->TakeWidget(),
					10
				);
			}

			PersistentLoadingWidget->ForceLayoutPrepass();

			if ( auto PC = Cast<AUK_PlayerController>(GetFirstLocalPlayerController()) )
			{
				PC->ApplyInputState(EInputState::UI);
			}
		}
	}
}

void UUK_GameInstance::HideLoading()
{
	if ( PersistentLoadingWidget )
	{
		if ( auto PC = Cast<AUK_PlayerController>(GetFirstLocalPlayerController()) )
		{
			PC->ApplyInputState(EInputState::Game);
		}

		if ( GEngine && GEngine->GameViewport )
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(PersistentLoadingWidget->TakeWidget());
		}

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
	if (!GetWorld() || IsEngineExitRequested() || GIsSlowTask || !GIsRunning)
	{
		return; 
	}
	
	if (GetWorld()->bIsTearingDown) 
	{
		return;
	}
	
	UUK_InGameSave* SaveInstance = Cast<UUK_InGameSave>(UGameplayStatics::CreateSaveGameObject(UUK_InGameSave::StaticClass()));
	if (!SaveInstance) return;
	
	if (UUKQuestManagerSubsystem* QuestSubsystem = GetSubsystem<UUKQuestManagerSubsystem>())
	{
		SaveInstance->QuestProgressMap = QuestSubsystem->RuntimeProgress;
	}
	TArray<AActor*> SaveAbleActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(),UUK_SaveInterface::StaticClass(),SaveAbleActors);
	
	for (AActor* Actor : SaveAbleActors)
	{
		if (IsValid(Actor) && !Actor->IsUnreachable())
		{
			IUK_SaveInterface* SaveIntf = Cast<IUK_SaveInterface>(Actor);
			if (SaveIntf)
			{
				SaveIntf->OnSaveGame(SaveInstance);
			}
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
	
	if (UUKQuestManagerSubsystem* QuestSubsystem = GetSubsystem<UUKQuestManagerSubsystem>())
	{
		QuestSubsystem->RuntimeProgress = LoadedInstance->QuestProgressMap;
	}
	TArray<AActor*> SaveAbleActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(),UUK_SaveInterface::StaticClass(),SaveAbleActors);
	
	for (AActor* Actor : SaveAbleActors)
	{
		IUK_SaveInterface* SaveIntf = Cast<IUK_SaveInterface>(Actor);
		if (SaveIntf)
		{
			SaveIntf->OnLoadGame(LoadedInstance);
		}
	}

	//시작하면 기존에 저장된 값 불러오는 법
	if ( UWorld* World = GetWorld() )
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UUK_QuestMain::StaticClass(), false);

		for ( UUserWidget* Widget : FoundWidgets )
		{
			if ( UUK_QuestMain* QuestMain = Cast<UUK_QuestMain>(Widget) )
			{
				QuestMain->RefreshQuestList();
			}
		}
	}
}

void UUK_GameInstance::RequestSaveAndQuit()
{
	SaveEntireGame();

	APlayerController* PC = GetFirstLocalPlayerController();
	UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
}
