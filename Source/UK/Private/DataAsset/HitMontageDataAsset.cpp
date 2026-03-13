// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/HitMontageDataAsset.h"

UAnimMontage* UHitMontageDataAsset::FindHitMontageByType(EHitReactionType HitType)
{
	UAnimMontage* HitAnim = *HitMontageMap.Find(HitType);
	return HitAnim;
}
