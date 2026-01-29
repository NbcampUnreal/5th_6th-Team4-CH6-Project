// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/UK_WeaponData.h"
#include "DataAsset/UK_StatusAnimData.h"

UUK_StatusAnimData* UUK_WeaponData::FindAnimsDataAssetByTag(const FGameplayTag& WeaponTag)
{
    if (const TObjectPtr <UUK_StatusAnimData>* FoundAction = WeaponAnimationDatas.Find(WeaponTag))
    {
        return *FoundAction;
    }
    return nullptr;
}
