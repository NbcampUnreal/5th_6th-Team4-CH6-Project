#include "AIMonster/BehaviorTree/UK_BTTask_GetDown.h"
#include "AIController.h"
#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "AIMonster/Monster/UK_BurrowMonster.h"

UUK_BTTask_GetDown::UUK_BTTask_GetDown()
{
	NodeName            = "Get Down";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UUK_BTTask_GetDown::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	AICon->StopMovement();

	UAnimInstance* AnimInst = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage*  Montage  = Monster->GetDownMontage;

	if (UUKAIMonsterAnimInstance* MonsterAnim = Cast<UUKAIMonsterAnimInstance>(AnimInst))
		MonsterAnim->bShouldEmerge = false;

	if (!AnimInst || !Montage)
	{
		Monster->SetBurrowed(true);
		return EBTNodeResult::Succeeded;
	}

	const float Length = AnimInst->Montage_Play(Montage, 1.0f);
	if (Length <= 0.f)
	{
		Monster->SetBurrowed(true);
		return EBTNodeResult::Succeeded;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UUK_BTTask_GetDown::OnMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, Montage);

	CachedOwnerComp = &OwnerComp;
	return EBTNodeResult::InProgress;
}

void UUK_BTTask_GetDown::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();
	CachedOwnerComp.Reset();

	if (AAIController* AICon = OwnerComp.GetAIOwner())
		if (AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn()))
			Monster->SetBurrowed(true);

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

EBTNodeResult::Type UUK_BTTask_GetDown::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}