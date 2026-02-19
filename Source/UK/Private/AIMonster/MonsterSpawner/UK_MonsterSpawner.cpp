// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/MonsterSpawner/UK_MonsterSpawner.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/UK_AiMonsterCtl.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BrainComponent.h"
#include "GameFramework/GameModeBase.h"
#include "server/UKGameMode.h"

AUK_MonsterSpawner::AUK_MonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UK_MonsterSpawner: MonsterClass is not set!"));
    }
}

void AUK_MonsterSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bShowDebugInfo)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnRadius, 32, FColor::Green, false, -1.0f, 0, 2.0f);
        
        FString InfoText = FString::Printf(TEXT("Active: %d/%d\nSpawned: %d\nDeaths: %d\nPooled: %d"),
            ActiveMonsters.Num(),
            MaxMonsters,
            TotalSpawnCount,
            TotalDeathCount,
            InactivePooledMonsters.Num());
    }
}

void AUK_MonsterSpawner::StartSpawning()
{
    if (bIsSpawning)
    {
        return;
    }

    bIsSpawning = true;
    SpawnInitialMonsters();
    
    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Started spawning"));
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
    
    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Stopped spawning"));
}

void AUK_MonsterSpawner::ClearAllMonsters()
{
    StopSpawning();

    for (AAIMonsterBase* Monster : ActiveMonsters)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }
    ActiveMonsters.Empty();

    for (AAIMonsterBase* Monster : ObjectPool)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }
    ObjectPool.Empty();
    InactivePooledMonsters.Empty();

    TotalSpawnCount = 0;
    TotalDeathCount = 0;

    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Cleared all monsters"));
}

void AUK_MonsterSpawner::OnMonsterDied(AAIMonsterBase* DeadMonster)
{
    if (!DeadMonster)
    {
        return;
    }

    TotalDeathCount++;
    ActiveMonsters.Remove(DeadMonster);

    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Monster died. Deaths: %d, Active: %d"), 
        TotalDeathCount, ActiveMonsters.Num());

    ReturnMonsterToPool(DeadMonster);
    
    FTimerHandle RespawnTimer;
    GetWorldTimerManager().SetTimer(RespawnTimer, [this]()
    {
        if (bIsSpawning && ActiveMonsters.Num() < MaxMonsters)
        {
            AAIMonsterBase* Monster = GetMonsterFromPool();
            if (Monster)
            {
                ActivateMonster(Monster);
            }
        }
    }, RespawnDelay, false);
    
    RespawnTimers.Add(RespawnTimer);
}

