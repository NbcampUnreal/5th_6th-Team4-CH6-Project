#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer_Peaceful.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTService_DetectPlayer_Peaceful::UUK_BTService_DetectPlayer_Peaceful()
{
	NodeName = "Detect Player (Peaceful)";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer_Peaceful, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer_Peaceful, SpawnLocationKey));
}

void UUK_BTService_DetectPlayer_Peaceful::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	AAIMonsterBase* Monster = Cast<AAIMonsterBase>(ControlledPawn);
	if (!Monster) return;

	// 공격자(Aggressor)가 있으면 최우선 타겟 (거리 무관)
	if (IsValid(Monster->Aggressor))
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Monster->Aggressor);
		return;
	}

	// 공격자가 없으면 가장 가까운 플레이어 찾기
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