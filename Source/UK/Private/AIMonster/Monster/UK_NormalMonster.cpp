// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMonster/Monster/UK_NormalMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/UK_AiMonsterCtl.h"

AUK_NormalMonster::AUK_NormalMonster()
{
	// Capsule 설정
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Movement 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;

	// AI Controller 설정
	AIControllerClass = AUK_AiMonsterCtl::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// AI 설정 (기본값)
	DetectionRadius = 800.0f;
	PatrolRadius = 1000.0f;
	LookAtRotationSpeed = 5.0f;
	MaxChaseDistance = 1500.0f;
}

void AUK_NormalMonster::BeginPlay()
{
	Super::BeginPlay();
}