#include "AIMonster/BehaviorTree/BTDecorator_IsAggressive.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UBTDecorator_IsAggressive::UBTDecorator_IsAggressive()
{
	NodeName = "Is Aggressive";
	FlowAbortMode = EBTFlowAbortMode::Both;
	bNotifyBecomeRelevant = true;
	bNotifyTick = false;
}
#pragma endregion

#pragma region Condition Check
bool UBTDecorator_IsAggressive::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return false;

	APawn* Pawn = AI->GetPawn();
	if (!Pawn) return false;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn);
	if (!Monster) return false;

	return Monster->bIsAggressive;
}
#pragma endregion

#pragma region Observer
void UBTDecorator_IsAggressive::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}
#pragma endregion