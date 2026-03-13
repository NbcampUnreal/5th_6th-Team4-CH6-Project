// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/HitMontageDataAsset.h"

UAnimMontage* UHitMontageDataAsset::FindHitMontageByType(EHitType TargetType)
{
	UAnimMontage* HitAnim = *HitMontageMap.Find(TargetType);
	return HitAnim;
}
