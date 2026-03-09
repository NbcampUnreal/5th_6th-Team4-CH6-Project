#include "AIMonster/BossMonster/AnimNotify/UK_AnimNotifyState_BossAttack.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"

void UUK_AnimNotifyState_BossAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (auto* Boss = Cast<AUK_BossMonsterBase>(MeshComp->GetOwner()))
		{
			Boss->StartAttack();
		}
	}
}

void UUK_AnimNotifyState_BossAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (auto* Boss = Cast<AUK_BossMonsterBase>(MeshComp->GetOwner()))
		{
			Boss->EndAttack();
		}
	}
}