// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"
#include "Kismet/GameplayStatics.h"

UUK_BTService_DetectPlayer::UUK_BTService_DetectPlayer()
{
    NodeName = "Detect Player";
    Interval = 0.5f; // 0.5초마다 체크
    RandomDeviation = 0.1f;
    
    TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, TargetPlayerKey), AActor::StaticClass());
    SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, SpawnLocationKey));
}

void UUK_BTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return;
    }

    // 몬스터의 감지 반경과 최대 추적 거리 가져오기
    AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
    if (Monster)
    {
        DetectionRadius = Monster->DetectionRadius;
        
        // 스폰 위치 가져오기
        FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
        float DistanceFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);
        
        // 최대 추적 거리를 벗어났다면 타겟 제거
        if (DistanceFromSpawn > Monster->MaxChaseDistance)
        {
            BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
            UE_LOG(LogTemp, Log, TEXT("%s: Too far from spawn, clearing target"), *Monster->GetName());
            return;
        }
    }

    // 플레이어 찾기
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());
        
        if (Distance <= DetectionRadius)
        {
            // 플레이어 발견
            BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, PlayerCharacter);
        }
        else
        {
            // 범위 밖
            BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
        }
    }
    else
    {
        BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
    }
}
