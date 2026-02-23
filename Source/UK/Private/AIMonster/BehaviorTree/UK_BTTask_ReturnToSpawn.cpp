#include "AIMonster/BehaviorTree/UK_BTTask_ReturnToSpawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UUK_BTTask_ReturnToSpawn::UUK_BTTask_ReturnToSpawn()
{
	NodeName = "Return To Spawn";
	bNotifyTick = true;
	bCreateNodeInstance = true;

	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, SpawnLocationKey));
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTTask_ReturnToSpawn, TargetPlayerKey), AActor::StaticClass());
}

EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	float DistFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	// 타겟 클리어
	BB->ClearValue(TargetPlayerKey.SelectedKeyName);

	// 이미 스폰 근처면 즉시 성공
	if (DistFromSpawn < ArrivalDistance)
	{
		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
		if (Monster && Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
		{
			Monster->ResetToPassive();
		}
		
		return EBTNodeResult::Succeeded;
	}

	// 속도 부스트
	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (Monster)
	{
		if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
		{
			OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
			MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed * ReturnSpeedMultiplier;
			bSpeedBoosted = true;
		}
	}

	// 스폰으로 이동
	FAIMoveRequest MoveRequest(SpawnLocation);
	MoveRequest.SetAcceptanceRadius(ArrivalDistance * 0.5f);
	MoveRequest.SetAllowPartialPath(false);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetReachTestIncludesAgentRadius(true);
	
	FNavPathSharedPtr NavPath;
	AICon->MoveTo(MoveRequest, &NavPath);
	
	return EBTNodeResult::InProgress;
}

void UUK_BTTask_ReturnToSpawn::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		RestoreSpeed(nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControlledPawn = AICon->GetPawn();
	if (!ControlledPawn)
	{
		RestoreSpeed(nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		RestoreSpeed(ControlledPawn);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	float DistFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	// 도착 체크
	if (DistFromSpawn <= ArrivalDistance)
	{
		RestoreSpeed(ControlledPawn);
		AICon->StopMovement();

		AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
		if (Monster && Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
		{
			Monster->ResetToPassive();
		}

		UE_LOG(LogTemp, Warning, TEXT("[Return] %s: Arrived at spawn!"), *ControlledPawn->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 이동 중 멈췄는지 체크 (스턱 방지)
	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (Monster)
	{
		if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
		{
			// 속도가 너무 느리면 다시 이동 명령
			if (MoveComp->Velocity.Size() < 10.f && DistFromSpawn > ArrivalDistance)
			{
				FAIMoveRequest MoveRequest(SpawnLocation);
				MoveRequest.SetAcceptanceRadius(ArrivalDistance * 0.5f);
				MoveRequest.SetAllowPartialPath(false);
				MoveRequest.SetUsePathfinding(true);
				
				AICon->MoveTo(MoveRequest);
			}
		}
	}
}

EBTNodeResult::Type UUK_BTTask_ReturnToSpawn::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		RestoreSpeed(AICon->GetPawn());
		AICon->StopMovement();
	}

	return EBTNodeResult::Aborted;
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

	bSpeedBoosted = false;
	OriginalMaxWalkSpeed = 0.f;
}