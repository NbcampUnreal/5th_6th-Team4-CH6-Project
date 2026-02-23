#include "AIMonster/BehaviorTree/UK_BTTask_RandomIdleAction.h"
#include "AIController.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UUK_BTTask_RandomIdleAction::UUK_BTTask_RandomIdleAction()
{
	NodeName = "Random Idle Action";
	bNotifyTick = true;
}

uint16 UUK_BTTask_RandomIdleAction::GetInstanceMemorySize() const
{
	return sizeof(FIdleActionMemory);
}

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

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// 행동 선택
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
		// 자연스러운 각도: 45~110도, 방향 랜덤
		const float Delta  = FMath::RandRange(45.f, 110.f) * (FMath::RandBool() ? 1.f : -1.f);
		Memory->TargetYaw  = Memory->StartYaw + Delta;
		break;
	}

	case EIdleActionType::PlayIdleMontage:
	{
		// 몽타주 재생 중 이동 완전 차단 ─────────────────────────────────
		Character->GetCharacterMovement()->MaxWalkSpeed = 0.f;

		Memory->TargetTime        = 15.f; // 안전 타임아웃
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

void UUK_BTTask_RandomIdleAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FIdleActionMemory* Memory = reinterpret_cast<FIdleActionMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(Character);

	// ── LookAround: EaseOut 으로 부드럽게 고개 돌리기 ───────────────────
	if (Memory->Action == EIdleActionType::LookAround)
	{
		FRotator Rot = Character->GetActorRotation();

		// FInterpTo: 목표에 가까워질수록 자동으로 느려짐 → 자연스러운 EaseOut
		Rot.Yaw = FMath::FInterpTo(Rot.Yaw, Memory->TargetYaw, DeltaSeconds, LookAroundSpeed * 0.1f);
		Character->SetActorRotation(Rot);
	}

	// ── 몽타주 종료 처리 ─────────────────────────────────────────────────
	if (Memory->bWaitingForMontage)
	{
		if (Memory->bMontageEnded && !Memory->bPostMontageWait)
		{
			Memory->bPostMontageWait    = true;
			Memory->PostMontageEndTime  = Memory->ElapsedTime + FMath::RandRange(0.5f, 1.2f);

			if (Monster) Monster->OnIdleMontageFinished.Unbind();
			return;
		}

		if (Memory->bPostMontageWait && Memory->ElapsedTime >= Memory->PostMontageEndTime)
		{
			// 이동 속도 복원 후 종료
			Character->GetCharacterMovement()->MaxWalkSpeed = 200.f; // SetWalkSpeed 서비스가 이어서 복원
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		// 타임아웃
		if (Memory->ElapsedTime >= Memory->TargetTime)
		{
			if (Monster) Monster->OnIdleMontageFinished.Unbind();
			Character->GetCharacterMovement()->MaxWalkSpeed = 200.f;
			RestoreRotationSettings(Character);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	// ── Wait / LookAround 시간 종료 ──────────────────────────────────────
	if (Memory->ElapsedTime >= Memory->TargetTime)
	{
		RestoreRotationSettings(Character);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UUK_BTTask_RandomIdleAction::RestoreRotationSettings(ACharacter* Character) const
{
	if (!Character) return;
	// 이동 시 다시 이동 방향으로 회전하도록 복원
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

EIdleActionType UUK_BTTask_RandomIdleAction::SelectWeightedAction() const
{
	if (ActionWeights.Num() == 0) return EIdleActionType::Wait;

	float Total = 0.f;
	for (float W : ActionWeights) Total += W;

	float Rand = FMath::FRandRange(0.f, Total);
	float Cumulative = 0.f;

	for (int32 i = 0; i < ActionWeights.Num(); ++i)
	{
		Cumulative += ActionWeights[i];
		if (Rand <= Cumulative)
			return static_cast<EIdleActionType>(i);
	}
	return EIdleActionType::Wait;
}