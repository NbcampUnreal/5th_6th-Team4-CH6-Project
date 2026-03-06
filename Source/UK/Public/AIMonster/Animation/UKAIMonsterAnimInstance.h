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

#pragma region Initialization
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
#pragma endregion

#pragma region Animation Properties
	/** 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Speed = 0.f;

	/** 사망 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsDead = false;

	/** 공격 몽타주 재생 중 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsAttacking = false;

	/** 적대 상태 여부 (평화 몬스터용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsAggressive = false;
#pragma endregion

#pragma region Movement Properties
	// BS2D Forward, Right Movespeed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RightSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Direction = 0.f;      

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float LeanAngle = 0.f;      

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsChasing = false;  
	
private:
	float PrevYaw = 0.f;
#pragma endregion

#pragma region Private
private:
	UPROPERTY()
	class AAIMonsterBase* OwnerMonster = nullptr;
#pragma endregion
};