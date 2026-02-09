#include "NPC/UK_NPCAIBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NPC/UK_NPCAICtl.h"
#include "DrawDebugHelpers.h"

AUK_NPCAIBase::AUK_NPCAIBase()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AUK_NPCAIBase::Interact_Implementation(AActor* Interactor)
{
}

void AUK_NPCAIBase::BeginPlay()
{
	Super::BeginPlay();

	//if ( HasAuthority() )
	//{
	UCharacterMovementComponent* Move = GetCharacterMovement();

	if ( Move )
	{
		Move->bUseRVOAvoidance = true;

		Move->AvoidanceConsiderationRadius = 50.f;
		Move->AvoidanceWeight = 0.5f;

		Move->SetAvoidanceGroup(1);
		Move->SetGroupsToAvoid(1);
		Move->SetGroupsToIgnore(0);
	}
	//}
}

void AUK_NPCAIBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AAIController* AICon = Cast<AAIController>(GetController());
	if ( AICon )
	{
		AUK_NPCAICtl* MyAI = Cast<AUK_NPCAICtl>(AICon);
		if ( MyAI )
		{
			MyAI->DrawSightDebug();
		}
	}
}