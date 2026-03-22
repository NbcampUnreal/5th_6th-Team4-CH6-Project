#include "UI/InGame/UK_XPbar.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/UK_CharacterBase.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

void UUK_XPbar::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerPawn = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (!PlayerPawn) return;
	
	ASC = PlayerPawn->GetAbilitySystemComponent();

	if (!ASC) return;
	
	InitXPBar();

	EXPChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetEXPAttribute())
		.AddUObject(this, &UUK_XPbar::UpdateXPBar);

	MaxEXPChangedHandle =
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxEXPAttribute())
		.AddUObject(this, &UUK_XPbar::UpdateXPBar);
}

void UUK_XPbar::NativeDestruct()
{
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetEXPAttribute())
			.Remove(EXPChangedHandle);

		ASC->GetGameplayAttributeValueChangeDelegate(
			UUK_PlayerStatusAttributeSet::GetMaxEXPAttribute())
			.Remove(MaxEXPChangedHandle);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(XPInterpTimerHandle);
	}

	Super::NativeDestruct();
}

void UUK_XPbar::InitXPBar()
{
	if (!ASC) return;
	

	const float CurrentEXP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetEXPAttribute());
	const float MaxEXP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxEXPAttribute());
	const float SafeMaxEXP = FMath::Max(MaxEXP, 1.f);

	CurrentXPPercent = CurrentEXP / SafeMaxEXP;
	TargetXPPercent = CurrentXPPercent;

	if (XPbar)
	{
		XPbar->SetPercent(CurrentXPPercent);
	}

	UpdateXPText(CurrentXPPercent);
}

void UUK_XPbar::UpdateXPBar(const FOnAttributeChangeData& Data)
{
	if (!ASC) return;
	

	const float CurrentEXP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetEXPAttribute());
	const float MaxEXP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxEXPAttribute());
	const float SafeMaxEXP = FMath::Max(MaxEXP, 1.f);

	TargetXPPercent = CurrentEXP / SafeMaxEXP;

	StartInterpTimer();
}

void UUK_XPbar::StartInterpTimer()
{
	if (!GetWorld())
	{
		return;
	}

	if (!GetWorld()->GetTimerManager().IsTimerActive(XPInterpTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			XPInterpTimerHandle,
			this,
			&UUK_XPbar::UpdateXPInterpolation,
			TimerSpeed,
			true
		);
	}
}

void UUK_XPbar::UpdateXPInterpolation()
{
	if (!XPbar || !GetWorld())
	{
		return;
	}

	CurrentXPPercent = FMath::FInterpTo(
		CurrentXPPercent,
		TargetXPPercent,
		TimerSpeed,
		InterpSpeed
	);

	XPbar->SetPercent(CurrentXPPercent);
	UpdateXPText(CurrentXPPercent);

	if (FMath::IsNearlyEqual(CurrentXPPercent, TargetXPPercent, 0.001f))
	{
		CurrentXPPercent = TargetXPPercent;
		XPbar->SetPercent(CurrentXPPercent);
		UpdateXPText(CurrentXPPercent);

		GetWorld()->GetTimerManager().ClearTimer(XPInterpTimerHandle);
	}
}

void UUK_XPbar::UpdateXPText(float Percent)
{
	if (!XPText)
	{
		return;
	}

	const int32 PercentValue = FMath::RoundToInt(Percent * 100.f);
	XPText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
}