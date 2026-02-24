// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

USTRUCT(BlueprintType)
struct FStatus
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	int32 MaxLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHp = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHp = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxMp = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentMp = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float Power = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentPower = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxStamina = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentStamina = 10.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeadDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpStatusDelegate, float, CurrentHp, float, MaxHp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMpStatusDelegate, float, CurrentMp, float, MaxMp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStatusDelegate, int32, Level);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerStatusDelegate, float, Power);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaStatusDelegate, float, CurrentStamina, float, MaxStamina);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatusComponent();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
#pragma region Status
public:
	void SetHp(const float CurrentHp);

	void SetMp(const float CurrentMp);

	void TakeMP(const float CurrentMp);

	void LevelUp();

	void SetStamina(const float CurrentStamina);

	void TakeStamina(const float CurrentStamina);

	bool IsDead() const;

	UFUNCTION()
	void OnRepStatus();
protected:

	UPROPERTY(ReplicatedUsing = OnRepStatus)
	FStatus Status;

#pragma endregion

#pragma region Battle
public:
	float ApplyDamage();

	void TakeDamage(float Damage);
#pragma endregion

#pragma region Delegate
	UPROPERTY(BlueprintAssignable)
	FOnHpStatusDelegate HpStatusDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnMpStatusDelegate MpStatusDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLevelStatusDelegate LevelStatusDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnPowerStatusDelegate PowerStatusDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnStaminaStatusDelegate StaminaStatusDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnDeadDelegate OnDeadDelegate;
#pragma endregion
};
