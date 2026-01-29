// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/UK_InputConfig.h"

UInputAction* UUK_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InputTag) const
{
    if (const TObjectPtr<UInputAction>* FoundAction = NativeInputActions.Find(InputTag))
    {
        return *FoundAction;
    }
    return nullptr;
}
