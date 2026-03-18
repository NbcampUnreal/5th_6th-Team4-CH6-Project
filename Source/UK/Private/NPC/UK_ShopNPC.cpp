#include "NPC/UK_ShopNPC.h"

AUK_ShopNPC::AUK_ShopNPC()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUK_ShopNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUK_ShopNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUK_ShopNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

