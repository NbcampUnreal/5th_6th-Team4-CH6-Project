#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI_MonsterStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHPChanged, float, NewHP);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UK_API UAI_MonsterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAI_MonsterStatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHP = 100.0f;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnHPChanged OnHPChanged;


	// Damage
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void TakeDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetHP(float NewHP);

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHP() const { return MaxHP; }

	// Death Event
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnMonsterDeathSignature OnDeath;
};