// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UKGameStateBase.generated.h"

UCLASS()
class UK_API AUKGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AUKGameStateBase();

	// “모두가 봐야 하는” 공용 상태 예시
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ServerBase")
	bool bIsWorldPublic = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};