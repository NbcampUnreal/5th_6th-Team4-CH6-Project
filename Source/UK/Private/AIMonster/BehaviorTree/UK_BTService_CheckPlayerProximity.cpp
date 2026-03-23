#include "AIMonster/BehaviorTree/UK_BTService_CheckPlayerProximity.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTService_CheckPlayerProximity::UUK_BTService_CheckPlayerProximity()
{
	NodeName = "Check Player Proximity";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, TargetPlayerKey), AActor::StaticClass());
	IsPlayerCloseKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, IsPlayerCloseKey));
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, SpawnLocationKey));
}
#pragma endregion

#pragma region Proximity Check
void UUK_BTService_CheckPlayerProximity::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	if (Monster->Personality != EMonsterPersonality::Peaceful) return;
	if (Monster->bIsAggressive) return;

	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	const float AlertDistSq = FMath::Square(Monster->AlertDistance);

	// ── 패스트패스: 기존 타겟과 거리만 재확인 ─────────────────────────────
	if (AActor* ExistingTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName)))
	{
		if (FVector::DistSquared(MonsterLocation, ExistingTarget->GetActorLocation()) <= AlertDistSq)
		{
			BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, true);
			return;
		}
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
	}

	// ── 풀 서치: 타겟 없을 때만 전체 순회 ──────────────────────────────────
	UWorld* World = GetWorld();
	if (!World)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
		return;
	}

	AActor* ClosestPlayer    = nullptr;
	float   ClosestDistSq    = MAX_FLT;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPawn()) continue;
 
		const float DistSq = FVector::DistSquared(MonsterLocation, PC->GetPawn()->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestPlayer = PC->GetPawn();
		}
	}
 
	if (!ClosestPlayer)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
		return;
	}

	if (ClosestDistSq <= AlertDistSq)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, true);
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, ClosestPlayer);
	}
	else
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
	}
}
#pragma endregion