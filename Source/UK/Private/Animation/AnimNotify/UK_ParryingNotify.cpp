#include "Animation/AnimNotify/UK_ParryingNotify.h"
#include "Components/SkeletalMeshComponent.h"

void UUK_ParryingNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && OverlayMaterial)
	{
		// 메시 전체 몸에 선택한 머티리얼을 덧입힙니다.
		MeshComp->SetOverlayMaterial(OverlayMaterial);
	}
}

void UUK_ParryingNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// 노티파이 구간이 끝나면 오버레이 머티리얼을 제거합니다.
		MeshComp->SetOverlayMaterial(nullptr);
	}
}