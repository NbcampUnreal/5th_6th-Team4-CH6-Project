// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Weapon/UK_WeaponBase.h"
#include "Character/UK_CharacterBase.h"

// Sets default values
AUK_WeaponBase::AUK_WeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
}

// Called when the game starts or when spawned
void AUK_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (!OwnerClass)
	{
		StaticMeshComponent->SetVisibility(false);
	}
}

// Called every frame
//void AUK_WeaponBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

