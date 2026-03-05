#include "AIMonster/BossMonster/UK_BossAIController.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AUK_BossAIController::AUK_BossAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}
void AUK_BossAIController::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (Player)
	{
		GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"),Player);
		UE_LOG(LogTemp, Warning, TEXT("Target Set"));
	}
}
void AUK_BossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("Boss Possessed"));
	if (!HasAuthority()) return;

	Boss = Cast<AUK_BossMonsterBase>(InPawn);
	if (!Boss) return;

	if (Boss->BehaviorTree)
	{
		RunBehaviorTree(Boss->BehaviorTree);
	}
	if (auto* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(TEXT("HomeLocation"),InPawn->GetActorLocation());
	}
	GetWorld()->GetTimerManager().SetTimer(
		PhaseSyncTimer,
		this,
		&AUK_BossAIController::SyncBossPhaseToBB,
		PhaseSyncInterval,
		true
	);

	SyncBossPhaseToBB();
}

void AUK_BossAIController::OnUnPossess()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(PhaseSyncTimer);
	}

	Boss = nullptr;

	Super::OnUnPossess();
}

void AUK_BossAIController::SyncBossPhaseToBB()
{
	if (!HasAuthority()) return;
	if (!Boss) return;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	const FGameplayTag PhaseTag = Boss->GetCurrentPhase();
	const FName PhaseName = PhaseTag.GetTagName();

	if (BB->GetValueAsName(TEXT("BossPhase")) != PhaseName)
	{
		BB->SetValueAsName(TEXT("BossPhase"), PhaseName);
	}
}