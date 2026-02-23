#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UUKAIMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerMonster = Cast<AAIMonsterBase>(TryGetPawnOwner());
}

void UUKAIMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!OwnerMonster) return;

	Speed = OwnerMonster->GetVelocity().Size2D();
	bIsDead = OwnerMonster->IsDead();
	bIsAttacking = OwnerMonster->bIsAttacking;
	bIsAggressive = OwnerMonster->GetIsAggressive();
	
	const FVector ForwardVector = OwnerMonster->GetActorForwardVector();
	const FVector RightVector = OwnerMonster->GetActorRightVector();
	
	ForwardSpeed = FVector::DotProduct(ForwardVector, OwnerMonster->GetVelocity());
	RightSpeed   = FVector::DotProduct(RightVector, OwnerMonster->GetVelocity());
}