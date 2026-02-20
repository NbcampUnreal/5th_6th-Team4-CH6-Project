// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponent/UK_ParkourComponent.h"

// Sets default values for this component's properties
UUK_ParkourComponent::UUK_ParkourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UUK_ParkourComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UUK_ParkourComponent::LineTrace()
{

}



// Called every frame
//void UUK_ParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//	// ...
//}

