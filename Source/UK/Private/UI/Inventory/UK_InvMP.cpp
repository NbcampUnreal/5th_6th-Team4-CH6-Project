#include "UI/Inventory/UK_InvMP.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"

void UUK_InvMP::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerPawn = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (!PlayerPawn) return;

	ASC = PlayerPawn->GetAbilitySystemComponent();
	if (!ASC) return;

	InitStatusUI();

	MaxMpChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxMpAttribute())
		.AddUObject(this, &UUK_InvMP::UpdateMpBar);

	MpChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute())
		.AddUObject(this, &UUK_InvMP::UpdateMpBar);

}

void UUK_InvMP::NativeDestruct()
{
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxMpAttribute())
			.Remove(MaxMpChangedHandle);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute())
			.Remove(MpChangedHandle);
	}

	Super::NativeDestruct();
}

void UUK_InvMP::InitStatusUI()
{
	if (!ASC) return;

	const float MaxMp = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetMaxMpAttribute());

	const float CurrentMp = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute());

	if (MPBar)
	{
		MPBar->SetPercent(MaxMp > 0.f ? CurrentMp / MaxMp : 0.f);
	}
	if (CurrentMPText)
	{
		CurrentMPText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentMp)));
	}
	if (MaxMPText)
	{
		MaxMPText->SetText(FText::AsNumber(FMath::FloorToInt(MaxMp)));
	}
}

void UUK_InvMP::UpdateMpBar(const FOnAttributeChangeData& Data)
{
	if (!ASC) return;

	const float MaxMp = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetMaxMpAttribute());

	const float CurrentMp = ASC->GetNumericAttribute(
		UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute());

	if (MPBar)
	{
		MPBar->SetPercent(MaxMp > 0.f ? CurrentMp / MaxMp : 0.f);
	}

	if (CurrentMPText)
	{
		CurrentMPText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentMp)));
	}

	if (MaxMPText)
	{
		MaxMPText->SetText(FText::AsNumber(FMath::FloorToInt(MaxMp)));
	}
}
