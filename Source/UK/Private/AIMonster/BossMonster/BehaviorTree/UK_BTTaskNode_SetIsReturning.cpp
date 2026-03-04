#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_SetIsReturning.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UUK_BTTaskNode_SetIsReturning::UUK_BTTaskNode_SetIsReturning()
{
	NodeName = TEXT("Set IsReturning");
}

EBTNodeResult::Type UUK_BTTaskNode_SetIsReturning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	BB->SetValueAsBool(TEXT("IsReturning"), true);

	return EBTNodeResult::Succeeded;

}
