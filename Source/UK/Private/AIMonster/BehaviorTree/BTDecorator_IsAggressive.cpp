#include "AIMonster/BehaviorTree/BTDecorator_IsAggressive.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"

UBTDecorator_IsAggressive::UBTDecorator_IsAggressive()
{
	NodeName = "Is Aggressive";
	
	//하위 우선순위 브랜치 실행 중에도 즉시 중단
	FlowAbortMode = EBTFlowAbortMode::Both;
	
	//조건 변화 감지를 위한 관찰 활성화
	bNotifyBecomeRelevant = true;
	bNotifyTick = false;
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
	
	return bResult;
}

void UBTDecorator_IsAggressive::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    
	// bIsAggressive 변화 감지 시작
	// ReceiveDamage에서 이미 SetIsAggressive 호출하므로
	// RequestExecution을 거기서 직접 호출하는 방식도 가능
}
