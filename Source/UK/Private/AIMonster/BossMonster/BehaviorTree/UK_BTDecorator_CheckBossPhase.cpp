#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_CheckBossPhase.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

UUK_BTDecorator_CheckBossPhase::UUK_BTDecorator_CheckBossPhase()
{
	NodeName = TEXT("Check Boss Phase");
}

bool UUK_BTDecorator_CheckBossPhase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if ( !AIC ) return false;

	AUK_BossMonsterBase* Boss =
		Cast<AUK_BossMonsterBase>(AIC->GetPawn());

	if ( !Boss ) return false;

	return Boss->GetCurrentPhase() == RequiredPhase;
}