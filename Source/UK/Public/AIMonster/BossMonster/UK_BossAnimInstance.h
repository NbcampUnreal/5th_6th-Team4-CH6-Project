#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UK_BossAnimInstance.generated.h"

class AUK_BossMonsterBase;

UCLASS()
class UK_API UUK_BossAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category="Anim")
	float Speed;

	UPROPERTY(BlueprintReadWrite, Category="Anim")
	bool bIsAttacking;
	
	UFUNCTION()
	void AnimNotify_StartAttack();

	UFUNCTION()
	void AnimNotify_EndAttack();
	
	virtual void NativeInitializeAnimation() override;
private:
	UPROPERTY()
	AUK_BossMonsterBase* OwnerBoss = nullptr;
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
