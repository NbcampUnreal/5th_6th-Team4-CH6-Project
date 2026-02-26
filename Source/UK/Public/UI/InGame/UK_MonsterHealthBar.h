#pragma once

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

	UFUNCTION()
	void UpdateHPBar(float NewHP);

	UPROPERTY(meta = ( BindWidget ))
	UProgressBar* MonsterHPBar;

	// 현재 감시 중인 스탯 컴포넌트 저장용
	UPROPERTY()
	UAI_MonsterStatComponent* TargetStatComp;

	FTimerHandle HPUpdateTimerHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void BindMonsterStats(UAI_MonsterStatComponent* StatComp);

	void SetHPBarActive(bool bActive);
};