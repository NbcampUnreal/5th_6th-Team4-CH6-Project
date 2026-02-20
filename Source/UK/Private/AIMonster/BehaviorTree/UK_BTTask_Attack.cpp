#include "AIMonster/BehaviorTree/UK_BTTask_Attack.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UUK_BTTask_Attack::UUK_BTTask_Attack()
{
	NodeName = "Attack (Montage)";
	bNotifyTick = false;         
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UUK_BTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead()) return EBTNodeResult::Failed;

	// ★★★ 이동 완전 정지 (텔레포트 방지)
	AICon->StopMovement();
	if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	// 타겟 방향 회전
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		AActor* TargetPlayer = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
		if (TargetPlayer)
		{
			FVector DirectionToTarget = TargetPlayer->GetActorLocation() - Monster->GetActorLocation();
			DirectionToTarget.Z = 0.f;
			
			if (!DirectionToTarget.IsNearlyZero())
			{
				FRotator TargetRotation = DirectionToTarget.Rotation();
				Monster->SetActorRotation(TargetRotation);
			}
		}
	}

	// 공격 몽타주 재생
	if (!Monster->PlayRandomAttackMontage())
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	Monster->OnAttackFinished.BindUObject(this, &UUK_BTTask_Attack::OnAttackFinished);

	return EBTNodeResult::InProgress;
}

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

EBTNodeResult::Type UUK_BTTask_Attack::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
		if (Monster)
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