// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_MarkerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void SetOriginLocation(FVector InLocation);

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeInAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOutAnim;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Location;
	
	UFUNCTION()
	void PlayFadeIn();
	
	UFUNCTION()
	void PlayFadeOut();
private:
	FVector OriginLocation;
	APawn* PlayerPawn;

	float LastDistance = 0.f;

	FTimerHandle DistanceCheckTimer;

	void CheckDistance();
};
