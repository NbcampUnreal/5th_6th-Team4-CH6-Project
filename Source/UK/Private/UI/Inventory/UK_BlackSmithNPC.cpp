// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UK_BlackSmithNPC.h"

#include "UI/InGame/UK_CheckPoint.h"

// Sets default values
AUK_BlackSmithNPC::AUK_BlackSmithNPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	MarkerComp = CreateDefaultSubobject<UChildActorComponent>("MarkerComp");
	MarkerComp->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AUK_BlackSmithNPC::BeginPlay()
{
	Super::BeginPlay();
	
	if (SelectMarker)
	{
		MarkerComp->SetChildActorClass(SelectMarker);
		MarkerComp->CreateChildActor();
        
		NPCMarker = Cast<AUK_CheckPoint>(MarkerComp->GetChildActor());
        
		if (NPCMarker)
		{
			FVector CheckLocation = GetActorLocation() + FVector(0.0f, 0.0f, 150.0f);
			NPCMarker->SetActorLocation(CheckLocation); 
		}
	}
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

