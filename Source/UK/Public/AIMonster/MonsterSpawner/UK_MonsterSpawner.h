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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
	float MinSpawnDistance = 200.0f;

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
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ClearAllMonsters();

	UFUNCTION()
	void OnMonsterDied(AAIMonsterBase* DeadMonster);
#pragma endregion

#pragma region Object Pool
private:
	void InitializeObjectPool();
	AAIMonsterBase* GetMonsterFromPool();
	void ReturnMonsterToPool(AAIMonsterBase* Monster);

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
};