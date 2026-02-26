#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTService_DetectPlayer::UUK_BTService_DetectPlayer()
{
	NodeName = "Detect Player";
	Interval = 0.1f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, SpawnLocationKey));
}

uint16 UUK_BTService_DetectPlayer::GetInstanceMemorySize() const
{
	return sizeof(FDetectPlayerMemory);
}

void UUK_BTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	FDetectPlayerMemory* Memory = reinterpret_cast<FDetectPlayerMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (Monster) DetectionRadius = Monster->DetectionRadius;

	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	const FVector SpawnLocation   = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
	const float   DistFromSpawn   = FVector::Dist(MonsterLocation, SpawnLocation);
	const float   ChaseLimit      = Monster ? Monster->MaxChaseDistance : 2500.f;

	bool bCurrentlyHasTarget = (BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName) != nullptr);

	// 타겟을 방금 잃었으면 복귀 중 플래그 세팅
	if (Memory->bHadTarget && !bCurrentlyHasTarget)
	{
		Memory->bReturning = true;
		Memory->bHadTarget = false;
	}

	// ── 복귀 중이면 스폰 근처 도달 전까지 재감지 차단 ────────────────────────
	if (Memory->bReturning)
	{
		if (DistFromSpawn <= ReturnDistanceThreshold)
		{
			// 스폰 근처 복귀 완료 → 차단 해제
			Memory->bReturning = false;
		}
		else
		{
			// 아직 복귀 중 → 재감지 차단
			return;
		}
	}
	// ─────────────────────────────────────────────────────────────────────────

	// 스폰에서 너무 멀면 감지 차단 + 복귀 플래그
	if (DistFromSpawn > ChaseLimit)
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		Memory->bHadTarget = false;
		Memory->bReturning = true;
		return;
	}

	// 플레이어 탐색
	UWorld* World = ControlledPawn->GetWorld();
	if (!World) return;

	AActor* BestTarget = nullptr;
	float   BestDist   = DetectionRadius;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) continue;

		float Dist = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist   = Dist;
			BestTarget = PlayerPawn;
		}
	}

	if (BestTarget)
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, BestTarget);
		Memory->bHadTarget = true;
	}
	else
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		Memory->bHadTarget = false;
	}
}