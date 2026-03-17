// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/HitMontageDataAsset.h"

UAnimMontage* UHitMontageDataAsset::FindHitMontageByType(EHitReactionType HitType)
{
	const TObjectPtr<UAnimMontage>* HitAnim = HitMontageMap.Find(HitType);

	if (HitAnim && *HitAnim)
	{
		return HitAnim->Get();
	}

	return nullptr;
}