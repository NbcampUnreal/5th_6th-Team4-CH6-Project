#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UK_BossProjectileBase.generated.h"

UCLASS()
class UK_API AUK_BossProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AUK_BossProjectileBase();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
public:	
	UPROPERTY(VisibleAnywhere, Category= "Components")
	class USphereComponent* Collisioncomp;
	
	UPROPERTY(VisibleAnywhere, Category= "Components")
	class UProjectileMovementComponent* ProjectileMovementcomp;
	
	UPROPERTY(VisibleAnywhere, Category= "Effects")
	class UNiagaraSystem* HitEffect;
	
	float DamageAmount = 20.f;
};
