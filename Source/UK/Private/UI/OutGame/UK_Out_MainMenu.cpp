// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_MainMenu.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/UK_PlayerController.h"

UUK_Out_MainMenu::UUK_Out_MainMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUK_Out_MainMenu::NativeConstruct()
{
	StartButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UUK_Out_MainMenu::OnPlayButtonClicked()
{
	AUK_PlayerController* PlayerController = GetOwningPlayer<AUK_PlayerController>();
	if ( IsValid(PlayerController) == true )
	{
		FText ServerIP = ServerIPEditableText->GetText();
		// 플레이어 시작 위치
		//PlayerController->RequestStartGame();
	}
}

void UUK_Out_MainMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
