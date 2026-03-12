// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Character/UK_CharacterBase.h"
#include "UK_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class AUK_CharacterBase> CharacterSelected;


};
