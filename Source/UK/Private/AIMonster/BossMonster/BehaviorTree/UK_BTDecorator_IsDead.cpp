#include "AIMonster/BossMonster/BehaviorTree/UK_BTDecorator_IsDead.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UUK_BTDecorator_IsDead::UUK_BTDecorator_IsDead()
{
	NodeName = TEXT("Is Dead");
}

bool UUK_BTDecorator_IsDead::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) const
{
	AAIController* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return false;

	APawn* Pawn = AICtl->GetPawn();
	if (!Pawn) return false;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Pawn);
	if (!Monster) return false;

	return Monster->IsDead();
}
