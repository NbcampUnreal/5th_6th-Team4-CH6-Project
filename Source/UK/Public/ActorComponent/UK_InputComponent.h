// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/UK_InputConfig.h"
#include "UK_InputComponent.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_InputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:
	template<class UserObject, typename callBlakFuncion>
	void BindInputAction(
		const UUK_InputConfig* InputConfig,
		const FGameplayTag& Tag,
		ETriggerEvent TrigerEvent,
		UserObject* ContextObject,
		callBlakFuncion Funcion
	);
};
template<class UserObject, typename callBlakFuncion>
inline void UUK_InputComponent::BindInputAction(
	const UUK_InputConfig* InputConfig,
	const FGameplayTag& Tag,
	ETriggerEvent TrigerEvent,
	UserObject* ContextObject,
	callBlakFuncion Funcion
)
{
	checkf(InputConfig, TEXT("Input DataAsset is nullptr"));
	if (UInputAction* FoundAction = InputConfig->FindNativeInputActionByTag(Tag))
	{
		BindAction(FoundAction, TrigerEvent, ContextObject, Funcion);
	}
	else
	{
		ensureMsgf(false, TEXT("InputAction not found for Tag: %s"), *Tag.ToString());
	}
}