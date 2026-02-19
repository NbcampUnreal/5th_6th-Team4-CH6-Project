// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_ClimbingComponent.h"

// Sets default values for this component's properties
UUK_ClimbingComponent::UUK_ClimbingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

// Called when the game starts
void UUK_ClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UUK_ClimbingComponent::SweepAndHits()
{
}

// Called every frame
//void UUK_ClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//}

