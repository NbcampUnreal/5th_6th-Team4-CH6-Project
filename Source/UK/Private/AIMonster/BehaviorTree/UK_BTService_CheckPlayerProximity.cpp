#include "AIMonster/BehaviorTree/UK_BTService_CheckPlayerProximity.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"
#include "Kismet/GameplayStatics.h"

UUK_BTService_CheckPlayerProximity::UUK_BTService_CheckPlayerProximity()
{
	NodeName = "Check Player Proximity";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
	
	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, TargetPlayerKey), AActor::StaticClass());
	IsPlayerCloseKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, IsPlayerCloseKey));
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_CheckPlayerProximity, SpawnLocationKey));
}

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

	// 평화로운 몬스터가 아니면 실행 안 함
	if (Monster->Personality != EMonsterPersonality::Peaceful) return;

	// 공격적인 상태면 이 서비스 실행 안 함
	if (Monster->bIsAggressive) return;

	//  모든 플레이어 순회 (기존: GetPlayerCharacter(0)만)
	//   서버에서 실행되므로 모든 PlayerController를 순회해서 가장 가까운 플레이어 찾기
	UWorld* World = GetWorld();
	if (!World)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
		return;
	}

	AActor* ClosestPlayer = nullptr;
	float ClosestDistance = MAX_FLT;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPawn()) continue;

		APawn* PlayerPawn = PC->GetPawn();
		float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestPlayer = PlayerPawn;
		}
	}

	if (!ClosestPlayer)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
		return;
	}

	// 경계 거리 체크
	if (ClosestDistance <= Monster->AlertDistance)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, true);
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, ClosestPlayer);
	}
	else
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
	}
}