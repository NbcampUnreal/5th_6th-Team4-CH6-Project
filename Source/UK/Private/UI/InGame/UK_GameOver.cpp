// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_GameOver.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UUK_GameOver::NativeConstruct()
{
	Super::NativeConstruct();

	if ( RestartButton )
	{
		RestartButton->OnClicked.AddDynamic(this, &ThisClass::OnRestartButtonClicked);
	}
	if ( ExitButton )
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnReExitButtonClicked);
	}
}

void UUK_GameOver::OnRestartButtonClicked()
{
	//Restart할 경우 저장된 위치나 특정 장면으로 돌아가게 설정
}

void UUK_GameOver::OnReExitButtonClicked()
{
	UGameplayStatics::OpenLevel(this, FName("MainMenu_Level"));
}
