#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "UK_InvMP.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
class AUK_CharacterBase;

UCLASS()
class UK_API UUK_InvMP : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitStatusUI();

	void UpdateMpBar(const FOnAttributeChangeData& Data);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentMPText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxMPText;

	UPROPERTY()
	AUK_CharacterBase* PlayerPawn;

	UPROPERTY()
	UAbilitySystemComponent* ASC;

	FDelegateHandle MaxMpChangedHandle;
	FDelegateHandle MpChangedHandle;

	
};
