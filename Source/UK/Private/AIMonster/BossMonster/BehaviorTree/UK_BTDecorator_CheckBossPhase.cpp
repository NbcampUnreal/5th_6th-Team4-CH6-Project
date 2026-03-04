#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_CheckBossPhase.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UUK_BTDecorator_CheckBossPhase::UUK_BTDecorator_CheckBossPhase()
{
	NodeName = TEXT("Check Boss Phase");
}

bool UUK_BTDecorator_CheckBossPhase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	AAIController* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	const FName CurrentPhaseName = BB->GetValueAsName(TEXT("BossPhase"));

	return CurrentPhaseName == RequiredPhaseTag.GetTagName();
}