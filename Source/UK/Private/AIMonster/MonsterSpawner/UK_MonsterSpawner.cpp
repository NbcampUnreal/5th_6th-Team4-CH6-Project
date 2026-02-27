#include "AIMonster/MonsterSpawner/UK_MonsterSpawner.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BrainComponent.h"
#include "GameFramework/GameModeBase.h"
#include "server/UKGameMode.h"

#pragma region Initialization
AUK_MonsterSpawner::AUK_MonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AUK_MonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (MonsterClass)
	{
		InitializeObjectPool();
		StartSpawning();
	}
}
#pragma endregion

#pragma region Spawning Control
void AUK_MonsterSpawner::StartSpawning()
{
	if (bIsSpawning) return;

	bIsSpawning = true;
	SpawnInitialMonsters();
}

void AUK_MonsterSpawner::StopSpawning()
{
	bIsSpawning = false;

	for (FTimerHandle& Timer : RespawnTimers)
	{
		if (Timer.IsValid())
		{
			GetWorldTimerManager().ClearTimer(Timer);
		}
	}
	RespawnTimers.Empty();
}

void AUK_MonsterSpawner::SpawnInitialMonsters()
{
	for (int32 i = 0; i < MaxMonsters; i++)
	{
		if (AAIMonsterBase* Monster = GetMonsterFromPool())
		{
			ActivateMonster(Monster);
		}
	}
}

FVector AUK_MonsterSpawner::GetRandomSpawnLocation() const
{
	const FVector BaseLocation           = GetActorLocation();
	const float   MinDistBetweenMonsters = 300.0f;
	const float   MinDistFromCenter      = 150.0f;

	for (int32 Attempt = 0; Attempt < 20; Attempt++)
	{
		FVector RandomOffset = UKismetMathLibrary::RandomUnitVector()
			* FMath::RandRange(MinDistFromCenter, SpawnRadius);
		RandomOffset.Z = 0.0f;

		const FVector Candidate = BaseLocation + RandomOffset;
		if (FVector::Dist2D(Candidate, BaseLocation) < MinDistFromCenter) continue;

		bool bTooClose = false;
		for (AAIMonsterBase* Monster : ActiveMonsters)
		{
			if (IsValid(Monster) && FVector::Dist2D(Candidate, Monster->GetActorLocation()) < MinDistBetweenMonsters)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose) return Candidate;
	}

	FVector Fallback = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(MinDistFromCenter, SpawnRadius);
	Fallback.Z = 0.0f;
	return BaseLocation + Fallback;
}
#pragma endregion

#pragma region Object Pool
void AUK_MonsterSpawner::InitializeObjectPool()
{
	for (AAIMonsterBase* Monster : ObjectPool)  { if (IsValid(Monster)) Monster->Destroy(); }
	ObjectPool.Empty();
	InactivePooledMonsters.Empty();

	for (AAIController* Ctrl : PooledControllers) { if (IsValid(Ctrl)) Ctrl->Destroy(); }
	PooledControllers.Empty();

	for (int32 i = 0; i < MaxMonsters; i++)
	{
		// ── 몬스터 생성 ─────────────────────────────────────────────────
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AAIMonsterBase* Monster = GetWorld()->SpawnActor<AAIMonsterBase>(
			MonsterClass, GetRandomSpawnLocation(), FRotator::ZeroRotator, SpawnParams);
		if (!Monster) continue;

		Monster->OwningSpawner = this;
		Monster->OnDeath.AddDynamic(this, &AUK_MonsterSpawner::OnMonsterDied);
		Monster->SetActorHiddenInGame(true);
		Monster->SetActorEnableCollision(false);
		Monster->SetActorTickEnabled(false);

		if (AController* AutoCtrl = Monster->GetController())
		{
			AutoCtrl->UnPossess();
		}

		ObjectPool.Add(Monster);
		InactivePooledMonsters.Add(Monster);
		TotalSpawnCount++;

		// ── 컨트롤러 사전 생성 (Possess 하지 않음) ──────────────────────
		if (Monster->AIControllerClass)
		{
			FActorSpawnParameters CtrlParams;
			CtrlParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			CtrlParams.ObjectFlags |= RF_Transient;

			if (AAIController* Ctrl = GetWorld()->SpawnActor<AAIController>(
				Monster->AIControllerClass,
				Monster->GetActorLocation(), FRotator::ZeroRotator, CtrlParams))
			{
				PooledControllers.Add(Ctrl);
			}
		}
	}
}

AAIMonsterBase* AUK_MonsterSpawner::GetMonsterFromPool()
{
	if (InactivePooledMonsters.Num() > 0) return InactivePooledMonsters.Pop();
	return nullptr;
}

