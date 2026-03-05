#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "GameplayTagContainer.h"
#include "UK_BossMonsterBase.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FBossPhaseChanged,const FGameplayTag&);

UCLASS()
class UK_API AUK_BossMonsterBase : public AAIMonsterBase
{
	GENERATED_BODY()

public:
	AUK_BossMonsterBase();
	
	FBossPhaseChanged OnBossPhaseChanged;
	
	UFUNCTION(BlueprintCallable, Category="Boss|Phase")
	FGameplayTag GetCurrentPhase() const { return CurrentPhaseTag; }
	
	virtual bool PlayRandomAttackMontage() override;
protected:
	virtual void BeginPlay() override;

	/* ================= Phase ================= */
	UPROPERTY(ReplicatedUsing=OnRep_Phase, BlueprintReadOnly, Category="Boss|Phase")
	FGameplayTag CurrentPhaseTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase2HPRatio = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase3HPRatio = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float EnrageHPRatio = 0.15f;

	UFUNCTION()
	void OnRep_Phase();

	void UpdatePhase();

	void SetPhase(const FGameplayTag& NewPhase);
	
	virtual void ShowHPBar() override;

protected:
	void ReceiveDamage(float Damage);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase1Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase2Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase3Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> EnragePatterns;;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	FGameplayTag Phase1Tag;
	FGameplayTag Phase2Tag;
	FGameplayTag Phase3Tag;
	FGameplayTag EnrageTag;
};
