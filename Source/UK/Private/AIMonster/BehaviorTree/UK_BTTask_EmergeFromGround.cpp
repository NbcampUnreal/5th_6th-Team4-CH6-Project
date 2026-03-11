#include "AIMonster/BehaviorTree/UK_BTTask_EmergeFromGround.h"
#include "AIController.h"
#include "AIMonster/Monster/UK_BurrowMonster.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_EmergeFromGround::UUK_BTTask_EmergeFromGround()
{
	NodeName            = "Emerge From Ground";
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region Task Interface
EBTNodeResult::Type UUK_BTTask_EmergeFromGround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	// 이동 활성화
	Monster->SetBurrowed(false);
	AICon->StopMovement();

	UAnimInstance* AnimInst = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage*  Montage  = Monster->EmergeMontage;

	// 몽타주 없으면 즉시 완료
	if (!AnimInst || !Montage)
	{
		return EBTNodeResult::Succeeded;
	}

	const float Length = AnimInst->Montage_Play(Montage, 1.0f);
	if (Length <= 0.f)
	{
		return EBTNodeResult::Succeeded;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UUK_BTTask_EmergeFromGround::OnMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, Montage);

	CachedOwnerComp = &OwnerComp;
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UUK_BTTask_EmergeFromGround::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AUK_BurrowMonster* Monster = Cast<AUK_BurrowMonster>(AICon->GetPawn()))
		{
			if (UAnimInstance* AnimInst = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr)
			{
				CleanupMontageDelegate(AnimInst, Monster->EmergeMontage);
				AnimInst->StopAllMontages(0.15f);
			}
			// Abort 시에도 이동은 열어둠 (이미 땅 위에 있는 상태)
			Monster->SetBurrowed(false);
		}
	}

	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Internal
void UUK_BTTask_EmergeFromGround::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();
	CachedOwnerComp.Reset();

	FinishLatentTask(OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
}

void UUK_BTTask_EmergeFromGround::CleanupMontageDelegate(UAnimInstance* AnimInst, UAnimMontage* Montage)
{
	if (!AnimInst || !Montage) return;

	FOnMontageEnded EmptyDelegate;
	AnimInst->Montage_SetEndDelegate(EmptyDelegate, Montage);
}
#pragma endregion