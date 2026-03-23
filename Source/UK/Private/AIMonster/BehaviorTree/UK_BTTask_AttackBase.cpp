#include "AIMonster/BehaviorTree/UK_BTTask_AttackBase.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/Monster/UK_EliteMonster.h"
#include "AIMonster/BehaviorTree/UKBTHelpers.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

static const FName TargetPlayerKeyName(TEXT("TargetPlayer"));

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

	AICon->StopMovement();
	UKBTHelpers::StopMovementImmediately(Monster);
	UKBTHelpers::SetOrientToMovement(Monster, false);

	if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
		MC->bUseControllerDesiredRotation = false;

	StopMovementAndRotateToTarget(Monster, OwnerComp);

	if (!PlayAttackMontage(Monster))
		return EBTNodeResult::Failed;

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
			CleanupAttack(Monster);
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
			if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement     = true;
				MC->bUseControllerDesiredRotation = false;
				MC->SetMovementMode(MOVE_Walking);
			}

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

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKeyName));
	if (!Target) return;

	const FVector ToTarget = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	Monster->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));
}

void UUK_BTTask_AttackBase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(AICon->GetPawn());
	if (!Monster || !Monster->bIsAttacking) return;

	if (AUK_EliteMonster* Elite = Cast<AUK_EliteMonster>(Monster))
	{
		if (Elite->bIsSpecialAttacking) return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKeyName));
	if (!Target) return;

	const FVector MonsterLoc = Monster->GetActorLocation();
	const FVector TargetLoc  = Target->GetActorLocation();
	const FVector ToTarget   = (TargetLoc - MonsterLoc).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return;

	const FRotator CurrentRot = Monster->GetActorRotation();
	const FRotator DesiredRot = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	const float    YawDiff    = FMath::Abs(FRotator::NormalizeAxis(DesiredRot.Yaw - CurrentRot.Yaw));

	FRotator NewRot;
	if (YawDiff >= 150.f)
	{
		const float TurnDir = FRotator::NormalizeAxis(DesiredRot.Yaw - CurrentRot.Yaw) > 0.f ? 1.f : -1.f;
		NewRot = FRotator(0.f, CurrentRot.Yaw + TurnDir * Monster->AttackTrackingRotSpeed * 3.f, 0.f);
	}
	else
	{
		NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaSeconds, Monster->AttackTrackingRotSpeed);
	}
	Monster->SetActorRotation(NewRot);

	const float TrackingRangeSq = FMath::Square(Monster->AttackRange * Monster->AttackTrackingRangeMultiplier);
	if (FVector::DistSquared2D(MonsterLoc, TargetLoc) > TrackingRangeSq) return;

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

	FSimpleDelegate& Delegate = GetAttackDelegate(Monster);
	Delegate.Unbind();

	Monster->bIsAttacking = false;

	if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
		Anim->StopAllMontages(0.25f);
}
#pragma endregion