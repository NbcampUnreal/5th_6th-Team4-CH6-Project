// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UK_ShopNPC.h"

// Sets default values
AUK_ShopNPC::AUK_ShopNPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUK_ShopNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUK_ShopNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUK_ShopNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

