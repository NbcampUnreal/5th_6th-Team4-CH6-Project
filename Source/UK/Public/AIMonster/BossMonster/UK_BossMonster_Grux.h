#pragma once

#include "CoreMinimal.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "UK_BossMonster_Grux.generated.h"

UCLASS()
class UK_API AUK_BossMonster_Grux : public AUK_BossMonsterBase
{
	GENERATED_BODY()
	
public:
	AUK_BossMonster_Grux();

protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
	virtual void UpdatePhase() override;
	virtual void ReceiveDamage(float Damage) override;
	virtual bool PlayRandomAttackMontage() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* SmashAttack;    // 광역 내려찍기 (페이즈 2)

	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* JumpAttack; // 점프 공격 (페이즈 2)
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* DashAttack; // 점프 공격 (페이즈 2)
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* Berserk;
	float BerserkPlayRate = 1.0f;
	
	
	UFUNCTION(BlueprintCallable, Category = "Grux|Patterns")
	void ExecuteJumpSmashDamage();
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	float SmashRadius = 600.f;
private:
	void ApplyBerserkBuff();
	void OnDashFinished(UAnimMontage* Montage, bool bInterrupted);
	
	bool ExecuteJumpAttackAction(float PlayRate);
	bool ExecuteDashAttackAction(float PlayRate);
	bool PlayBaseAttackWithSpeed(float PlayRate);
	
	FTimerHandle DashTimerHandle;
	FTimerHandle StopDashTimerHandle;
	FTimerHandle RotationTimerHandle;
	
	FVector DashDirection;
	
	void ExecuteDashMove(); //대쉬 공격 루프함수
	void StopDashMovement(); //대쉬만 할수있게 가속중지 함수
	void LookAtTargetSmooth(); //회전 보간 함수
};
