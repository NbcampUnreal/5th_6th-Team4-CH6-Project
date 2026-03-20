#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_ClearTarget.h"

#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossMonster_Grux.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTTaskNode_ClearTarget::UUK_BTTaskNode_ClearTarget()
{
	NodeName = TEXT("Clear Target");
}

EBTNodeResult::Type UUK_BTTaskNode_ClearTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	BB->ClearValue(TEXT("TargetActor"));

	return EBTNodeResult::Succeeded;
}
