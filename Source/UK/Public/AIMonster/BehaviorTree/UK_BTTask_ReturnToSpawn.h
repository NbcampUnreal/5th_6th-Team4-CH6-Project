// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_ReturnToSpawn.generated.h"

/**
 * 스폰 위치로 돌아가는 태스크
 */
UCLASS()
class UK_API UUK_BTTask_ReturnToSpawn : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UUK_BTTask_ReturnToSpawn();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector SpawnLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetPlayerKey;
};
