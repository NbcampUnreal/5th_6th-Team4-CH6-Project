#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "UK_XPbar.generated.h"

class UProgressBar;
class UTextBlock;
class AUK_CharacterBase;

UCLASS()
class UK_API UUK_XPbar : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


	void UpdateXPBar(const FOnAttributeChangeData& Data);
	void InitXPBar();

	UPROPERTY(meta = ( BindWidget ))
	UProgressBar* XPbar;

	UPROPERTY(meta = ( BindWidget ))
	UTextBlock* XPText;

private:
	UPROPERTY()
	AUK_CharacterBase* PlayerPawn = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* ASC = nullptr;

	FDelegateHandle EXPChangedHandle;
	FDelegateHandle MaxEXPChangedHandle;

	FTimerHandle XPInterpTimerHandle;

	float TargetXPPercent = 0.f;
	float CurrentXPPercent = 0.f;

	const float InterpSpeed = 5.0f;
	const float TimerSpeed = 0.02f;

	void UpdateXPInterpolation();
	void StartInterpTimer();
	void UpdateXPText(float Percent);
};