void AUK_MonsterSpawner::SpawnInitialMonsters()
{
    for (int32 i = 0; i < MaxMonsters; i++)
    {
        AAIMonsterBase* Monster = GetMonsterFromPool();
        if (Monster)
        {
            ActivateMonster(Monster);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Spawned %d initial monsters"), ActiveMonsters.Num());
}

FVector AUK_MonsterSpawner::GetRandomSpawnLocation() const
{
    FVector BaseLocation = GetActorLocation();
    
    const int32 MaxAttempts = 20;
    const float MinDistanceBetweenMonsters = 200.0f;
    const float MinDistanceFromCenter = 150.0f;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(MinDistanceFromCenter, SpawnRadius);
        RandomOffset.Z = 0.0f;
        
        FVector CandidateLocation = BaseLocation + RandomOffset;
        
        // 스포너 중심과 거리 체크
        if (FVector::Dist2D(CandidateLocation, BaseLocation) < MinDistanceFromCenter)
        {
            continue;
        }
        
        // 기존 활성 몬스터들과 거리 체크
        bool bTooClose = false;
        for (AAIMonsterBase* Monster : ActiveMonsters)
        {
            if (IsValid(Monster))
            {
                if (FVector::Dist2D(CandidateLocation, Monster->GetActorLocation()) < MinDistanceBetweenMonsters)
                {
                    bTooClose = true;
                    break;
                }
            }
        }
        
        if (!bTooClose)
        {
            return CandidateLocation;
        }
    }
    
    // 폴백 - 최소한 중심에서는 떨어뜨리기
    FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(MinDistanceFromCenter, SpawnRadius);
    RandomOffset.Z = 0.0f;
    return BaseLocation + RandomOffset;
}

void AUK_MonsterSpawner::InitializeObjectPool()
{
    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Initializing object pool with %d monsters"), MaxMonsters);

    for (AAIMonsterBase* Monster : ObjectPool)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }
    ObjectPool.Empty();
    InactivePooledMonsters.Empty();

    for (int32 i = 0; i < MaxMonsters; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AAIMonsterBase* Monster = GetWorld()->SpawnActor<AAIMonsterBase>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);
        
        if (Monster)
        {
            // 스포너 참조 설정
            Monster->OwningSpawner = this;
            Monster->OnDeath.AddDynamic(this, &AUK_MonsterSpawner::OnMonsterDied);
            
            // 비활성 상태로 시작
            Monster->SetActorHiddenInGame(true);
            Monster->SetActorEnableCollision(false);
            Monster->SetActorTickEnabled(false);
            
            // AIController 제거 (재활성화 시 다시 생성)
            if (AAIController* AIController = Cast<AAIController>(Monster->GetController()))
            {
                Monster->DetachFromControllerPendingDestroy();
                AIController->UnPossess();
            }
            
            ObjectPool.Add(Monster);
            InactivePooledMonsters.Add(Monster);
            
            TotalSpawnCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Object pool initialized with %d monsters"), ObjectPool.Num());
}

AAIMonsterBase* AUK_MonsterSpawner::GetMonsterFromPool()
{
    if (InactivePooledMonsters.Num() > 0)
    {
        return InactivePooledMonsters.Pop();
    }
    
    return nullptr;
}

void AUK_MonsterSpawner::ReturnMonsterToPool(AAIMonsterBase* Monster)
{
    if (!Monster)
    {
        return;
    }

    DeactivateMonster(Monster);
    
    if (!InactivePooledMonsters.Contains(Monster))
    {
        InactivePooledMonsters.Add(Monster);
    }
}

void AUK_MonsterSpawner::ActivateMonster(AAIMonsterBase* Monster)
{
    if (!Monster)
    {
        return;
    }

    // 새 위치로 이동
    FVector NewLocation = GetRandomSpawnLocation();
    Monster->SetActorLocation(NewLocation);
    Monster->SpawnLocation = NewLocation;

    // 활성화
    Monster->SetActorHiddenInGame(false);
    Monster->SetActorEnableCollision(true);
    Monster->SetActorTickEnabled(true);

    AAIController* AIController = Cast<AAIController>(Monster->GetController());
    
    if (!AIController && Monster->AIControllerClass)
    {
        // AIController 생성
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.Instigator = Monster->GetInstigator();
        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnInfo.OverrideLevel = Monster->GetLevel();
        SpawnInfo.ObjectFlags |= RF_Transient;
        
        AIController = GetWorld()->SpawnActor<AAIController>(
            Monster->AIControllerClass, 
            Monster->GetActorLocation(), 
            Monster->GetActorRotation(), 
            SpawnInfo
        );
        
        if (AIController)
        {
            AIController->Possess(Monster);
        }
    }
    else if (AIController)
    {
        // 이미 AIController 있으면 재시작
        if (UBrainComponent* BrainComp = AIController->GetBrainComponent())
        {
            BrainComp->RestartLogic();
        }
        else if (Monster->BehaviorTree)
        {
            AIController->RunBehaviorTree(Monster->BehaviorTree);
        }
        
        // Blackboard 초기화
        if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsVector(TEXT("SpawnLocation"), NewLocation);
            BlackboardComp->SetValueAsVector(TEXT("PatrolLocation"), NewLocation);
            BlackboardComp->ClearValue(TEXT("TargetPlayer"));
        }
    }

    // 체력과 상태 초기화
    Monster->ResetHealth();

    // HP 확인 로그
    if (UAI_MonsterStatComponent* StatComp = Monster->GetStatComponent())
    {
        UE_LOG(LogTemp, Warning, TEXT("Monster Respawn: %s | HP: %.1f / %.1f | Location: %s"), 
            *Monster->GetName(),
            StatComp->GetHP(),
            StatComp->GetMaxHP(),
            *NewLocation.ToString());
    }
    
    ActiveMonsters.Add(Monster);

    // GameMode에 몬스터 등록 (킬 알림 시스템)
    RegisterMonsterToGameMode(Monster);

    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Activated monster from pool. Active: %d"), ActiveMonsters.Num());
}

void AUK_MonsterSpawner::RegisterMonsterToGameMode(AAIMonsterBase* Monster)
{
    if (!Monster)
    {
        UE_LOG(LogTemp, Error, TEXT("[Spawner] RegisterMonsterToGameMode: Monster is NULL!"));
        return;
    }

    // World 체크
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[Spawner] RegisterMonsterToGameMode: World is NULL!"));
        return;
    }

    // Authority 체크 (서버인지 확인)
    if (!HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Spawner] RegisterMonsterToGameMode: Not on server (Client)"));
        return;
    }

    // GameMode 가져오기
    AGameModeBase* GameModeBase = World->GetAuthGameMode();
    if (!GameModeBase)
    {
        UE_LOG(LogTemp, Error, TEXT("[Spawner] RegisterMonsterToGameMode: GameMode is NULL!"));
        return;
    }

    // GameMode 타입 확인
    UE_LOG(LogTemp, Warning, TEXT("[Spawner] GameMode found: %s"), *GameModeBase->GetClass()->GetName());

    // AUKGameMode로 캐스팅
    AUKGameMode* GameMode = Cast<AUKGameMode>(GameModeBase);
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("[Spawner] RegisterMonsterToGameMode: Failed to cast to AUKGameMode!"));
        UE_LOG(LogTemp, Error, TEXT("[Spawner] Current GameMode class: %s"), *GameModeBase->GetClass()->GetName());
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Spawner] Successfully cast to AUKGameMode! Calling OnMonsterSpawned..."));
    GameMode->OnMonsterSpawned(Monster);
    
    UE_LOG(LogTemp, Warning, TEXT("[Spawner] Successfully registered %s to GameMode"), *Monster->GetName());
}

void AUK_MonsterSpawner::DeactivateMonster(AAIMonsterBase* Monster)
{
    if (!Monster)
    {
        return;
    }

    // 비활성화
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
        
        // AIController 제거 (재활성화 시 새로 생성)
        Monster->DetachFromControllerPendingDestroy();
        AIController->UnPossess();
        
        UE_LOG(LogTemp, Log, TEXT("AIController removed from %s"), *Monster->GetName());
    }

    ActiveMonsters.Remove(Monster);

    UE_LOG(LogTemp, Log, TEXT("UK_MonsterSpawner: Deactivated monster. Active: %d, Pooled: %d"), 
        ActiveMonsters.Num(), InactivePooledMonsters.Num());
}