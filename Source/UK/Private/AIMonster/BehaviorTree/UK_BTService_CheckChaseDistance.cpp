#include "AIMonster/BehaviorTree/UK_BTService_CheckChaseDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIMonster/AIMonsterBase.h"

UUK_BTService_CheckChaseDistance::UUK_BTService_CheckChaseDistance()
{
	NodeName = "Check Chase Distance";
	Interval = 0.3f;  
	RandomDeviation = 0.1f;
	
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckChaseDistance, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckChaseDistance, SpawnLocationKey));
}

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

	FVector MonsterLocation = ControlledPawn->GetActorLocation();
	FVector PlayerLocation = TargetPlayer->GetActorLocation();
	FVector SpawnLocation = BlackboardComp->GetValueAsVector(SpawnLocationKey.SelectedKeyName);

	float DistanceToPlayer = FVector::Dist(MonsterLocation, PlayerLocation);
	float DistanceFromSpawn = FVector::Dist(MonsterLocation, SpawnLocation);

	float ChaseLimit = Monster->MaxChaseDistance > 0 ? Monster->MaxChaseDistance : MaxChaseDistance;

	// 추격 중단 조건
	bool bShouldStopChase = false;
	FString StopReason;

	// 스폰에서 너무 멀어짐
	if (DistanceFromSpawn > ChaseLimit)
	{
		bShouldStopChase = true;
		StopReason = FString::Printf(TEXT("Too far from spawn: %.1f > %.1f"), DistanceFromSpawn, ChaseLimit);
	}
	// 플레이어가 너무 멀어짐
	else if (DistanceToPlayer > ChaseLimit)
	{
		bShouldStopChase = true;
		StopReason = FString::Printf(TEXT("Player too far: %.1f > %.1f"), DistanceToPlayer, ChaseLimit);
	}
	// 공격 범위 안에 들어옴 (추격 중단, 공격 시작)
	else if (DistanceToPlayer <= Monster->AttackRange)
	{
		// 이 경우는 추격 중단이 아니라 공격으로 전환되어야 함
		// BT의 InAttackRange 데코레이터가 처리하므로 여기서는 체크만
		UE_LOG(LogTemp, Log, TEXT("[ChaseDistance] %s: In attack range (%.1f <= %.1f)"),
			*Monster->GetName(), DistanceToPlayer, Monster->AttackRange);
		return;
	}

	if (bShouldStopChase)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ChaseDistance] %s: Stopping chase - %s"),
			*Monster->GetName(), *StopReason);

		// 타겟 클리어 -> BT가 ReturnToSpawn으로 전환
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);

		// 이동 즉시 정지
		AIController->StopMovement();

		// 평화 몬스터라면 리셋
		if (Monster->Personality == EMonsterPersonality::Peaceful && Monster->GetIsAggressive())
		{
			Monster->ResetToPassive();
		}
	}
}