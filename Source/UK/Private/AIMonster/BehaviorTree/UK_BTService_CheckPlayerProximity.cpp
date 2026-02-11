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

	// 플레이어 찾기
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
		return;
	}

	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());

	// 경계 거리 체크
	if (Distance <= Monster->AlertDistance)
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, true);
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, PlayerCharacter);
		
		UE_LOG(LogTemp, Log, TEXT("[Proximity] %s detected player nearby (%.1f)"), 
			*Monster->GetName(), Distance);
	}
	else
	{
		BlackboardComp->SetValueAsBool(IsPlayerCloseKey.SelectedKeyName, false);
	}
}