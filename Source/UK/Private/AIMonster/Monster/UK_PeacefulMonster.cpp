#include "AIMonster/Monster/UK_PeacefulMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIMonster/UK_AiMonsterCtl.h"

AUK_PeacefulMonster::AUK_PeacefulMonster()
{
	// Capsule 설정
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// Movement 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);  // 느린 회전
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;  // 느린 이동속도

	// AI Controller 설정
	AIControllerClass = AUK_AiMonsterCtl::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 평화로운 몬스터 설정
	Personality = EMonsterPersonality::Peaceful;
	
	// 경계 거리
	AlertDistance = 300.0f;
	
	// 링크 시스템 반경
	AllyCallRadius = 1200.0f;
	
	// 리셋 거리
	ResetDistance = 2500.0f;
	
	// 탐지 거리 (공격적일 때만 사용)
	DetectionRadius = 1000.0f;
	
	// 최대 추적 거리
	MaxChaseDistance = 2000.0f;
	
	// 공격력 (화나면 세게 때림)
	AttackDamage = 25.0f;
	
	// 공격 범위
	AttackRange = 150.0f;
}

void AUK_PeacefulMonster::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// 평화로운 상태로 시작
		RequestState(EMonsterState::Passive);
		
		UE_LOG(LogTemp, Log, TEXT("[Peaceful Monster] %s spawned in Passive state"), *GetName());
	}
}