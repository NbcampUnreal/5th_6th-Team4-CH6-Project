#include "AIMonster/BehaviorTree/UK_BTTask_AlertStandby.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTTask_AlertStandby::UUK_BTTask_AlertStandby()
{
	NodeName    = "Alert Standby";
	bNotifyTick = true;

	PendingTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertStandby, PendingTargetKey), AActor::StaticClass());
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_AlertStandby, TargetPlayerKey), AActor::StaticClass());
}

uint16 UUK_BTTask_AlertStandby::GetInstanceMemorySize() const
{
	return sizeof(FAlertStandbyMemory);
}
#pragma endregion

#pragma region Helpers
static void SetOrientToMovement(APawn* Pawn, bool bEnable)
{
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			Move->bOrientRotationToMovement = bEnable;
	}
}
#pragma endregion

#pragma region Task Interface
EBTNodeResult::Type UUK_BTTask_AlertStandby::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FAlertStandbyMemory* Mem = reinterpret_cast<FAlertStandbyMemory*>(NodeMemory);
	Mem->ElapsedTime = 0.f;

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	AICon->StopMovement();
	SetOrientToMovement(ControlledPawn, true);

	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		Monster->ShowAlertIcon();

	return EBTNodeResult::InProgress;
}

void UUK_BTTask_AlertStandby::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FAlertStandbyMemory* Mem = reinterpret_cast<FAlertStandbyMemory*>(NodeMemory);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(PendingTargetKey.SelectedKeyName));
	if (Target)
	{
		const FVector  ToTarget   = (Target->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
		const FRotator TargetRot  = FRotationMatrix::MakeFromX(ToTarget).Rotator();
		const FRotator CurrentRot = ControlledPawn->GetActorRotation();
		const FRotator NewRot     = FMath::RInterpTo(CurrentRot, FRotator(0.f, TargetRot.Yaw, 0.f), DeltaSeconds, LookAtSpeed);
		ControlledPawn->SetActorRotation(NewRot);
	}

	Mem->ElapsedTime += DeltaSeconds;
	if (Mem->ElapsedTime >= AlertDuration)
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
			Monster->HideAlertIcon();

		if (ACharacter* Char = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement     = true;
				MC->bUseControllerDesiredRotation = false;
			}
		}

		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		if (Target)
			BB->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Target);

		BB->ClearValue(PendingTargetKey.SelectedKeyName);

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UUK_BTTask_AlertStandby::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		if (APawn* ControlledPawn = AICon->GetPawn())
		{
			if (ACharacter* Char = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* MC = Char->GetCharacterMovement())
				{
					MC->bOrientRotationToMovement     = true;
					MC->bUseControllerDesiredRotation = false;
				}
			}

			AICon->ClearFocus(EAIFocusPriority::Gameplay);

			if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
				Monster->HideAlertIcon();
		}

		AICon->StopMovement();
	}

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		BB->ClearValue(PendingTargetKey.SelectedKeyName);

	return EBTNodeResult::Aborted;
}
#pragma endregion