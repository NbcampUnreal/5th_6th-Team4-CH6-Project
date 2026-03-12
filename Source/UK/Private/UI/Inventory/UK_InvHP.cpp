#include "UI/Inventory/UK_InvHP.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

void UUK_InvHP::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerPawn = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (!PlayerPawn) return;

	ASC = PlayerPawn->GetAbilitySystemComponent();
	if (!ASC) return;

	InitStatusUI();

	MaxHealthChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UUK_InvHP::UpdateHealthBar);

	HealthChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UUK_InvHP::UpdateHealthBar);

}

void UUK_InvHP::NativeDestruct()
{
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute())
			.Remove(MaxHealthChangedHandle);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);
	}

	Super::NativeDestruct();
}

void UUK_InvHP::InitStatusUI()
{
	if (!ASC) return;

	const float MaxHealth = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());

	const float CurrentHealth = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetHealthAttribute());

	if (HPBar)
	{
		HPBar->SetPercent(MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f);
	}
	if (CurrentHPText)
	{
		CurrentHPText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentHealth)));
	}
	if (MaxHPText)
	{
		MaxHPText->SetText(FText::AsNumber(FMath::FloorToInt(MaxHealth)));
	}
}

void UUK_InvHP::UpdateHealthBar(const FOnAttributeChangeData& Data)
{
	if (!ASC) return;

	const float MaxHealth = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());

	const float CurrentHealth = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetHealthAttribute());

	if (HPBar)
	{
		HPBar->SetPercent(MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f);
	}

	if (CurrentHPText)
	{
		CurrentHPText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentHealth)));
	}

	if (MaxHPText)
	{
		MaxHPText->SetText(FText::AsNumber(FMath::FloorToInt(MaxHealth)));
	}
}