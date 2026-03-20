#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UK_InvSystem.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class UK_API UUK_InvSystem : public UUserWidget
{
	GENERATED_BODY()

public:
	//에디터에서 수정 가능한 설정값들
	UPROPERTY(EditAnywhere, Category = "StatUIFont")
	int32 NormalFontSize = 18;

	UPROPERTY(EditAnywhere, Category = "StatUIFont")
	int32 ChangedFontSize = 20;

	UPROPERTY(EditAnywhere, Category = "StatUIColor")
	FLinearColor IncreaseColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "StatUIColor")
	FLinearColor DecreaseColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, Category = "StatUIColor")
	FLinearColor NormalColor = FLinearColor::White;

public:

	virtual void NativeConstruct() override;

	//각 스탯 변경 시 UI 텍스트를 갱신할 함수들
	UFUNCTION() 
	void UpdateHealth(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateMaxHealth(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateAttackPower(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateCurrentMp(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateMaxMp(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateDefence(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateCriticalChance(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateCriticalDamage(float OldValue, float NewValue);

	UFUNCTION() 
	void UpdateMaxStamina(float OldValue, float NewValue);
	UFUNCTION()
	void ProcessStatUpdate(UTextBlock* TargetText, class UImage* ArrowImage, float OldValue, float NewValue);


public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateHealth;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateMaxHealth;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateAttackPower;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateCurrentMp;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateMaxMp;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateDefence;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateCriticalChance;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateCriticalDamage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StateMaxStamina;

	UPROPERTY(meta = (BindWidget))
	UImage* Arrow_Health;
	UPROPERTY(meta = (BindWidget))
	UImage* Arrow_MaxHealth;
	UPROPERTY(meta = (BindWidget)) 
	UImage* Arrow_Attack;
	UPROPERTY(meta = (BindWidget))
	UImage* Arrow_CurrentMp;
	UPROPERTY(meta = (BindWidget)) 
	UImage* Arrow_MaxMp;
	UPROPERTY(meta = (BindWidget))
	UImage* Arrow_Defence;
	UPROPERTY(meta = (BindWidget)) 
	UImage* Arrow_CritChance;
	UPROPERTY(meta = (BindWidget))
	UImage* Arrow_CritDamage;
	UPROPERTY(meta = (BindWidget)) 
	UImage* Arrow_Stamina;
};
