// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OutGame/UK_Out_CharacterSelect.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/UK_PlayerController_Title.h"

UUK_Out_CharacterSelect::UUK_Out_CharacterSelect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUK_Out_CharacterSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartButtonClicked);
	}
}

void UUK_Out_CharacterSelect::OnStartButtonClicked()
{
	AUK_PlayerController_Title* PlayerController = GetOwningPlayer<AUK_PlayerController_Title>();
	
	if ( IsValid(PlayerController) == true )
	{
		//FText ServerIP = ServerIPEditableText->GetText();
		PlayerController->StartGame();
	}
}