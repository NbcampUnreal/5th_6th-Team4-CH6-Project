// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UK_BlackSmithNPC.h"

// Sets default values
AUK_BlackSmithNPC::AUK_BlackSmithNPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUK_BlackSmithNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUK_BlackSmithNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUK_BlackSmithNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

