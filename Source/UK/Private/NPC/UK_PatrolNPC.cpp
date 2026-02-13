#include "NPC/UK_PatrolNPC.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"

AUK_PatrolNPC::AUK_PatrolNPC()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AUK_PatrolNPC::BeginPlay()
{
	Super::BeginPlay();

}

FVector AUK_PatrolNPC::GetNextPatrolPoint()
{
	switch ( PatrolType )
	{
	case EPatrolType::RandomArea:
	{
		UNavigationSystemV1* NavSys =
			UNavigationSystemV1::GetCurrent(GetWorld());

		if ( !NavSys )
			return GetActorLocation();

		FNavLocation Result;

		NavSys->GetRandomReachablePointInRadius(
			GetActorLocation(),
			PatrolRadius,
			Result
		);

		return Result.Location;
	}

	case EPatrolType::FixedRoute:
	{
	}
	}

	return GetActorLocation();
}