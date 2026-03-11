#include "AIMonster/BehaviorTree/UK_BTService_CheckChaseDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

#pragma region Initialization
UUK_BTService_CheckChaseDistance::UUK_BTService_CheckChaseDistance()
{
	NodeName = "Check Chase Distance";
	Interval = 0.3f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckChaseDistance, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckChaseDistance, SpawnLocationKey));
}
#pragma endregion

#pragma region Chase Distance Check
void UUK_BTService_CheckChaseDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	AActor* TargetPlayer = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName));
	if (!TargetPlayer) return;

	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	const FVector SpawnLocation   = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);

	const float DistanceToPlayer  = FVector::Dist(MonsterLocation, TargetPlayer->GetActorLocation());
	const float DistanceFromSpawn = FVector::Dist(MonsterLocation, SpawnLocation);
	const float ChaseLimit        = Monster->MaxChaseDistance > 0 ? Monster->MaxChaseDistance : MaxChaseDistance;

	const bool bShouldStopChase = (DistanceFromSpawn > ChaseLimit) || (DistanceToPlayer > ChaseLimit);
	if (!bShouldStopChase) return;

	BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);

	AIController->ClearFocus(EAIFocusPriority::Gameplay);

	if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
	{
		Monster->ResetToPassive();
	}
}
#pragma endregion