#include "AIMonster/BehaviorTree/UK_BTTask_AttackBase.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/Monster/UK_EliteMonster.h"
#include "AIMonster/BehaviorTree/UKBTHelpers.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_AttackBase::UUK_BTTask_AttackBase()
{
	NodeName            = "Attack (Base)";
	bNotifyTick         = true;
	bCreateNodeInstance = true;
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_AttackBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || Monster->IsDead()) return EBTNodeResult::Failed;

	// ── 이동 정지 ────────────────────────────────────────────────────────
	AICon->StopMovement();
	UKBTHelpers::StopMovementImmediately(Monster);
	UKBTHelpers::SetOrientToMovement(Monster, false);
	
	if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
	{
		MC->bUseControllerDesiredRotation = false;
	}

	// ── 타겟 방향으로 회전 ────────────────────────────────────────────────
	StopMovementAndRotateToTarget(Monster, OwnerComp);

	// ── 공격 몽타주 재생 (자식 구현) ──────────────────────────────────────
	if (!PlayAttackMontage(Monster))
		return EBTNodeResult::Failed;

	// ── 콜백 등록 ────────────────────────────────────────────────────────
	CachedOwnerComp = &OwnerComp;
	FSimpleDelegate& Delegate = GetAttackDelegate(Monster);
	Delegate.BindUObject(this, &UUK_BTTask_AttackBase::OnAttackComplete);  

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UUK_BTTask_AttackBase::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn()))
		{
			CleanupAttack(Monster);
		}
	}

	CachedOwnerComp.Reset();
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Attack Callback
void UUK_BTTask_AttackBase::OnAttackComplete()  
{
	if (!CachedOwnerComp.IsValid()) return;

	UBehaviorTreeComponent& OwnerComp = *CachedOwnerComp.Get();

	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn()))
		{
			// 공격 종료 시 이동 설정 완전 복원
			if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement     = true;   
				MC->bUseControllerDesiredRotation = false;
				MC->SetMovementMode(MOVE_Walking);          
			}

			// 공격 상태 정리
			Monster->bIsAttacking = false;

			FSimpleDelegate& Delegate = GetAttackDelegate(Monster);
			Delegate.Unbind();
		}
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	CachedOwnerComp.Reset();
}
#pragma endregion

#pragma region Helpers
void UUK_BTTask_AttackBase::StopMovementAndRotateToTarget(AAIMonsterBase* Monster, UBehaviorTreeComponent& OwnerComp)
{
	if (!Monster) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
	if (!Target) return;

	const FVector ToTarget = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	const FRotator TargetRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	Monster->SetActorRotation(TargetRot);
}

void UUK_BTTask_AttackBase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || !Monster->bIsAttacking) return;

	// 스페셜 어택 중에는 회전/추적 없음
	if (AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(Monster))
	{
		if (Elite->bIsSpecialAttacking) return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetPlayer")));
	if (!Target) return;

	const FVector MonsterLoc = Monster->GetActorLocation();
	const FVector TargetLoc  = Target->GetActorLocation();
	const FVector ToTarget   = (TargetLoc - MonsterLoc).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	// ── 공격 중 타겟 방향으로 부드럽게 회전 ──────────────────────────────
	const FRotator CurrentRot  = Monster->GetActorRotation();
	const FRotator DesiredRot  = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	const float    YawDiff     = FMath::Abs(FRotator::NormalizeAxis(DesiredRot.Yaw - CurrentRot.Yaw));

	FRotator NewRot;
	if (YawDiff >= 150.f)
	{
		// 뒤에 있을 때: 콜리전 방향 고착 방지 - 고정 Yaw로 강제 회전
		const float TurnDir = FRotator::NormalizeAxis(DesiredRot.Yaw - CurrentRot.Yaw) > 0.f ? 1.f : -1.f;
		NewRot = FRotator(0.f, CurrentRot.Yaw + TurnDir * Monster->AttackTrackingRotSpeed * 3.f, 0.f);
	}
	else
	{
		NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaSeconds, Monster->AttackTrackingRotSpeed);
	}
	Monster->SetActorRotation(NewRot);

	// ── 사거리 밖이면 타겟 방향으로 소폭 추적 이동 ───────────────────────
	const float Dist = FVector::Dist2D(MonsterLoc, TargetLoc);
	if (Dist > Monster->AttackRange * Monster->AttackTrackingRangeMultiplier) return;

	Monster->AddMovementInput(ToTarget, Monster->AttackTrackingSpeed);
}

void UUK_BTTask_AttackBase::CleanupAttack(AAIMonsterBase* Monster)
{
	if (!Monster) return;

	if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
	{
		MC->bOrientRotationToMovement     = true;   
		MC->bUseControllerDesiredRotation = false;
		MC->SetMovementMode(MOVE_Walking);          
	}

	// 델리게이트 해제
	FSimpleDelegate& Delegate = GetAttackDelegate(Monster);
	Delegate.Unbind();

	// 공격 상태 정리
	Monster->bIsAttacking = false;

	// 몽타주 정지
	if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
	{
		Anim->StopAllMontages(0.25f);
	}
}
#pragma endregion