#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UUK_BTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	UE_LOG(LogTemp, Warning, TEXT("BTTask_Attack Executed"));
	AAIController* AICon = OwnerComp.GetAIOwner();
	if ( !AICon )
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if ( !Pawn )
	{
		return EBTNodeResult::Failed;
	}

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn);
	if ( !Monster )
	{
		return EBTNodeResult::Failed;
	}

	// 실제 공격 호출
	Monster->OnAttack();

	return EBTNodeResult::Succeeded;
}