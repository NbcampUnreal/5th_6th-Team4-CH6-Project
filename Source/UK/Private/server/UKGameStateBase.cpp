// Fill out your copyright notice in the Description page of Project Settings.

#include "Server/UKGameStateBase.h"
#include "Net/UnrealNetwork.h"

AUKGameStateBase::AUKGameStateBase()
{
	// 필요하면 설정
}

void AUKGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUKGameStateBase, bIsWorldPublic);
}