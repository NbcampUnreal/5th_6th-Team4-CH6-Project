// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "AbilitySystemComponent.h"
#include "UK_Stamina.generated.h"

/**
 * 
 */
class AUK_CharacterBase;
UCLASS()
class UK_API UUK_Stamina : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:

	// PlayerController가 연결해줄 함수
	//void BindStatusComponent(UStatusComponent* NewStatusComp);

	// 스테미나 업데이트
	//UFUNCTION()
	void UpdateStaminaBar(const FOnAttributeChangeData& Data);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI", meta = ( DisplayName = "OnUpdateStaminaPercent" ))
	void K2_OnUpdateStaminaPercent(float NewPercent);

	UFUNCTION()
	void SetTrackingPosition(const FVector2D& ScreenPos, float Scale, bool bVisible);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = ( BindWidget ))
	UImage* StaminaBar;

	UPROPERTY(BlueprintReadWrite, Category = "UI|Stamina")
	float Percent;
private:
	UPROPERTY()
	UMaterialInstanceDynamic* StaminaMID;

	// UPROPERTY()
	// UStatusComponent* CachedStatusComp;
	
	UPROPERTY()
	AUK_CharacterBase* PlayerPawn;
	
	UPROPERTY()
	UAbilitySystemComponent* ASC;
};
