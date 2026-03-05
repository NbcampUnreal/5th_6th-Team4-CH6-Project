#include "AIMonster/BossMonster/BehaviorTree/UK_BTTask_BossAttackPattern.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

UUK_BTTask_BossAttackPattern::UUK_BTTask_BossAttackPattern()
{
	NodeName = "Boss Attack Pattern";
	bNotifyTick = false;
}

EBTNodeResult::Type UUK_BTTask_BossAttackPattern::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory
)
{
	AAIController* AICtl = OwnerComp.GetAIOwner();

	if (!AICtl) return EBTNodeResult::Failed;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());

	if (!Boss) return EBTNodeResult::Failed;

	const bool bSuccess = Boss->PlayRandomAttackMontage();

	return bSuccess ? EBTNodeResult::Succeeded: EBTNodeResult::Failed;
}