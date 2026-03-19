// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/OutGame/UK_Out_LoadingLogo.h"
#include "UK_Out_Loading.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Out_Loading : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void LoadingLogoFunc(const FGeometry& MyGeometry, float InDeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void HandleLoadingComplete();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	TSubclassOf<class UUK_Out_LoadingLogo> LogoClass;

	UPROPERTY(BlueprintReadWrite, Category = "Loading")
	class UUK_Out_LoadingLogo* TargetLogoWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float CurrentPercentage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float TargetValue = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
	float InterpSpeed = 2.0f;

private:
	FTimerHandle FinishTimerHandle;
};
