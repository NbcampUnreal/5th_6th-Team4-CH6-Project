// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "UK_PeacefulMonster.generated.h"

UCLASS()
class UK_API AUK_PeacefulMonster : public AAIMonsterBase
{
	GENERATED_BODY()

public:
	AUK_PeacefulMonster();

protected:
	virtual void BeginPlay() override;
};
