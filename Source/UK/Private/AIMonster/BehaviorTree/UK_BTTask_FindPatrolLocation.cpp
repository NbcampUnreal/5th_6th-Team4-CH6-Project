// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/BehaviorTree/UK_BTTask_FindPatrolLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTTask_FindPatrolLocation::UUK_BTTask_FindPatrolLocation()
{
    NodeName = "Find Patrol Location";
    
    PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindPatrolLocation, PatrolLocationKey));
    SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_FindPatrolLocation, SpawnLocationKey));
}

EBTNodeResult::Type UUK_BTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 스폰 위치 가져오기
    FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
    
    // 몬스터의 순찰 반경 가져오기
    AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
    if (Monster)
    {
        PatrolRadius = Monster->PatrolRadius;
    }

    // 네비게이션 시스템으로 랜덤 위치 찾기
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation ResultLocation;
        bool bSuccess = NavSystem->GetRandomPointInNavigableRadius(SpawnLocation, PatrolRadius, ResultLocation);
        
        if (bSuccess)
        {
            BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, ResultLocation.Location);
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}
