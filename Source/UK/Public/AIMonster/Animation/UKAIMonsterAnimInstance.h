#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UKAIMonsterAnimInstance.generated.h"

/**
 * 몬스터 ABP용 C++ 베이스 클래스
 */
UCLASS()
class UK_API UUKAIMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float Speed = 0.f;

	/** 사망 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;

	/** 공격 몽타주 재생 중 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsAttacking = false;

	/** 적대 상태 여부 (평화 몬스터용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsAggressive = false;
	
	// BS2D Forward, Right Movespeed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float ForwardSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RightSpeed;

private:
	UPROPERTY()
	class AAIMonsterBase* OwnerMonster = nullptr;
};