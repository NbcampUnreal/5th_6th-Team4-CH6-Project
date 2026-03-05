#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_AiMonsterCtl.h"

#pragma region Initialization
UUK_BTService_DetectPlayer::UUK_BTService_DetectPlayer()
{
	NodeName        = "Detect Player";
	Interval        = 0.1f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, SpawnLocationKey));
	PendingTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer, PendingTargetKey), AActor::StaticClass());
}

uint16 UUK_BTService_DetectPlayer::GetInstanceMemorySize() const
{
	return sizeof(FDetectPlayerMemory);
}
#pragma endregion

#pragma region Player Detection
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

	const bool bHasTarget  = (BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName)  != nullptr);
	const bool bHasPending = (BlackboardComp->GetValueAsObject(PendingTargetKey.SelectedKeyName) != nullptr);

	// 복귀 플래그 관리 
	// TargetPlayer/Pending 둘 다 사라진 직후 → 복귀 상태 진입
	if (Memory->bHadTarget && !bHasTarget && !bHasPending)
	{
		Memory->bReturning = true;
		Memory->bHadTarget = false;
	}

	// 복귀 중: 스폰 지점 근처 도착 전까지 감지 억제
	if (Memory->bReturning)
	{
		if (DistFromSpawn <= ReturnDistanceThreshold)
		{
			// 복귀 완료
			Memory->bReturning = false;
		
			BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
			BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
			Memory->bHadTarget = false;
		
			UE_LOG(LogTemp, Warning, TEXT("[DetectPlayer] %s: ✓ Return completed, ready for patrol"), 
				*GetName());
		
			return;
		}
		else
		{
			return;  
		}
	}

	// 추격 한계 초과 → 강제 이탈 처리 (AlertStandby도 Abort됨)
	if (DistFromSpawn > ChaseLimit)
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
		Memory->bHadTarget = false;
		Memory->bReturning = true;
		return;
	}

	// 패스트패스: AIPerceptionComponent 결과 재사용
	if (AUK_AiMonsterCtl* MonsterCtl = Cast<AUK_AiMonsterCtl>(AIController))
	{
		AActor* CtlTarget = MonsterCtl->GetCurrentTarget();
		if (CtlTarget)
		{
			if (BlackboardComp->GetValueAsObject(TargetPlayerKey.SelectedKeyName)) 
			{
				return; 
			}

			if (!BlackboardComp->GetValueAsObject(PendingTargetKey.SelectedKeyName))
			{
				BlackboardComp->SetValueAsObject(PendingTargetKey.SelectedKeyName, CtlTarget);
			}
		}
		return;
	}

	// 폴백: 직접 PlayerController 순회 
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

		const float Dist = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist   = Dist;
			BestTarget = PlayerPawn;
		}
	}

	if (BestTarget)
	{
		if (!Memory->bHadTarget)
			BlackboardComp->SetValueAsObject(PendingTargetKey.SelectedKeyName, BestTarget);

		Memory->bHadTarget = true;
	}
	else
	{
		BlackboardComp->ClearValue(PendingTargetKey.SelectedKeyName);
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		Memory->bHadTarget = false;
	}
}
#pragma endregion