#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "UK_WarpTower.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UGameplayEffect;
class UChildActorComponent;
class AUK_CheckPoint;

UCLASS()
class UK_API AUK_WarpTower : public AActor
{
	GENERATED_BODY()

public:
	AUK_WarpTower();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleQuestMarkerResetRequested();

	UFUNCTION()
	void HandleQuestMarkerRouteResolved(FName QuestId, EUKQuestMarkerTargetType TargetType, FName TargetId);

	void RefreshWarpMarker();
	void HideMarkerInternal();
	void ShowMarkerInternal(EUKQuestMarkerState MarkerState);
	void UpdateMarkerRotation();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> MarkerComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarpTower|Marker")
	TSubclassOf<AUK_CheckPoint> SelectMarker;

	UPROPERTY()
	TObjectPtr<AUK_CheckPoint> WarpMarker;

	// 상태 변수 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarpTower", meta = ( AllowPrivateAccess = "true" ))
	bool bIsActivated = false;

	FTimerHandle MarkerTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WarpTower|Marker")
	bool bPlayerInRange = false;

	// 회복을 위한 GameplayEffect (블루프린트에서 할당) 
	UPROPERTY(EditAnywhere, Category = "WarpTower|GAS")
	TSubclassOf<UGameplayEffect> RecoveryEffectClass;

	// 활성화 시 발생할 이펙트/사운드 
	UFUNCTION(BlueprintImplementableEvent, Category = "WarpTower")
	void OnTowerActivated();

protected:
	// 타워 고유 식별자 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarpTower")
	FName WarpPointID;
};