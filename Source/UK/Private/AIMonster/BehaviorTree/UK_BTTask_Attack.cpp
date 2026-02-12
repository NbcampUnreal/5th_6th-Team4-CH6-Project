#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName = "Attack (Montage)";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UUK_BTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	if (!Monster->PlayRandomAttackMontage())
		return EBTNodeResult::Failed;

	// 몽타주 재생 중 → InProgress, TickTask에서 종료 감지
	return EBTNodeResult::InProgress;
}

void UUK_BTTask_Attack::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 공격 몽타주 끝나면 성공 완료
	if (!Monster->bIsAttacking)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UUK_BTTask_Attack::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
		if (Monster)
		{
			Monster->bIsAttacking = false;
			if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
			{
				Anim->StopAllMontages(0.25f);
			}
		}
	}
	return EBTNodeResult::Aborted;
}