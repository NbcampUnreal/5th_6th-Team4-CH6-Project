// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTaskNode_BossReset.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_BTTaskNode_BossReset : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUK_BTTaskNode_BossReset();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};