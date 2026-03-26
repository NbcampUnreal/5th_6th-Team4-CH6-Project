// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_GameOver.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_PlayerController.h"

void UUK_GameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RestartButton )
	{
		RestartButton->OnClicked.AddDynamic(this, &UUK_GameOver::OnRestartButtonClicked);
	}
}


void UUK_GameOver::SetupGameOverUI()
{
	if ( FadeIn )
	{
		PlayAnimation(FadeIn);
	}

	// PlayerController를 통해 입력 상태 관리 (중복 코드 제거)
	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		PC->ApplyInputState(EInputState::UI);
	}

	// 게임 일시정지는 게임 성격에 따라 선택 (워프 기능을 쓰려면 일시정지를 풀거나 맵만 예외처리 해야 함)
	// UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void UUK_GameOver::OnRestartButtonClicked()
{
	AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer());
	if ( !PC || !GameOverMapClass ) return;

	PC->HideGameOverUI();

	UUserWidget* MapWidget = CreateWidget<UUserWidget>(PC, GameOverMapClass);
	if ( MapWidget )
	{
		MapWidget->AddToViewport(0);

		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(MapWidget->TakeWidget());
		PC->SetInputMode(Mode);
	}
}