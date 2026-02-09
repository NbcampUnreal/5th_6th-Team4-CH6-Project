// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/UK_NPCAIBase.h"
#include "UK_PatrolNPC.generated.h"

/**
 * 
 */
UCLASS()
class UK_API AUK_PatrolNPC : public AUK_NPCAIBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
};
