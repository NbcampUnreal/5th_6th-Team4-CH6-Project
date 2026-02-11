#include "AIMonster/BehaviorTree/UK_BTTask_AlertBehavior.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTTask_AlertBehavior::UUK_BTTask_AlertBehavior()
{
	NodeName = "Alert Behavior (Back Off)";
	
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertBehavior, TargetPlayerKey), AActor::StaticClass());
}

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

	// 타겟 플레이어 가져오기
	AActor* TargetPlayer = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
	if (!TargetPlayer) return EBTNodeResult::Failed;

	// 플레이어 반대 방향으로 이동할 위치 계산
	FVector MonsterLocation = ControlledPawn->GetActorLocation();
	FVector PlayerLocation = TargetPlayer->GetActorLocation();
	
	FVector DirectionAwayFromPlayer = (MonsterLocation - PlayerLocation).GetSafeNormal();
	FVector BackOffLocation = MonsterLocation + (DirectionAwayFromPlayer * BackOffDistance);
	
	// 네비게이션 시스템으로 유효한 위치 찾기
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem)
	{
		FNavLocation ResultLocation;
		bool bSuccess = NavSystem->ProjectPointToNavigation(BackOffLocation, ResultLocation, FVector(500, 500, 500));
		
		if (bSuccess)
		{
			// 경계 상태로 변경
			Monster->RequestState(EMonsterState::Alert);
			
			// 뒤로 이동
			AIController->MoveToLocation(ResultLocation.Location, 50.0f);
			
			UE_LOG(LogTemp, Log, TEXT("[Alert] %s backing away from player"), *Monster->GetName());
			
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}