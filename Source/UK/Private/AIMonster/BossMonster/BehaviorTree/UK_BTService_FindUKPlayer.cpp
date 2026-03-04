#include "AIMonster/BossMonster/BehaviorTree/UK_BTService_FindUKPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Character/UK_CharacterBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UUK_BTService_FindUKPlayer::UUK_BTService_FindUKPlayer()
{
	NodeName = TEXT("Find UK Player");
	bNotifyTick = true;
	Interval = 0.3f;
}

void UUK_BTService_FindUKPlayer::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	auto* AI = OwnerComp.GetAIOwner();
	if (!AI) return;

	APawn* SelfPawn = AI->GetPawn();
	if (!SelfPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AUK_CharacterBase* ClosestPlayer = nullptr;
	float MinDist = DetectRadius;
	
	for (TActorIterator<AUK_CharacterBase> It(World); It; ++It)
	{
		AUK_CharacterBase* Player = *It;

		if (!IsValid(Player)) continue;

		float Dist = FVector::Dist(
			Player->GetActorLocation(),
			SelfPawn->GetActorLocation()
		);

		if (Dist <= MinDist)
		{
			MinDist = Dist;
			ClosestPlayer = Player;
		}
	}

	if (ClosestPlayer)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), ClosestPlayer);
	}
	else
	{
		BB->ClearValue(TEXT("TargetActor"));
	}
}