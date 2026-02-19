#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MonsterHealthBar.generated.h"

// 전방 선언
class UAI_MonsterStatComponent;
class UProgressBar;

UCLASS()
class UK_API UUK_MonsterHealthBar : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 매 프레임 체력을 감시하기 위해 Tick 추가
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void UpdateHPBar(float CurrentHP, float MaxHP);

	UPROPERTY(meta = ( BindWidget ))
	UProgressBar* MonsterHPBar;

	// 현재 감시 중인 스탯 컴포넌트 저장용
	UPROPERTY()
	UAI_MonsterStatComponent* TargetStatComp;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BindMonsterStats(UAI_MonsterStatComponent* StatComp);
};