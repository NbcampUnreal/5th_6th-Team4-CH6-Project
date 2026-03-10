#include "AIMonster/BossMonster/BehaviorTree/UK_BTTaskNode_BossAttack.h"
#include "AIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossAnimInstance.h"

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
	if (!AICtl) return EBTNodeResult::Failed;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());
	if (!Boss) return EBTNodeResult::Failed;

	CurrentTime = 0.f;
	Boss->bIsAttacking = false; 
	
	bool bAttackStarted = Boss->PlayRandomAttackMontage();
    
	if (!bAttackStarted)
	{
		return EBTNodeResult::Failed;
	}
	
	return EBTNodeResult::InProgress;
}

void UUK_BTTaskNode_BossAttack::TickTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds)
{
	CurrentTime += DeltaSeconds;

	if (CurrentTime >= AttackDuration)
	{
		auto* AICtl = OwnerComp.GetAIOwner();
		if (!AICtl) return;

		auto* Boss = Cast<AUK_BossMonsterBase>(AICtl->GetPawn());
		if (!Boss) return;
		
		auto* Anim = Cast<UUK_BossAnimInstance>(Boss->GetMesh()->GetAnimInstance());
		if (Anim)
		{
			Anim->bIsAttacking = false;
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
