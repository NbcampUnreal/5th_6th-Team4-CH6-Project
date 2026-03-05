#include "AIMonster/BehaviorTree/UK_BTTask_CheckResetDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_CheckResetDistance::UUK_BTTask_CheckResetDistance()
{
	NodeName = "Check Reset Distance";

	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_CheckResetDistance, SpawnLocationKey));
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_CheckResetDistance, TargetPlayerKey), AActor::StaticClass());
}
#pragma endregion

#pragma region Reset Distance Check
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

	if (Monster->Personality != EMonsterPersonality::Peaceful) return EBTNodeResult::Failed;
	if (!Monster->bIsAggressive)                               return EBTNodeResult::Failed;

	const FVector SpawnLocation   = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float   DistanceFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	if (DistanceFromSpawn > Monster->ResetDistance)
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		Monster->ResetToPassive();
		BlackboardComp->SetValueAsVector(TEXT("PatrolLocation"), SpawnLocation);
	}

	return EBTNodeResult::Succeeded;
}
#pragma endregion