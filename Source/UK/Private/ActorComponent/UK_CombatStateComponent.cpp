// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_CombatStateComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UUK_CombatStateComponent::UUK_CombatStateComponent() 
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


void UUK_CombatStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called when the game starts
void UUK_CombatStateComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
//void UUK_CombatStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//}

