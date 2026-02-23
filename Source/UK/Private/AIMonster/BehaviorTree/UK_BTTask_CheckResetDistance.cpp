#include "AIMonster/BehaviorTree/UK_BTTask_CheckResetDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTTask_CheckResetDistance::UUK_BTTask_CheckResetDistance()
{
	NodeName = "Check Reset Distance";
	
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_CheckResetDistance, SpawnLocationKey));
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_CheckResetDistance, TargetPlayerKey), AActor::StaticClass());
}

EBTNodeResult::Type UUK_BTTask_CheckResetDistance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (!Monster) return EBTNodeResult::Failed;

	// 평화로운 몬스터가 아니면 실행 안 함
	if (Monster->Personality != EMonsterPersonality::Peaceful) 
		return EBTNodeResult::Failed;

	// 적대적이지 않으면 실행 안 함
	if (!Monster->bIsAggressive) 
		return EBTNodeResult::Failed;

	// 스폰 위치 가져오기
	FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	FVector CurrentLocation = ControlledPawn->GetActorLocation();
	
	float DistanceFromSpawn = FVector::Dist(CurrentLocation, SpawnLocation);

	// 리셋 거리를 벗어났는지 체크
	if (DistanceFromSpawn > Monster->ResetDistance)
	{
		// 타겟 클리어
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		
		// 몬스터를 평화로운 상태로 리셋
		Monster->ResetToPassive();
		
		// 스폰 위치로 복귀
		BlackboardComp->SetValueAsVector(TEXT("PatrolLocation"), SpawnLocation);
		
		return EBTNodeResult::Succeeded;
	}

	// 리셋할 필요 없음 - Succeeded 리턴 (Sequence 계속 진행)
	return EBTNodeResult::Succeeded;
}