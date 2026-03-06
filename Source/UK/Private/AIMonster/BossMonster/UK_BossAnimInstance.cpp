#include "AIMonster/BossMonster/UK_BossAnimInstance.h"
#include "GameFramework/Character.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

void UUK_BossAnimInstance::AnimNotify_StartAttack()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(Pawn);
	if (!Boss) return;

	Boss->StartAttack();
}

void UUK_BossAnimInstance::AnimNotify_EndAttack()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	AUK_BossMonsterBase* Boss = Cast<AUK_BossMonsterBase>(Pawn);
	if (!Boss) return;

	Boss->EndAttack();
}

void UUK_BossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	Speed = Pawn->GetVelocity().Size();
}
