#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#pragma region Initialization
UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName           = "Attack (Montage)";
	bNotifyTick        = false;
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead()) return EBTNodeResult::Failed;

	// ── 이동 정지 ────────────────────────────────────────────────────────
	AICon->StopMovement();
	if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	// ── 타겟 방향 회전 ────────────────────────────────────────────────────
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		if (AActor* TargetPlayer = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer"))))
		{
			FVector Dir = TargetPlayer->GetActorLocation() - Monster->GetActorLocation();
			Dir.Z = 0.f;
			if (!Dir.IsNearlyZero())
			{
				Monster->SetActorRotation(Dir.Rotation());
			}
		}
	}

	// ── 공격 몽타주 재생 ─────────────────────────────────────────────────
	if (!Monster->PlayRandomAttackMontage())
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	Monster->OnAttackFinished.BindUObject(this, &UUK_BTTask_Attack::OnAttackFinished);

	return EBTNodeResult::InProgress;
}
#pragma endregion

#pragma region Attack Callback
void UUK_BTTask_Attack::OnAttackFinished(bool bSucceeded)
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();

	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn()))
		{
			Monster->OnAttackFinished.Unbind();
		}
	}

	FinishLatentTask(OwnerComp, bSucceeded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	CachedOwnerComp.Reset();
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn()))
		{
			Monster->OnAttackFinished.Unbind();
			Monster->bIsAttacking = false;
			if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
			{
				Anim->StopAllMontages(0.25f);
			}
		}
	}

	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}
#pragma endregion