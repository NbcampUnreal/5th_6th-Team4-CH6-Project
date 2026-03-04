#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_BossAttack.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

UUK_BTTaskNode_BossAttack::UUK_BTTaskNode_BossAttack()
{
	NodeName = TEXT("Boss Attack");
}

EBTNodeResult::Type UUK_BTTaskNode_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return EBTNodeResult::Failed;

	auto* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());

	if (!Boss) return EBTNodeResult::Failed;

	bool ShouldAttack = Boss->PlayRandomAttackMontage();

	return ShouldAttack ?
		EBTNodeResult::Succeeded :
		EBTNodeResult::Failed;
}
