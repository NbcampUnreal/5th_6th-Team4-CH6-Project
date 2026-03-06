#include "AIMonster/BehaviorTree/UK_BTService_DetectPlayer_Peaceful.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_AiMonsterCtl.h"

#pragma region Initialization
UUK_BTService_DetectPlayer_Peaceful::UUK_BTService_DetectPlayer_Peaceful()
{
	NodeName = "Detect Player (Peaceful)";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	TargetPlayerKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer_Peaceful, TargetPlayerKey), AActor::StaticClass());
	SpawnLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UUK_BTService_DetectPlayer_Peaceful, SpawnLocationKey));
}
#pragma endregion

#pragma region Player Detection
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

	// 공격자 최우선 (거리 무관)
	if (IsValid(Monster->Aggressor))
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, Monster->Aggressor);
		AIController->SetFocus(Monster->Aggressor); 
		return;
	}

	// ── 패스트패스: AIPerceptionComponent 결과 재사용 ───────────────────
	if (AUK_AiMonsterCtl* MonsterCtl = Cast<AUK_AiMonsterCtl>(AIController))
	{
		AActor* CtlTarget = MonsterCtl->GetCurrentTarget();
		if (CtlTarget)
			BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, CtlTarget);
		else
			BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		return;
	}

	// ── 폴백: 직접 PlayerController 순회 ────────────────────────────────
	const FVector MonsterLocation = ControlledPawn->GetActorLocation();
	AActor* BestTarget = nullptr;
	float   BestDist   = DetectionRadius;

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

		const float Dist = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist   = Dist;
			BestTarget = PlayerPawn;
		}
	}

	if (BestTarget)
	{
		BlackboardComp->SetValueAsObject(TargetPlayerKey.SelectedKeyName, BestTarget);
		AIController->SetFocus(BestTarget);
	}
	else
	{
		BlackboardComp->ClearValue(TargetPlayerKey.SelectedKeyName);
		AIController->ClearFocus(EAIFocusPriority::Gameplay); 
	}
}
#pragma endregion