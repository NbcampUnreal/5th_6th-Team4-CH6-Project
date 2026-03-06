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
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* SmashAttack;    // 광역 내려찍기 (페이즈 2)

	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* JumpAttack; // 점프 공격 (페이즈 2)

	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	UAnimMontage* Berserk;      // 광폭화 포효 (페이즈 3 진입용)
	
	virtual bool PlayRandomAttackMontage() override;
	
	UFUNCTION(BlueprintCallable, Category = "Grux|Patterns")
	void ExecuteJumpSmashDamage();
	
	UPROPERTY(EditDefaultsOnly, Category = "Grux|Patterns")
	float SmashRadius = 600.f;
private:
	void ApplyBerserkBuff();
};
