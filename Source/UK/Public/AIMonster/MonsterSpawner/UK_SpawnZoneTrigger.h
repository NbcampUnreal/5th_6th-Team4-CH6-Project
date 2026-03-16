#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_SpawnZoneTrigger.generated.h"

class AUK_MonsterSpawner;
class USphereComponent;

UCLASS()
class UK_API AUK_SpawnZoneTrigger : public AActor
{
	GENERATED_BODY()

#pragma region Initialization
public:
	AUK_SpawnZoneTrigger();
protected:
	virtual void BeginPlay() override;
#pragma endregion

#pragma region Settings
public:
	/** 이 트리거가 관리할 스포너 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZoneTrigger")
	TArray<AUK_MonsterSpawner*> ManagedSpawners;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZoneTrigger")
	float ActivationRadius = 3000.0f;
#pragma endregion

#pragma region Components
private:
	UPROPERTY(VisibleAnywhere, Category = "ZoneTrigger")
	USphereComponent* TriggerSphere;
#pragma endregion

#pragma region Overlap
private:
	UFUNCTION()
	void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	int32 PlayerRefCount = 0; // 멀티 플레이어 대비
#pragma endregion
};