#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "UK_InvHP.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
class AUK_CharacterBase;

UCLASS()
class UK_API UUK_InvHP : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitStatusUI();

	void UpdateHealthBar(const FOnAttributeChangeData& Data);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentHPText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxHPText;

	UPROPERTY()
	AUK_CharacterBase* PlayerPawn;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle HealthChangedHandle;
};