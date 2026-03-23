// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_MonsterSpawner.generated.h"

class AAIMonsterBase;
class AAIController;

UCLASS()
class UK_API AUK_MonsterSpawner : public AActor
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AUK_MonsterSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

#pragma region Spawner Settings
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	TSubclassOf<AAIMonsterBase> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	int32 MaxMonsters = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float SpawnRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float RespawnDelay = 10.0f;

	/** 몬스터 간 최소 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinSpawnDistance = 200.0f;

	/** 스포너 중심에서 최소 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinCenterDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Debug")
	bool bShowDebugInfo = true;
#pragma endregion

#pragma region Spawning Control
public:
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	bool StopSpawning();

	UFUNCTION()
	void OnMonsterDied(AAIMonsterBase* DeadMonster);
private:
	int32 TriggerRefCount = 0;
#pragma endregion

#pragma region Object Pool
public:                                    
	void InitializeObjectPool();
	TArray<AAIMonsterBase*> GetActiveMonsters() const { return ActiveMonsters; }
	AAIMonsterBase* GetMonsterFromPool();
	void ReturnMonsterToPool(AAIMonsterBase* Monster);
    
private:
	UPROPERTY()
	TArray<AAIMonsterBase*> ObjectPool;

	UPROPERTY()
	TArray<AAIMonsterBase*> InactivePooledMonsters;

	UPROPERTY()
	TArray<AAIController*> PooledControllers;
#pragma endregion

#pragma region Monster Lifecycle
private:
	void SpawnInitialMonsters();
	FVector GetRandomSpawnLocation() const;
	void ActivateMonster(AAIMonsterBase* Monster);
	void DeactivateMonster(AAIMonsterBase* Monster);

	UPROPERTY()
	TArray<AAIMonsterBase*> ActiveMonsters;

	TArray<FTimerHandle> RespawnTimers;
	bool bIsSpawning = false;
#pragma endregion

#pragma region Game Mode Integration
private:
	void RegisterMonsterToGameMode(AAIMonsterBase* Monster);
#pragma endregion

#pragma region Statistics
private:
	int32 TotalSpawnCount = 0;
	int32 TotalDeathCount = 0;
#pragma endregion
	
#pragma region Distance LOD
private:
	void UpdateMonsterLOD();
    
	UPROPERTY(EditAnywhere, Category = "Spawner|LOD")
	float LODDistance = 3000.f;
    
	FTimerHandle LODTimerHandle;
#pragma endregion
};