#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h" // 태그 사용을 위해 추가
#include "UK_VFXAnimNotify.generated.h"

UCLASS()
class UK_API UUK_VFXAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 에디터의 노티파이 타임라인에서 어떤 VFX 쓸지 태그로 지정함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag VFXEventTag;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};