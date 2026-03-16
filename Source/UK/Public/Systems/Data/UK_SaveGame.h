// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Systems/UK_GameInstance.h"
#include "UK_SaveGame.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	TSubclassOf<class AUK_CharacterBase> SavedCharacterClass;
	
};
