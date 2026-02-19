// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/BehaviorTree/UK_BTTask_ReturnToSpawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTTask_ReturnToSpawn::UUK_BTTask_ReturnToSpawn()
{
    NodeName = "Return To Spawn";
    
    SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, SpawnLocationKey));
    TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, TargetPlayerKey), AActor::StaticClass());
}

EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }
	
    FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    float DistanceFromSpawn = FVector::Dist(CurrentLocation, SpawnLocation);
    
    // 스폰 위치 근처에 있으면 복귀 불필요
    if (DistanceFromSpawn < 200.0f) // 200 유닛 이내면 이미 도착
    {
        UE_LOG(LogTemp, Log, TEXT("%s: Already at spawn, no need to return"), *ControlledPawn->GetName());
        return EBTNodeResult::Failed; // Failed 반환 → 다음 Sequence (순찰)로
    }
    
    // 멀리 떨어졌을 때만 복귀 필요
    AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
    if (Monster && DistanceFromSpawn > Monster->MaxChaseDistance)
    {
        // 타겟 플레이어 클리어
        BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
        
        // 순찰 위치를 스폰 위치로 설정
        BlackboardComp->SetValueAsVector(TEXT("PatrolLocation"), SpawnLocation);
        
        UE_LOG(LogTemp, Log, TEXT("%s: Too far from spawn (%.2f), returning"), *ControlledPawn->GetName(), DistanceFromSpawn);
        
        return EBTNodeResult::Succeeded;
    }

    // 복귀 불필요
    return EBTNodeResult::Failed;
}