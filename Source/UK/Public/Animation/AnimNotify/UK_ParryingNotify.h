#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UK_ParryingNotify.generated.h"

UCLASS()
class UK_API UUK_ParryingNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// 몽타주 노티파이 상세창에서 직접 'Fresnel_Mat'를 선택할 수 있게 합니다.
	UPROPERTY(EditAnywhere, Category = "Parry Effect")
	class UMaterialInterface* OverlayMaterial;
};