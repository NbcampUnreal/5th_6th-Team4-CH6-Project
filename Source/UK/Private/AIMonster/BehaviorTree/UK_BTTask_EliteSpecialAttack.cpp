#include "AIMonster/BehaviorTree/UK_BTTask_EliteSpecialAttack.h"
#include "AIController.h"
#include "AIMonster/Monster/UK_EliteMonster.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_EliteSpecialAttack::UUK_BTTask_EliteSpecialAttack()
{
	NodeName            = "Elite Special Attack";
	bNotifyTick         = false;
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_EliteSpecialAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn());
	if (!Elite || Elite->IsDead()) return EBTNodeResult::Failed;

	if (!Elite->CanUseSpecialAttack()) return EBTNodeResult::Failed;

	// ── 이동 정지 ────────────────────────────────────────────────────────
	AICon->StopMovement();
	if (UCharacterMovementComponent* MC = Elite->GetCharacterMovement())
	{
		MC->StopMovementImmediately();
	}

	// ── 타겟 방향 회전 ────────────────────────────────────────────────────
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer"))))
		{
			FVector Dir = Target->GetActorLocation() - Elite->GetActorLocation();
			Dir.Z = 0.f;
			if (!Dir.IsNearlyZero())
			{
				Elite->SetActorRotation(Dir.Rotation());
			}
		}
	}

	// ── 특수 공격 실행 ────────────────────────────────────────────────────
	if (!Elite->PlaySpecialAttack()) return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;
	Elite->OnSpecialAttackFinished.BindUObject(this, &UUK_BTTask_EliteSpecialAttack::OnSpecialAttackFinished);

	return EBTNodeResult::InProgress;
}
#pragma endregion

#pragma region Special Attack Callback
void UUK_BTTask_EliteSpecialAttack::OnSpecialAttackFinished(bool bSucceeded)
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();

	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn()))
		{
			Elite->OnSpecialAttackFinished.Unbind();
		}
	}

	FinishLatentTask(OwnerComp, bSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	CachedOwnerComp.Reset();
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_EliteSpecialAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(AICon->GetPawn()))
		{
			Elite->OnSpecialAttackFinished.Unbind();
			Elite->bIsAttacking = false;
			if (UAnimInstance* Anim = Elite->GetMesh()->GetAnimInstance())
			{
				Anim->StopAllMontages(0.25f);
			}
		}
	}

	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}
#pragma endregion