// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UK_InputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class UK_API UUK_InputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UInputAction* FindNativeInputActionByTag(const FGameplayTag& InputTag) const;
	UInputMappingContext* GetIMC()const { return DefaultIMC; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputMappingContext* DefaultIMC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = ( TitleProperty = "InputTag" ))
	TMap<FGameplayTag, TObjectPtr <UInputAction>> NativeInputAction;

};
