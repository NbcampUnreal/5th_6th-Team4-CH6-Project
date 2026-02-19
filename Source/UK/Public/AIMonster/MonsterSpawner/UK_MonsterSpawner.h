// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_MonsterSpawner.generated.h"

class AAIMonsterBase;

UCLASS()
class UK_API AUK_MonsterSpawner : public AActor
{
    GENERATED_BODY()
    
public:    
    AUK_MonsterSpawner();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // 스포너 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    TSubclassOf<AAIMonsterBase> MonsterClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    int32 MaxMonsters = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    float RespawnDelay = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinSpawnDistance = 200.0f;  // 몬스터 간 최소 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinCenterDistance = 150.0f;  // 스포너 중심에서 최소 거리

    // 디버그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Debug")
    bool bShowDebugInfo = true;

    // 스포너 제어
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StartSpawning();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StopSpawning();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void ClearAllMonsters();

    // 몬스터 사망 처리
    UFUNCTION()
    void OnMonsterDied(AAIMonsterBase* DeadMonster);

private:
    // 내부 함수
    void SpawnInitialMonsters();
    FVector GetRandomSpawnLocation() const;
    
    // Object Pooling
    void InitializeObjectPool();
    AAIMonsterBase* GetMonsterFromPool();
    void ReturnMonsterToPool(AAIMonsterBase* Monster);
    void ActivateMonster(AAIMonsterBase* Monster);
    void DeactivateMonster(AAIMonsterBase* Monster);
    
    // GameMode 등록
    void RegisterMonsterToGameMode(AAIMonsterBase* Monster);

    // 스포너 상태
    UPROPERTY()
    TArray<AAIMonsterBase*> ActiveMonsters;

    UPROPERTY()
    TArray<AAIMonsterBase*> ObjectPool;

    UPROPERTY()
    TArray<AAIMonsterBase*> InactivePooledMonsters;

    TArray<FTimerHandle> RespawnTimers;

    bool bIsSpawning = false;

    // 통계
    int32 TotalSpawnCount = 0;
    int32 TotalDeathCount = 0;
};