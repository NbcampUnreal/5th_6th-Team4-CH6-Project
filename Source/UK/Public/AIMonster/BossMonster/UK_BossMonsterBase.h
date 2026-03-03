#pragma once

#include "CoreMinimal.h"
#include "AIMonster/AIMonsterBase.h"
#include "AIMonster/BossMonster/UK_BossTypes.h"
#include "UK_BossMonsterBase.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1,
	Phase2,
	Phase3, 
	Enrage
};

UCLASS()
class UK_API AUK_BossMonsterBase : public AAIMonsterBase
{
	GENERATED_BODY()

public:
	AUK_BossMonsterBase();

	bool PlayPhasePattern();

	UFUNCTION(BlueprintCallable)
	EBossPhase GetCurrentPhase() const
	{
		return CurrentPhase;
	}
protected:
	virtual void BeginPlay() override;

	/* ================= Phase ================= */

	UPROPERTY(ReplicatedUsing = OnRep_BossPhase, BlueprintReadOnly, Category = "Boss|Phase")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase2HPRatio = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float Phase3HPRatio = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
	float EnrageHPRatio = 0.15f;

	UFUNCTION()
	void OnRep_BossPhase();

	void UpdatePhase();

	void SetPhase(EBossPhase NewPhase);


	virtual void ShowHPBar() override;

protected:
	virtual void ReceiveDamage(float Damage) override;

	virtual bool PlayRandomAttackMontage() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase1Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase2Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> Phase3Patterns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TArray<UAnimMontage*> EnragePatterns;;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
