// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UK_BTTask_FindPatrolLocation.generated.h"

/**
 * 스폰 위치 주변에서 순찰 위치를 찾는 태스크
 */
UCLASS()
class UK_API UUK_BTTask_FindPatrolLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UUK_BTTask_FindPatrolLocation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector SpawnLocationKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float PatrolRadius = 1000.0f;
};
