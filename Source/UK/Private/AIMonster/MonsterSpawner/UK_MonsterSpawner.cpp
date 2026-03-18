#include "AIMonster/MonsterSpawner/UK_MonsterSpawner.h"
#include "AIMonster/AIMonsterBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
}

void AUK_MonsterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	
	Super::EndPlay(EndPlayReason);
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
	const FVector BaseLocation = GetActorLocation();

	for (int32 Attempt = 0; Attempt < 20; Attempt++)
	{
		FVector Offset = UKismetMathLibrary::RandomUnitVector()
			* FMath::RandRange(MinCenterDistance, SpawnRadius);
		Offset.Z = 0.f;

		const FVector Candidate = BaseLocation + Offset;
		if (FVector::Dist2D(Candidate, BaseLocation) < MinCenterDistance) continue;

		bool bTooClose = false;
		for (AAIMonsterBase* Monster : ActiveMonsters)
		{
			if (IsValid(Monster) &&
				FVector::Dist2D(Candidate, Monster->GetActorLocation()) < MinSpawnDistance)
			{
				bTooClose = true;
				break;
			}
		}

		if (!bTooClose) return Candidate;
	}

	// 폴백
	FVector Fallback = UKismetMathLibrary::RandomUnitVector()
		* FMath::RandRange(MinCenterDistance, SpawnRadius);
	Fallback.Z = 0.f;
	return BaseLocation + Fallback;
}
#pragma endregion

#pragma region Object Pool
void AUK_MonsterSpawner::InitializeObjectPool()
{
	for (AAIMonsterBase* M : ObjectPool)    { if (IsValid(M)) M->Destroy(); }
	for (AAIController*  C : PooledControllers) { if (IsValid(C)) C->Destroy(); }
	ObjectPool.Empty();
	InactivePooledMonsters.Empty();
	PooledControllers.Empty();

	for (int32 i = 0; i < MaxMonsters; i++)
	{
		// ── 몬스터 생성 ──────────────────────────────────────────────────
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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

		// ── 컨트롤러 사전 생성 (Possess 하지 않음) ───────────────────────
		if (Monster->AIControllerClass)
		{
			FActorSpawnParameters CtrlParams;
			CtrlParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
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
    if (!Monster || !IsValid(Monster)) return;

    const FVector NewLocation = GetRandomSpawnLocation();
    Monster->SetActorLocation(NewLocation);
    Monster->SpawnLocation = NewLocation;
    Monster->SetActorHiddenInGame(false);
    Monster->SetActorEnableCollision(true);
    Monster->SetActorTickEnabled(true);

    if (UCapsuleComponent* Capsule = Monster->GetCapsuleComponent())
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    

    if (USkeletalMeshComponent* Mesh = Monster->GetMesh())
    {
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetAllBodiesSimulatePhysics(false);
    }

    if (UCharacterMovementComponent* Movement = Monster->GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Walking);
        Movement->Velocity = FVector::ZeroVector;
    }
	
    Monster->ResetHealth();
    Monster->bIsAttacking = false;
    Monster->bIsHit = false;
    Monster->bIsAggressive = false;
    Monster->Aggressor = nullptr;

    if (Monster->Personality == EMonsterPersonality::Peaceful)
        Monster->RequestState(EMonsterState::Passive);
    else
        Monster->RequestState(EMonsterState::Idle);
    
    AAIController* AICon = Cast<AAIController>(Monster->GetController());
	if (AICon)
	{
		AICon->StopMovement();
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
			Brain->StopLogic(TEXT("Re-pooling"));
		AICon->UnPossess();
		if (!PooledControllers.Contains(AICon))
			PooledControllers.Add(AICon);
		AICon = nullptr;
	}
	
	if (PooledControllers.Num() > 0)
		AICon = PooledControllers.Pop();
	else if (Monster->AIControllerClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.ObjectFlags |= RF_Transient;
		AICon = GetWorld()->SpawnActor<AAIController>(
			Monster->AIControllerClass, NewLocation, FRotator::ZeroRotator, SpawnInfo);
	}
	
	if (AICon)
		AICon->Possess(Monster);
	
	if (AICon)
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
			Brain->RestartLogic();
		else if (Monster->BehaviorTree)
			AICon->RunBehaviorTree(Monster->BehaviorTree);
		
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			BB->SetValueAsVector(TEXT("SpawnLocation"), NewLocation);
			BB->SetValueAsVector(TEXT("PatrolLocation"), NewLocation);
			BB->ClearValue(TEXT("TargetPlayer"));
		}
	}
	
	Monster->ResetAppearance();

    ActiveMonsters.Add(Monster);
    RegisterMonsterToGameMode(Monster);
}

void AUK_MonsterSpawner::DeactivateMonster(AAIMonsterBase* Monster)
{
	if (!Monster || !IsValid(Monster)) return;

	if (UWorld* World = Monster->GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(Monster);
	}

	Monster->SetActorHiddenInGame(true);
	Monster->SetActorEnableCollision(false);
	Monster->SetActorTickEnabled(false);

	if (AAIController* AICon = Cast<AAIController>(Monster->GetController()))
	{
		AICon->StopMovement();

		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Pooled"));
		}

		AICon->UnPossess();
		
		if (!PooledControllers.Contains(AICon))
		{
			PooledControllers.Add(AICon);
		}
	}

	ActiveMonsters.Remove(Monster);
}

void AUK_MonsterSpawner::OnMonsterDied(AAIMonsterBase* DeadMonster)
{
	if (!DeadMonster || !IsValid(DeadMonster)) return;

	TotalDeathCount++;
	ActiveMonsters.Remove(DeadMonster);
	ReturnMonsterToPool(DeadMonster);

	FTimerHandle RespawnTimer;
	FTimerDelegate RespawnDelegate;
	
	TWeakObjectPtr<AUK_MonsterSpawner> WeakThis(this);
	
	RespawnDelegate.BindLambda([WeakThis]()
	{
		if (!WeakThis.IsValid()) return;
		
		AUK_MonsterSpawner* Spawner = WeakThis.Get();
		if (Spawner->bIsSpawning && Spawner->ActiveMonsters.Num() < Spawner->MaxMonsters)
		{
			if (AAIMonsterBase* Monster = Spawner->GetMonsterFromPool())
			{
				Spawner->ActivateMonster(Monster);
			}
		}
	});

	GetWorldTimerManager().SetTimer(RespawnTimer, RespawnDelegate, RespawnDelay, false);
	RespawnTimers.Add(RespawnTimer);
}
#pragma endregion

#pragma region Game Mode Integration
void AUK_MonsterSpawner::RegisterMonsterToGameMode(AAIMonsterBase* Monster)
{
	if (!Monster) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AUKGameMode* GameMode = Cast<AUKGameMode>(World->GetAuthGameMode());
	if (!GameMode) return;

	GameMode->OnMonsterSpawned(Monster);
}
#pragma endregion