#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_WarpTower.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UGameplayEffect;

UCLASS()
class UK_API AUK_WarpTower : public AActor
{
	GENERATED_BODY()

public:
	AUK_WarpTower();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	// 상태 변수 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarpTower", meta = ( AllowPrivateAccess = "true" ))
	bool bIsActivated = false;

	// 회복을 위한 GameplayEffect (블루프린트에서 할당) 
	UPROPERTY(EditAnywhere, Category = "WarpTower|GAS")
	TSubclassOf<UGameplayEffect> RecoveryEffectClass;

	// 활성화 시 발생할 이펙트/사운드 
	UFUNCTION(BlueprintImplementableEvent, Category = "WarpTower")
	void OnTowerActivated();
};