void AUK_MonsterSpawner::ReturnMonsterToPool(AAIMonsterBase* Monster)
{
	if (!Monster) return;
	DeactivateMonster(Monster);
	if (!InactivePooledMonsters.Contains(Monster))
	{
		InactivePooledMonsters.Add(Monster);
	}
}
#pragma endregion

#pragma region Monster Lifecycle
void AUK_MonsterSpawner::ActivateMonster(AAIMonsterBase* Monster)
{
	if (!Monster) return;

	const FVector NewLocation  = GetRandomSpawnLocation();
	Monster->SetActorLocation(NewLocation);
	Monster->SpawnLocation = NewLocation;

	Monster->SetActorHiddenInGame(false);
	Monster->SetActorEnableCollision(true);
	Monster->SetActorTickEnabled(true);

	// 컨트롤러: 풀에서 꺼내거나 새로 스폰
	AAIController* AIController = Cast<AAIController>(Monster->GetController());
	if (!AIController)
	{
		if (PooledControllers.Num() > 0)
		{
			AIController = PooledControllers.Pop();
		}
		else if (Monster->AIControllerClass)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.ObjectFlags |= RF_Transient;

			AIController = GetWorld()->SpawnActor<AAIController>(
				Monster->AIControllerClass,
				NewLocation, FRotator::ZeroRotator, SpawnInfo);
		}

		if (AIController) AIController->Possess(Monster);
	}

	// BT 재시작 및 블랙보드 초기화
	if (AIController)
	{
		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->RestartLogic();
		}
		else if (Monster->BehaviorTree)
		{
			AIController->RunBehaviorTree(Monster->BehaviorTree);
		}

		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), NewLocation);
			BB->SetValueAsVector(TEXT("PatrolLocation"), NewLocation);
			BB->ClearValue(TEXT("TargetPlayer"));
		}
	}

	Monster->ResetHealth();
	ActiveMonsters.Add(Monster);
	RegisterMonsterToGameMode(Monster);
}

void AUK_MonsterSpawner::DeactivateMonster(AAIMonsterBase* Monster)
{
	if (!Monster) return;

	Monster->SetActorHiddenInGame(true);
	Monster->SetActorEnableCollision(false);
	Monster->SetActorTickEnabled(false);

	if (AAIController* AIController = Cast<AAIController>(Monster->GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
		{
			BrainComp->StopLogic(TEXT("Pooled"));
		}

		// UnPossess 후 풀로 반환 (Destroy 대신)
		Monster->DetachFromControllerPendingDestroy();
		AIController->UnPossess();
		PooledControllers.Add(AIController);
	}

	ActiveMonsters.Remove(Monster);
}

void AUK_MonsterSpawner::OnMonsterDied(AAIMonsterBase* DeadMonster)
{
	if (!DeadMonster) return;

	TotalDeathCount++;
	ActiveMonsters.Remove(DeadMonster);
	ReturnMonsterToPool(DeadMonster);

	FTimerHandle RespawnTimer;
	GetWorldTimerManager().SetTimer(RespawnTimer, [this]()
	{
		if (bIsSpawning && ActiveMonsters.Num() < MaxMonsters)
		{
			if (AAIMonsterBase* Monster = GetMonsterFromPool())
			{
				ActivateMonster(Monster);
			}
		}
	}, RespawnDelay, false);

	RespawnTimers.Add(RespawnTimer);
}
#pragma endregion

#pragma region Cleanup
void AUK_MonsterSpawner::ClearAllMonsters()
{
	StopSpawning();

	for (AAIMonsterBase* Monster : ActiveMonsters) { if (IsValid(Monster)) Monster->Destroy(); }
	ActiveMonsters.Empty();

	for (AAIMonsterBase* Monster : ObjectPool) { if (IsValid(Monster)) Monster->Destroy(); }
	ObjectPool.Empty();
	InactivePooledMonsters.Empty();

	for (AAIController* Ctrl : PooledControllers) { if (IsValid(Ctrl)) Ctrl->Destroy(); }
	PooledControllers.Empty();

	TotalSpawnCount = 0;
	TotalDeathCount = 0;
}
#pragma endregion

#pragma region Game Mode Integration
void AUK_MonsterSpawner::RegisterMonsterToGameMode(AAIMonsterBase* Monster)
{
	if (!Monster || !HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AUKGameMode* GameMode = Cast<AUKGameMode>(World->GetAuthGameMode());
	if (!GameMode) return;

	GameMode->OnMonsterSpawned(Monster);
}
#pragma endregion