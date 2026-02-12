#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTService_DetectPlayer::UUK_BTService_DetectPlayer()
{
	NodeName = "Detect Player";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, SpawnLocationKey));
}

void UUK_BTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	// 몬스터 설정값 가져오기
	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (Monster)
	{
		DetectionRadius = Monster->DetectionRadius;

		// 스폰 위치 기준 최대 추적 거리 체크
		FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);
		float DistanceFromSpawn = FVector::Dist(ControlledPawn->GetActorLocation(), SpawnLocation);

		if (DistanceFromSpawn > Monster->MaxChaseDistance)
		{
			BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
			return;
		}
	}

	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	AActor* BestTarget = nullptr;
	float BestDist = DetectionRadius;

	UWorld* World = ControlledPawn->GetWorld();
	if (!World)
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) continue;

		float Dist = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestTarget = PlayerPawn;
		}
	}

	if (BestTarget)
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, BestTarget);
	}
	else
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
	}
}