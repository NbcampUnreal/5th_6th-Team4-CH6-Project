#include "AIMonster/BossMonster/UK_BossAnimInstance.h"

#include "GameFramework/Character.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

void UUK_BossAnimInstance::AnimNotify_StartAttack()
{
	if (OwnerBoss) OwnerBoss->StartAttack();
}

void UUK_BossAnimInstance::AnimNotify_EndAttack()
{
	if (OwnerBoss) OwnerBoss->EndAttack();
}

void UUK_BossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerBoss = Cast<AUK_BossMonsterBase>(TryGetPawnOwner());
}

void UUK_BossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!OwnerBoss) return;

	Speed = OwnerBoss->GetVelocity().Size();
}
