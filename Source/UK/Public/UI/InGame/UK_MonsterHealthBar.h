#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIMonster/AttibuteSet/UK_MonsterAttributeSet.h"
#include "UK_MonsterHealthBar.generated.h"

// 전방 선언
//class UAI_MonsterStatComponent;
class UProgressBar;
class UTextBlock;

UCLASS()
class UK_API UUK_MonsterHealthBar : public UUserWidget
{
	GENERATED_BODY()

protected:

	//UFUNCTION()
	//void UpdateHPBar(float NewHP);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* MonsterHPBar;
	
	// 현재 감시 중인 스탯 컴포넌트 저장용
	//UPROPERTY()
	//UAI_MonsterStatComponent* TargetStatComp;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MonsterNameText;

	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UUK_MonsterAttributeSet* AttributeSet;
	
	FTimerHandle HPUpdateTimerHandle;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	void UpdateHealthDisplay();

public:
	//UFUNCTION(BlueprintCallable, Category = "UI")
	//void BindMonsterStats(UAI_MonsterStatComponent* StatComp);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void BindMonsterAttributes(UAbilitySystemComponent* ASC, UUK_MonsterAttributeSet* Attributes);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetMonsterName(const FText& InName);
	
	void SetHPBarActive(bool bActive);
};