// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UKGameModeBase_Title.generated.h"

/**
 * 
 */
UCLASS()
class UK_API AUKGameModeBase_Title : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUKGameModeBase_Title();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;
	
};
