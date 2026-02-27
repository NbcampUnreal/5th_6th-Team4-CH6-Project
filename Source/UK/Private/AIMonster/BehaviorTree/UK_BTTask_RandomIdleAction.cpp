#include "AIMonster/BehaviorTree/UK_BTTask_RandomIdleAction.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
UUK_BTTask_RandomIdleAction::UUK_BTTask_RandomIdleAction()
{
	NodeName    = "Random Idle Action";
	bNotifyTick = true;
}

uint16 UUK_BTTask_RandomIdleAction::GetInstanceMemorySize() const
{
	return sizeof(FIdleActionMemory);
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_RandomIdleAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FIdleActionMemory* Memory = reinterpret_cast<FIdleActionMemory*>(NodeMemory);
	*Memory = FIdleActionMemory{};

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Character);

	AIController->StopMovement();
	Character->GetCharacterMovement()->StopMovementImmediately();
	Character->GetCharacterMovement()->bOrientRotationToMovement       = false;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation   = false;

	// ── 행동 선택 ────────────────────────────────────────────────────────
	EIdleActionType Selected = SelectWeightedAction();
	if (Selected == EIdleActionType::PlayIdleMontage && (!Monster || Monster->IdleMontages.Num() == 0))
	{
		Selected = EIdleActionType::Wait;
	}
	Memory->Action = Selected;

	switch (Memory->Action)
	{
	case EIdleActionType::Wait:
		Memory->TargetTime = FMath::RandRange(MinWaitTime, MaxWaitTime);
		break;

	case EIdleActionType::LookAround:
	{
		Memory->TargetTime = FMath::RandRange(MinWaitTime, MaxWaitTime);
		Memory->StartYaw   = Character->GetActorRotation().Yaw;
		const float Delta  = FMath::RandRange(45.f, 110.f) * (FMath::RandBool() ? 1.f : -1.f);
		Memory->TargetYaw  = Memory->StartYaw + Delta;
		break;
	}

	case EIdleActionType::PlayIdleMontage:
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 0.f;
		Memory->TargetTime         = 15.f;
		Memory->bWaitingForMontage = true;

		Monster->OnIdleMontageFinished.BindLambda([Memory]()
		{
			Memory->bMontageEnded = true;
		});

		Monster->PlayRandomIdleMontage();
		break;
	}
	}

	return EBTNodeResult::InProgress;
}
#pragma endregion

#pragma region Idle Tick
void UUK_BTTask_RandomIdleAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FIdleActionMemory* Memory = reinterpret_cast<FIdleActionMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Character);

	// ── LookAround: EaseOut 회전 ─────────────────────────────────────────
	if (Memory->Action == EIdleActionType::LookAround)
	{
		FRotator Rot = Character->GetActorRotation();
		Rot.Yaw = FMath::FInterpTo(Rot.Yaw, Memory->TargetYaw, DeltaSeconds, LookAroundSpeed * 0.1f);
		Character->SetActorRotation(Rot);
	}

	// ── 몽타주 종료 처리 ─────────────────────────────────────────────────
	if (Memory->bWaitingForMontage)
	{
		if (Memory->bMontageEnded && !Memory->bPostMontageWait)
		{
			Memory->bPostMontageWait   = true;
			Memory->PostMontageEndTime = Memory->ElapsedTime + FMath::RandRange(0.5f, 1.2f);
			if (Monster) Monster->OnIdleMontageFinished.Unbind();
			return;
		}

		if (Memory->bPostMontageWait && Memory->ElapsedTime >= Memory->PostMontageEndTime)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 200.f;
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		if (Memory->ElapsedTime >= Memory->TargetTime)
		{
			if (Monster) Monster->OnIdleMontageFinished.Unbind();
			Character->GetCharacterMovement()->MaxWalkSpeed = 200.f;
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	// ── Wait / LookAround 종료 ───────────────────────────────────────────
	if (Memory->ElapsedTime >= Memory->TargetTime)
	{
		RestoreRotationSettings(Character);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_RandomIdleAction::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FIdleActionMemory* Memory = reinterpret_cast<FIdleActionMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Aborted;

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) return EBTNodeResult::Aborted;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Character);

	if (Memory && Memory->bWaitingForMontage)
	{
		if (Monster) Monster->OnIdleMontageFinished.Unbind();
		if (UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance())
		{
			Anim->StopAllMontages(0.15f);
		}
	}

	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		if (MoveComp->MaxWalkSpeed <= 0.f)
		{
			MoveComp->MaxWalkSpeed = 200.f;
		}
	}

	RestoreRotationSettings(Character);
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Helpers
void UUK_BTTask_RandomIdleAction::RestoreRotationSettings(ACharacter* Character) const
{
	if (!Character) return;
	Character->GetCharacterMovement()->bOrientRotationToMovement     = true;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

EIdleActionType UUK_BTTask_RandomIdleAction::SelectWeightedAction() const
{
	if (ActionWeights.Num() == 0) return EIdleActionType::Wait;

	float Total = 0.f;
	for (float W : ActionWeights) Total += W;

	float Rand       = FMath::FRandRange(0.f, Total);
	float Cumulative = 0.f;

	for (int32 i = 0; i < ActionWeights.Num(); ++i)
	{
		Cumulative += ActionWeights[i];
		if (Rand <= Cumulative)
			return static_cast<EIdleActionType>(i);
	}
	return EIdleActionType::Wait;
}
#pragma endregion