#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_BossAttack.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

UUK_BTTaskNode_BossAttack::UUK_BTTaskNode_BossAttack()
{
	NodeName = TEXT("Boss Attack");
	bNotifyTick = true;
	AttackDuration = 2.0f;
	CurrentTime = 0.f;
}

EBTNodeResult::Type UUK_BTTaskNode_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICtl = OwnerComp.GetAIOwner();
	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());

	if (!Boss || Boss->bIsAttacking) return EBTNodeResult::Failed;
	
	AICtl->StopMovement();
	bool bAttackStarted = Boss->PlayRandomAttackMontage();
    
	if (bAttackStarted)
	{
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

void UUK_BTTaskNode_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds)
{
	auto* AICtl = OwnerComp.GetAIOwner();
	if (!AICtl) return;
	
	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(OwnerComp.GetAIOwner()->GetPawn());

	if (Boss && Boss->bIsAttacking)
	{
		AICtl->StopMovement();
	}
	else if (Boss && Boss->bIsAttacking == false)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
