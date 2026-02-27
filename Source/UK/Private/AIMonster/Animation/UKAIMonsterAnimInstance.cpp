#include "AIMonster/Animation/UKAIMonsterAnimInstance.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma region Initialization
void UUKAIMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerMonster = Cast<AAIMonsterBase>(TryGetPawnOwner());
}
#pragma endregion

#pragma region Animation Update
void UUKAIMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!OwnerMonster) return;

	Speed        = OwnerMonster->GetVelocity().Size2D();
	bIsDead      = OwnerMonster->IsDead();
	bIsAttacking = OwnerMonster->bIsAttacking;
	bIsAggressive = OwnerMonster->GetIsAggressive();

	const FVector ForwardVector = OwnerMonster->GetActorForwardVector();
	const FVector RightVector   = OwnerMonster->GetActorRightVector();
	const FVector Velocity      = OwnerMonster->GetVelocity();

	ForwardSpeed = FVector::DotProduct(ForwardVector, Velocity);
	RightSpeed   = FVector::DotProduct(RightVector,   Velocity);
}
#pragma endregion