// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/UK_StatusAnimData.h"
#include "DataAsset/UK_AnimData.h"

UUK_AnimData* UUK_StatusAnimData::FindAnimsDataAssetByType(const EComboAttackType AttackType)
{
	if (const TObjectPtr <UUK_AnimData>* FoundAction = ComboAnimationDatas.Find(AttackType))
	{
		return *FoundAction;
	}
	return nullptr;
}

