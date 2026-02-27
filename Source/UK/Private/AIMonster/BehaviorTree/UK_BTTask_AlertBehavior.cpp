#include "AIMonster/BehaviorTree/UK_BTTask_AlertBehavior.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_AlertBehavior::UUK_BTTask_AlertBehavior()
{
	NodeName = "Alert Behavior (Back Off)";
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertBehavior, TargetPlayerKey), AActor::StaticClass());
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_AlertBehavior::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (!Monster) return EBTNodeResult::Failed;

	AActor* TargetPlayer = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
	if (!TargetPlayer) return EBTNodeResult::Failed;

	// ── 후퇴 위치 계산 ──────────────────────────────────────────────────
	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	const FVector PlayerLocation  = TargetPlayer->GetActorLocation();

	const FVector DirectionAway  = (MonsterLocation - PlayerLocation).GetSafeNormal();
	const FVector BackOffLocation = MonsterLocation + (DirectionAway * BackOffDistance);

	// ── 네비게이션 유효 위치 확인 후 이동 ────────────────────────────────
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ResultLocation;
		if (NavSystem->ProjectPointToNavigation(BackOffLocation, ResultLocation, FVector(500, 500, 500)))
		{
			Monster->RequestState(EMonsterState::Alert);
			AIController->MoveToLocation(ResultLocation.Location, 50.0f);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
#pragma endregion