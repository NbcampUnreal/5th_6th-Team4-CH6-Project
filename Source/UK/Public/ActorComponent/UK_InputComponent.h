// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/UK_InputConfig.h"
#include "GameplayTagContainer.h"
#include "UK_InputComponent.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_InputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:

	template<class UserObject, typename CallbackFunc>
	void BindNativeInputAction(
		const UUK_InputConfig* InputActionConfig,
		const FGameplayTag& InputTag,
		ETriggerEvent TriggerEvent,
		UserObject* ContextObject,
		CallbackFunc Func);
};

template<class UserObject, typename CallbackFunc>
inline void UUK_InputComponent::BindNativeInputAction(const UUK_InputConfig* InputActionConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func)
{
	if ( UInputAction* FoundAction = InputActionConfig->FindNativeInputActionByTag(InputTag) )
	{
		BindAction(FoundAction, TriggerEvent, ContextObject, Func);
	}
}
