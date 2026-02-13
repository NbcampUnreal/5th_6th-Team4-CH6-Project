// Fill out your copyright notice in the Description page of Project Settings.


#include "server/UKGameModeBase_Title.h"
#include "GameFramework/GameStateBase.h"

AUKGameModeBase_Title::AUKGameModeBase_Title()
{
	bUseSeamlessTravel = true;
}

void AUKGameModeBase_Title::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//초기 설정, 이름 등..

}

void AUKGameModeBase_Title::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);

	//맵이 바뀌어도 사라지지않아야 하는 액터들
}