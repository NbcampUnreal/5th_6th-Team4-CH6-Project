// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UK_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatusComponent();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#pragma region GetSet
	int GetMaxHP() const { return MaxHp; }
	void SetMaxHP(const int32 CurrentHp) { MaxHp = CurrentHp; };

	int GetMaxMp() const { return MaxMp; };
	void SetMaxMp(const int32 CurrentMp) { MaxMp = CurrentMp; };

	int GetStr() const { return Str; };
	void SetStr(const int32 CurrentStr) { Str = CurrentStr; };
#pragma endregion
protected:
	UPROPERTY()
	int32 MaxHp;
	UPROPERTY()
	int32 MaxMp;
	UPROPERTY()
	int32 Str;
};
