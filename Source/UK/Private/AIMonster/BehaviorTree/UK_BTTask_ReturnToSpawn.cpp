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

	const FVector SpawnLocation  = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float   DistFromSpawn  = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	// TargetPlayer 클리어 및 Focus 해제
	BB->ClearValue(TargetPlayerKey.SelectedKeyName);
	AICon->ClearFocus(EAIFocusPriority::Gameplay);

	// 이미 스폰 근처면 즉시 종료
	if (DistFromSpawn < ArrivalDistance)
	{
		if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
		{
			Monster->ResetToPassive();
		}
		return EBTNodeResult::Succeeded;
	}

	// ── 회전 설정 확인 ───────────────────────────────────────────────────
	if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
	{
		// 복귀 시 이동 설정 강제 복원
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->SetMovementMode(MOVE_Walking);  
		
		// 속도 확인 및 설정
		if (MoveComp->MaxWalkSpeed <= 0.f)
		{
			MoveComp->MaxWalkSpeed = 600.f;
		}
		
		OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		MoveComp->MaxWalkSpeed = OriginalMaxWalkSpeed * ReturnSpeedMultiplier;
		bSpeedBoosted = true;
	}

	StuckRetryTimer = 0.f;
	
	// SimpleMoveToLocation - PathFollowing 없이 직접 이동
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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) { RestoreSpeed(ControlledPawn); FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	const FVector SpawnLocation = BB->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float DistFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

	if (DistFromSpawn <= ArrivalDistance)
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

	// 멈춰있을 때만 재요청 (매 틱 호출 제거) ← 핵심 수정
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
	UE_LOG(LogTemp, Warning, TEXT("[ReturnToSpawn] ABORTED - Interrupted by higher priority"));
	
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (AICon)
	{
		APawn* ControlledPawn = AICon->GetPawn();
		
		// 이동 완전 정지
		AICon->StopMovement();
		
		// 속도 복원
		RestoreSpeed(ControlledPawn);
		
		// 경로 취소
		if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
		{
			PFC->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, FAIRequestID::CurrentRequest);
		}
		
		// 회전 설정 복원
		if (AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn))
		{
			if (UCharacterMovementComponent* MC = Monster->GetCharacterMovement())
			{
				MC->bOrientRotationToMovement = true;
				MC->bUseControllerDesiredRotation = false;
				MC->SetMovementMode(MOVE_Walking); 
				
				UE_LOG(LogTemp, Warning, TEXT("[ReturnToSpawn] AbortTask - Movement restored: Orient=%d, Mode=%d"), 
					MC->bOrientRotationToMovement, (int32)MC->MovementMode);
			}
		}
	}
	
	StuckRetryTimer = 0.f;
	return EBTNodeResult::Aborted;
}
#pragma endregion

#pragma region Helpers
void UUK_BTTask_ReturnToSpawn::RequestMoveTo(AAIController* AICon, const FVector& Dest)
{
	if (!AICon)
	{
		return;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn)
	{
		return;
	}

	// 이동 전 PathFollowing 상태 체크
	UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent();
	if (!PFC)
	{
		return;
	}

	// 기존 경로 완전히 중단
	PFC->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished);
	
	// MovementComponent 강제 활성화
	if (UCharacterMovementComponent* MC = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
	{
		MC->SetComponentTickEnabled(true);
		MC->Activate(true);
	}

	// AI 이동 요청
	FAIMoveRequest MoveRequest(Dest);
	MoveRequest.SetAcceptanceRadius(ArrivalDistance * 0.5f);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetReachTestIncludesAgentRadius(true);
	
	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult MoveResult = AICon->MoveTo(MoveRequest, &NavPath);
	
	// PathFollowing 상태 확인
	if (PFC)
	{
		EPathFollowingStatus::Type Status = PFC->GetStatus();
		
		// Moving 상태가 아니면 강제로 시작
		if (Status != EPathFollowingStatus::Moving && NavPath.IsValid())
		{
			PFC->RequestMove(MoveRequest, NavPath);
		}
	}
	
	// 실패 시 직접 이동
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Error, TEXT("[ReturnToSpawn] MoveTo FAILED - Trying direct MoveToLocation"));
	}
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