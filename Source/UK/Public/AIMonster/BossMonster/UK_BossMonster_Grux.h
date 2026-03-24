#pragma once

#include "CoreMinimal.h"
#include "AIMonster/BossMonster/UK_BossMonsterBase.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "UK_BossMonster_Grux.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UDecalComponent;

UCLASS()
class UK_API AUK_BossMonster_Grux : public AUK_BossMonsterBase
{
	GENERATED_BODY()
	
public:
	AUK_BossMonster_Grux();
	virtual void UpdatePhase() override;
protected:
	virtual void BeginPlay() override;
	virtual bool PlayRandomAttackMontage() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux")
	float BaseAttackDamage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* SmashAttack;    // 광역 내려찍기 (페이즈 2)

	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* JumpAttack; // 점프 공격 (페이즈 2)
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* DashAttack; // 점프 공격 (페이즈 2)
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* RangedAttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Projectile")
	TSubclassOf<AActor> SwordWaveClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Projectile")
	FName MuzzleSocketName = TEXT("Muzzle_Front");
	
	UFUNCTION(BlueprintCallable, Category = "Grux|Patterns")
	void LaunchSwordWave();
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* Berserk;
	float BerserkPlayRate = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Combat | Effects")
	TObjectPtr<UNiagaraSystem> BerserkLoopEffect;
	
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BerserkComponent;
	
	UPROPERTY(EditAnywhere, Category = "Combat | Effects")
	FName BerserkSocketName = TEXT("SmashPoint");
	
	UFUNCTION(BlueprintCallable, Category = "Grux|Patterns")
	void ExecuteJumpSmashDamage();
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	float SmashRadius = 600.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Boss | Combat")
	UDecalComponent* JumpTargetDecal;
	
	UPROPERTY(EditAnywhere, Category = "Boss | Combat")
	float JumpTargetRadius = 400.f; // 데칼 크기
	
private:
	void ApplyBerserkBuff();
	void OnDashFinished(UAnimMontage* Montage, bool bInterrupted);
	
	bool ExecuteJumpAttackAction(float PlayRate);
	bool ExecuteDashAttackAction(float PlayRate);
	bool ExecuteRangedAttackAction(float PlayRate);
	bool PlayBaseAttackWithSpeed(float PlayRate);
	
	FTimerHandle DashTimerHandle;
	FTimerHandle StopDashTimerHandle;
	FTimerHandle RotationTimerHandle;
	
	FVector DashDirection;
	
	void ExecuteDashMove(); //대쉬 공격 루프함수
	void StopDashMovement(); //대쉬만 할수있게 가속중지 함수
	void LookAtTargetSmooth(); //회전 보간 함수
	
public:
	virtual void ResetForReturn() override;
};
