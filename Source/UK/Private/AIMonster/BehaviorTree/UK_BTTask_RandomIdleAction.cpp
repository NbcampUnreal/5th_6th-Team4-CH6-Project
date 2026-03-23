#include "AIMonster/BehaviorTree/UK_BTTask_RandomIdleAction.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	constexpr float DefaultRestoreWalkSpeed = 200.f;
}

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
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();

	AIController->StopMovement();
	MoveComp->StopMovementImmediately();
	MoveComp->bOrientRotationToMovement     = false;
	MoveComp->bUseControllerDesiredRotation = false;

	Memory->CachedWalkSpeed = MoveComp->MaxWalkSpeed > 0.f
		? MoveComp->MaxWalkSpeed
		: DefaultRestoreWalkSpeed;

	EIdleActionType Selected = SelectWeightedAction();
	if (Selected == EIdleActionType::PlayIdleMontage && (!Monster || Monster->IdleMontages.Num() == 0))
		Selected = EIdleActionType::Wait;

	Memory->Action = Selected;

	switch (Memory->Action)
	{
	case EIdleActionType::Wait:
		Memory->TargetTime = FMath::RandRange(MinWaitTime, MaxWaitTime);
		break;

	case EIdleActionType::LookAround:
	{
		Memory->TargetTime  = FMath::RandRange(MinWaitTime, MaxWaitTime);
		Memory->StartYaw    = Character->GetActorRotation().Yaw;
		const float Delta   = FMath::RandRange(60.f, 130.f) * (FMath::RandBool() ? 1.f : -1.f);
		Memory->TargetYaw   = Memory->StartYaw + Delta;
		Memory->MidYaw      = Memory->StartYaw + Delta * 0.35f;
		Memory->bReachedMid = false;
		break;
	}

	case EIdleActionType::PlayIdleMontage:
	{
		MoveComp->MaxWalkSpeed     = 0.f;
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

	if (Memory->Action == EIdleActionType::LookAround)
	{
		FRotator Rot = Character->GetActorRotation();
		const float DestYaw = Memory->bReachedMid ? Memory->TargetYaw : Memory->MidYaw;
		const float Speed   = Memory->bReachedMid
							  ? LookAroundSpeed * 0.13f
							  : LookAroundSpeed * 0.06f;

		Rot.Yaw = FMath::FInterpTo(Rot.Yaw, DestYaw, DeltaSeconds, Speed);

		if (!Memory->bReachedMid && FMath::Abs(FMath::FindDeltaAngleDegrees(Rot.Yaw, Memory->MidYaw)) < 2.f)
		{
			Memory->bReachedMid  = true;
			Memory->TargetTime  += FMath::RandRange(0.4f, 0.9f);
		}
		Character->SetActorRotation(Rot);
	}

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
			Character->GetCharacterMovement()->MaxWalkSpeed = Memory->CachedWalkSpeed;
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		if (Memory->ElapsedTime >= Memory->TargetTime)
		{
			if (Monster) Monster->OnIdleMontageFinished.Unbind();
			Character->GetCharacterMovement()->MaxWalkSpeed = Memory->CachedWalkSpeed;
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

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
			Anim->StopAllMontages(0.15f);
	}

	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		if (MoveComp->MaxWalkSpeed <= 0.f)
		{
			MoveComp->MaxWalkSpeed = (Memory && Memory->CachedWalkSpeed > 0.f)
				? Memory->CachedWalkSpeed
				: DefaultRestoreWalkSpeed;
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