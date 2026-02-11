#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer_Peaceful.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"
#include "Kismet/GameplayStatics.h"

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

	UE_LOG(LogTemp, Log, TEXT("[DetectPlayer Service] %s | bIsAggressive=%d | Aggressor=%s"), 
		*Monster->GetName(), 
		Monster->bIsAggressive,
		Monster->Aggressor ? *Monster->Aggressor->GetName() : TEXT("null"));

	// ✅ 수정: Aggressive Mode Sequence 안에서 실행되므로 bIsAggressive 체크 불필요!
	// Decorator가 이미 체크했음
	
	// 공격자가 설정되어 있으면 그 공격자를 타겟으로 (거리 무관)
	if (IsValid(Monster->Aggressor))
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Monster->Aggressor);
		UE_LOG(LogTemp, Log, TEXT("[DetectPlayer Service] Set target to Aggressor: %s"), *Monster->Aggressor->GetName());
		return;
	}

	// 공격자가 없으면 가장 가까운 플레이어 찾기
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter)
	{
		float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());
		
		if (Distance <= DetectionRadius)
		{
			BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, PlayerCharacter);
			UE_LOG(LogTemp, Log, TEXT("[DetectPlayer Service] Set target to Player: %s (dist=%.1f)"), *PlayerCharacter->GetName(), Distance);
		}
		else
		{
			BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
			UE_LOG(LogTemp, Warning, TEXT("[DetectPlayer Service] Player too far (dist=%.1f > %.1f), clearing target"), Distance, DetectionRadius);
		}
	}
	else
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		UE_LOG(LogTemp, Warning, TEXT("[DetectPlayer Service] No player found, clearing target"));
	}
}