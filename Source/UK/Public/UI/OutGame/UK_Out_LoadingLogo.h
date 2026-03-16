// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_Out_LoadingLogo.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_Out_LoadingLogo : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "Loading")
	void UpdatePercentage(float NewPercentage);

};
