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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	const FVector SpawnLocation  = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float   DistFromSpawn  = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	BB->ClearValue(TargetPlayerKey.SelectedKeyName);

	// 이미 스폰 근처면 즉시 종료
	if (DistFromSpawn < ArrivalDistance)
	{
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		{
			if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
			{
				Monster->ResetToPassive();
			}
		}
		return EBTNodeResult::Failed;
	}

	// ── 복귀 속도 부스트 ─────────────────────────────────────────────────
	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
	{
		if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
		{
			OriginalMaxWalkSpeed       = MoveComp->MaxWalkSpeed;
			MoveComp->MaxWalkSpeed     = OriginalMaxWalkSpeed * ReturnSpeedMultiplier;
			bSpeedBoosted              = true;
		}
	}

	StuckRetryTimer = 0.f;
	RequestMoveTo(AICon, SpawnLocation);
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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) { RestoreSpeed(ControlledPawn); FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	const FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float   DistFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	// ── 도착 체크 ────────────────────────────────────────────────────────
	if (DistFromSpawn <= ArrivalDistance)
	{
		RestoreSpeed(ControlledPawn);
		AICon->StopMovement();

		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		{
			if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
			{
				Monster->ResetToPassive();
			}
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// ── 스턱 감지: 쿨다운 후 재이동 ─────────────────────────────────────
	if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
	{
		if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
		{
			if (MoveComp->Velocity.SizeSquared() < 100.f)
			{
				StuckRetryTimer += DeltaSeconds;
				if (StuckRetryTimer >= StuckRetryInterval)
				{
					StuckRetryTimer = 0.f;
					RequestMoveTo(AICon, SpawnLocation);
				}
			}
			else
			{
				StuckRetryTimer = 0.f;
			}
		}
	}
}
#pragma endregion

#pragma region Abort
EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		RestoreSpeed(AICon->GetPawn());
		AICon->StopMovement();
	}
	StuckRetryTimer = 0.f;
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Helpers
void UUK_BTTask_ReturnToSpawn::RequestMoveTo(AAIController* AICon, const FVector& Dest)
{
	if (!AICon) return;

	FAIMoveRequest MoveRequest(Dest);
	MoveRequest.SetAcceptanceRadius(ArrivalDistance * 0.5f);
	MoveRequest.SetAllowPartialPath(false);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetReachTestIncludesAgentRadius(true);
	AICon->MoveTo(MoveRequest);
}

void UUK_BTTask_ReturnToSpawn::RestoreSpeed(APawn* InPawn)
{
	if (!bSpeedBoosted) return;

	if (InPawn)
	{
		if (ACharacter* Char = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
			}
		}
	}

	bSpeedBoosted        = false;
	OriginalMaxWalkSpeed = 0.f;
}
#pragma endregion