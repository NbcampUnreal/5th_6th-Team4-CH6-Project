// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_GameOverMap.h"
#include "Character/UK_PlayerController.h"
#include "UI/InGame/UK_WarpIcon.h"
#include "Kismet/GameplayStatics.h"

void UUK_GameOverMap::CloseGameOverMap()
{
	if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
	{
		PC->ApplyInputState(EInputState::Game);

		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);

		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	RemoveFromParent();
}
