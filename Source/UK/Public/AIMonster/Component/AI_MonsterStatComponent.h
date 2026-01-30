#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI_MonsterStatComponent.generated.h"

USTRUCT(BlueprintType)
struct FMonsterStats
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentHP = 100.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChangedDelegate, float, CurrentHP, float, MaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UAI_MonsterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UAI_MonsterStatComponent();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_Stats)
	FMonsterStats Stats;

	UFUNCTION()
	void OnRep_Stats();

public:

	/* Delegate */

	UPROPERTY(BlueprintAssignable)
	FOnDeathDelegate OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOnHPChangedDelegate OnHPChanged;

	float GetHP() const { return Stats.CurrentHP; }
	float GetMaxHP() const { return Stats.MaxHP; }
	bool IsDead() const;

	void TakeDamage(float Damage);
	void SetHP(float NewHP);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

