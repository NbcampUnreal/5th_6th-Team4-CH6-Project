#include "AIMonster/Monster/UK_BurrowMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/UK_AiMonsterCtl.h"

#pragma region Initialization
AUK_BurrowMonster::AUK_BurrowMonster()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate              = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed              = 350.0f;

	AIControllerClass = AUK_AiMonsterCtl::StaticClass();
	AutoPossessAI     = EAutoPossessAI::PlacedInWorldOrSpawned;

	DetectionRadius  = 800.0f;
	MaxChaseDistance = 1800.0f;
	AttackRange      = 250.0f;
	AttackCooldown   = 1.5f;

	PatrolRadius = 0.0f;
}

void AUK_BurrowMonster::BeginPlay()
{
	Super::BeginPlay();

	SetBurrowed(true);
}
#pragma endregion

#pragma region Burrow
void AUK_BurrowMonster::SetBurrowed(bool bBurrow)
{
	bIsBurrowed = bBurrow;

	if (UCharacterMovementComponent* MC = GetCharacterMovement())
	{
		if (bBurrow)
		{
			MC->StopMovementImmediately();
			MC->DisableMovement(); 
		}
		else
		{
			MC->SetMovementMode(MOVE_Walking);
		}
	}
}
#pragma endregion