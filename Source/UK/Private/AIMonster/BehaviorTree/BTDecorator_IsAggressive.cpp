#include "AIMonster/BehaviorTree/BTDecorator_IsAggressive.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"

UBTDecorator_IsAggressive::UBTDecorator_IsAggressive()
{
	NodeName = "Is Aggressive";
}

bool UBTDecorator_IsAggressive::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return false;

	APawn* Pawn = AI->GetPawn();
	if (!Pawn) return false;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn);
	if (!Monster) return false;

	// 적대적인 상태인지 체크
	bool bResult = Monster->bIsAggressive;
	
	UE_LOG(LogTemp, Warning, TEXT("[IsAggressive Decorator] %s | bIsAggressive=%d | Inverse=%d | FinalResult=%d"), 
		*Monster->GetName(), 
		bResult, 
		IsInversed(), 
		IsInversed() ? !bResult : bResult);
	
	return bResult;
}