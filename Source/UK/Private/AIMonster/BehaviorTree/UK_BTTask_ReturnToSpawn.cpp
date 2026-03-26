#include "AIMonster/BehaviorTree/UK_BTTask_ReturnToSpawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

#pragma region Initialization
UUK_BTTask_ReturnToSpawn::UUK_BTTask_ReturnToSpawn()
{
	NodeName            = "Return To Spawn";
	bNotifyTick         = true;
	bCreateNodeInstance = true;

	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, SpawnLocationKey));
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, TargetPlayerKey), AActor::StaticClass());
}
#pragma endregion

#pragma region Execution
EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (!Monster) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	const FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);

	BB->ClearValue(TargetPlayerKey.SelectedKeyName);
	AICon->ClearFocus(EAIFocusPriority::Gameplay);
	AICon->ClearFocus(EAIFocusPriority::Default);

	if (FVector::DistSquared(ControlledPawn->GetActorLocation(), SpawnLocation) < FMath::Square(ArrivalDistance))
	{
		if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
			Monster->ResetToPassive();
		return EBTNodeResult::Succeeded;
	}

	if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->bOrientRotationToMovement     = true;
		MoveComp->SetMovementMode(MOVE_Walking);

		if (MoveComp->MaxWalkSpeed <= 0.f)
			MoveComp->MaxWalkSpeed = 600.f;

		OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed * ReturnSpeedMultiplier;
		bSpeedBoosted = true;
	}

	StuckRetryTimer = 0.f;
	AICon->MoveToLocation(SpawnLocation, ArrivalDistance, false, true, false, false);

	return EBTNodeResult::InProgress;
}
#pragma endregion

#pragma region Return Tick
void UUK_BTTask_ReturnToSpawn::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) { RestoreSpeed(nullptr); FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) { RestoreSpeed(nullptr); FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
	{
		if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
		{
			MoveComp->bUseControllerDesiredRotation = false;
			MoveComp->bOrientRotationToMovement     = true;
		}
	}
	AICon->ClearFocus(EAIFocusPriority::Gameplay);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) { RestoreSpeed(ControlledPawn); FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	const FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);

	if (FVector::DistSquared(ControlledPawn->GetActorLocation(), SpawnLocation) <= FMath::Square(ArrivalDistance))
	{
		RestoreSpeed(ControlledPawn);
		AICon->StopMovement();

		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		{
			if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
				Monster->ResetToPassive();
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	StuckRetryTimer += DeltaSeconds;
	if (StuckRetryTimer >= 1.0f)
	{
		StuckRetryTimer = 0.f;
		if (ControlledPawn->GetVelocity().SizeSquared2D() < 10.f)
		{
			AICon->MoveToLocation(SpawnLocation, ArrivalDistance, false, true, false, false);
		}
	}
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		APawn* ControlledPawn = AICon->GetPawn();
		AICon->StopMovement();
		RestoreSpeed(ControlledPawn);

		if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
			PFC->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, FAIRequestID::CurrentRequest);

		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement     = true;
				MC->bUseControllerDesiredRotation = false;
				MC->SetMovementMode(MOVE_Walking);
			}
		}
	}

	StuckRetryTimer = 0.f;
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Helpers
void UUK_BTTask_ReturnToSpawn::RestoreSpeed(APawn* InPawn)
{
	if (!bSpeedBoosted) return;

	if (InPawn)
	{
		if (ACharacter* Char = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
				MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
		}
	}

	bSpeedBoosted        = false;
	OriginalMaxWalkSpeed = 0.f;
}
#pragma endregion