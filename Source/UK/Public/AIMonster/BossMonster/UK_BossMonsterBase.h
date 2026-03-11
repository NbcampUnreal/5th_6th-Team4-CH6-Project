#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameplayTagContainer.h"
#include "UK_BossMonsterBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FBossPhaseChanged,const FGameplayTag&);
class UUK_BossAnimInstance;
class UGameplayEffect;
class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, const FGameplayTag&, NewPhaseTag);

UCLASS()
class UK_API AUK_BossMonsterBase : public AAIMonsterBase
{
	GENERATED_BODY()

public:
	AUK_BossMonsterBase();
	
protected:
	virtual void BeginPlay() override;
	
	AActor* GetTargetActor() const;
	
	float PlayMontage(UAnimMontage* Montage, float InPlayRate = 1.f);
	
#pragma region Phase & Combat
public:
	virtual void ReceiveDamage(float Damage) override;
	
	virtual bool PlayRandomAttackMontage() override;

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void SetWeaponCollisionEnabled(bool bEnabled);

	void StartAttack();
	void EndAttack();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Phase")
	FGameplayTag CurrentPhaseTag;
	
	virtual void UpdatePhase();
	
	int32 CurrentPhase = 1;
	
	//페이즈 변경 알림 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
	FOnBossPhaseChanged OnBossPhaseChanged;
	
	FGameplayTag GetCurrentPhase() const { return CurrentPhaseTag; }
	
	virtual void NotifyAttacked(AController* InstigatorController) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Boss|Patterns")
	TArray<UAnimMontage*> Phase1Patterns;

	UPROPERTY(EditAnywhere, Category = "Boss|Patterns")
	TArray<UAnimMontage*> Phase2Patterns;
	
	UPROPERTY(EditAnywhere, Category = "Design | Animation")
	UAnimMontage* HitReactMontage;
	
	float LastHitReactTime = 0.f; //마지막으로 받은 피격 애님 플레이 시간 체크용
	
	UPROPERTY(VisibleAnywhere, Category = "Boss|Combat")
	UCapsuleComponent* WeaponCollision_R;
	
	UPROPERTY(VisibleAnywhere, Category = "Boss|Combat")
	UCapsuleComponent* WeaponCollision_L;
	
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY()
	TArray<AActor*> HitActors;
#pragma endregion
};
