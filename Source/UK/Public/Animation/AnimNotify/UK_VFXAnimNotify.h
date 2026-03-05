#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UK_VFXAnimNotify.generated.h"

UCLASS()
class UK_API UUK_VFXAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	class UNiagaraSystem* NiagaraVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector VFXScale = FVector(1.0f);

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};