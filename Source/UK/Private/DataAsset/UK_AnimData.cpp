// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/UK_AnimData.h"

UAnimMontage* UUK_AnimData::FindMontageByAttribute(ECharacterAttribute Attribute)
{
	if (UAnimMontage* FoundMontage = *ComboMontageMap.Find(Attribute))
	{
		return FoundMontage;
	}
	return nullptr;
}
