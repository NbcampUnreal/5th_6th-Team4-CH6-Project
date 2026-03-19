#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_AiMonsterCtl.h"

#pragma region Initialization
UUK_BTService_DetectPlayer::UUK_BTService_DetectPlayer()
{
	NodeName        = "Detect Player";
	Interval        = 0.1f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, SpawnLocationKey));
	PendingTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, PendingTargetKey), AActor::StaticClass());
}

uint16 UUK_BTService_DetectPlayer::GetInstanceMemorySize() const
{
	return sizeof(FDetectPlayerMemory);
}
#pragma endregion

#pragma region Player Detection
void UUK_BTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (bPeacefulMode)
	{
		TickPeacefulMode(OwnerComp);
	}
	else
	{
		TickNormalMode(OwnerComp, NodeMemory);
	}
}
#pragma endregion

#pragma region Normal Mode 
void UUK_BTService_DetectPlayer::TickNormalMode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FDetectPlayerMemory* Memory = reinterpret_cast<FDetectPlayerMemory*>(NodeMemory);

    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	const float UseRadius = Monster ? Monster->DetectionRadius : DetectionRadius;

    const FVector MonsterLocation = ControlledPawn->GetActorLocation();
    const FVector SpawnLocation   = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float DistFromSpawnSq = FVector::DistSquared(MonsterLocation, SpawnLocation);
    const float   ChaseLimit      = Monster ? Monster->MaxChaseDistance : 2500.f;

    const bool bHasTarget  = (BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName)  != nullptr);
    const bool bHasPending = (BlackboardComp->GetValueAsObject(PendingTargetKey.SelectedKeyName) != nullptr);

    // 복귀 플래그 관리
    if (Memory->bHadTarget && !bHasTarget && !bHasPending)
    {
        Memory->bReturning = true;
        Memory->bHadTarget = false;
    }

    if (Memory->bReturning)
    {
        // 복귀 중엔 타겟 즉시 클리어 + 재감지 완전 차단
        BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
        BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);

        if (DistFromSpawnSq <= FMath::Square(ReturnDistanceThreshold))
        {
            Memory->bReturning = false;
        }
        return;
    }

    if (DistFromSpawnSq > FMath::Square(ChaseLimit))
    {
        Memory->bReturning = true;
        Memory->bHadTarget = false;
        BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
        BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
        return;
    }

    AActor* DetectedPlayer = TryGetPerceptionTarget(AIController);
    if (!DetectedPlayer)
        DetectedPlayer = FindClosestPlayer(MonsterLocation, DetectionRadius, ControlledPawn->GetWorld());

    // Perception이 근거리에서 실패하는 경우 보정: AttackRange * 2 이내는 항상 감지
    if (!DetectedPlayer && Monster)
    {
        const float GuaranteedRange = Monster->AttackRange * 2.f;
        DetectedPlayer = FindClosestPlayer(MonsterLocation, GuaranteedRange, ControlledPawn->GetWorld());
    }

    if (DetectedPlayer)
    {
        if (BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName))
        {
            return;
        }

        if (!BlackboardComp->GetValueAsObject(PendingTargetKey.SelectedKeyName))
        {
            BlackboardComp->SetValueAsObject(PendingTargetKey.SelectedKeyName, DetectedPlayer);
            AIController->SetFocus(DetectedPlayer);
        }

        Memory->bHadTarget = true;
    }
    else
    {
        if (BlackboardComp->GetValueAsObject(PendingTargetKey.SelectedKeyName))
        {
            return;
        }

        BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
        BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
        Memory->bHadTarget = false;
        AIController->ClearFocus(EAIFocusPriority::Gameplay);
    }
}
#pragma endregion

#pragma region Peaceful Mode 
void UUK_BTService_DetectPlayer::TickPeacefulMode(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (!Monster) return;

	// ── Aggressor 최우선 ─────────────────────────────────────────────────
	if (IsValid(Monster->Aggressor))
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Monster->Aggressor);
		return;
	}

	// ── 플레이어 감지 ────────────────────────────────────────────────────
	AActor* DetectedPlayer = TryGetPerceptionTarget(AIController);
	if (!DetectedPlayer)
	{
		DetectedPlayer = FindClosestPlayer(ControlledPawn->GetActorLocation(), DetectionRadius, ControlledPawn->GetWorld());
	}

	if (DetectedPlayer)
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, DetectedPlayer);
	}
	else
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
	}
}
#pragma endregion

#pragma region Helpers
AActor* UUK_BTService_DetectPlayer::TryGetPerceptionTarget(AAIController* AICon)
{
	if (!AICon) return nullptr;

	AUK_AiMonsterCtl* MonsterCtl = Cast<AUK_AiMonsterCtl>(AICon);
	if (!MonsterCtl) return nullptr;

	return MonsterCtl->GetCurrentTarget();
}

AActor* UUK_BTService_DetectPlayer::FindClosestPlayer(const FVector& Location, float MaxRadius, UWorld* World)
{
	if (!World) return nullptr;

	AActor* BestTarget = nullptr;
	float   BestDist   = MaxRadius;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) continue;

		const float Dist = FVector::Dist(Location, PlayerPawn->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist   = Dist;
			BestTarget = PlayerPawn;
		}
	}

	return BestTarget;
}
#pragma endregion