#include "UI/InGame/UK_MonsterHealthBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

/*
 *void UUK_MonsterHealthBar::UpdateHPBar(float NewHP) 
{ 
	 if ( MonsterHPBar && TargetStatComp && NewHP > 0.f ) 
	{ 
		float TargetPercent = NewHP / TargetStatComp->GetMaxHP(); 
		if ( MonsterHPBar->GetPercent() != TargetPercent ) 
		{
			MonsterHPBar->SetPercent(TargetPercent); } 
	}
}

void UUK_MonsterHealthBar::BindMonsterStats(UAI_MonsterStatComponent* StatComp) 
{ 
	// 에러가 나던 AddDynamic 라인을 삭제하고 컴포넌트 주소만 저장 
	if ( StatComp ) 
	{ 
		TargetStatComp = StatComp; 
		TargetStatComp->OnHPChanged.AddDynamic(this, &UUK_MonsterHealthBar::UpdateHPBar); 
		UpdateHPBar(TargetStatComp->GetHP());
	} 
}
*/
//void UUK_MonsterHealthBar::SetHPBarActive(bool bActive) {
//	if ( bActive ) 
//	{ 
//		// 1. UI를 보이게 설정 
//		SetVisibility(ESlateVisibility::SelfHitTestInvisible); 
//		// 2. 타이머 시작 (0.1초 간격으로 반복) 
//		if ( GetWorld() ) 
//		{
//			GetWorld()->GetTimerManager().SetTimer( HPUpdateTimerHandle, this, &UUK_MonsterHealthBar::UpdateHPBar, 0.1f, true ); 
//		} 
//	} 
//	else 
//	{ 
//		SetVisibility(ESlateVisibility::SelfHitTestInvisible); 
//		// 1. UI를 완전히 숨김 (렌더링 안 함) 
//		// //SetVisibility(ESlateVisibility::Collapsed); 
//		// 2. 타이머 중지 (연산 안 함) 
//		if ( GetWorld() ) 
//		{ 
//			GetWorld()->GetTimerManager().ClearTimer(HPUpdateTimerHandle); 
//		} 
//	} 
//}

void UUK_MonsterHealthBar::BindMonsterAttributes(UAbilitySystemComponent* ASC, UUK_MonsterAttributeSet* Attributes)
{
	if (!ASC || !Attributes) return;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			Attributes->GetHealthAttribute()
		).RemoveAll(this);  
	}
	
	AbilitySystemComponent = ASC;
	AttributeSet = Attributes;

	// Health 변경 콜백 등록
	ASC->GetGameplayAttributeValueChangeDelegate(
		Attributes->GetHealthAttribute()
	).AddUObject(this, &UUK_MonsterHealthBar::OnHealthChanged);

	// 초기 값으로 UI 업데이트
	UpdateHealthDisplay();
}

void UUK_MonsterHealthBar::SetMonsterName(const FText& InName)
{
	if (MonsterNameText)
	{
		MonsterNameText->SetText(InName);
	}
}

void UUK_MonsterHealthBar::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue < Data.OldValue)
	{
		UpdateHealthDisplay();
	}
}

void UUK_MonsterHealthBar::UpdateHealthDisplay()
{
	if (!AttributeSet || !MonsterHPBar) return;

	const float CurrentHealth = AttributeSet->GetHealth();
	const float MaxHealth = AttributeSet->GetMaxHealth();
	const float HealthPercent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;

	if (MonsterHPBar->GetPercent() != HealthPercent)
	{
		MonsterHPBar->SetPercent(HealthPercent);
	}
}

void UUK_MonsterHealthBar::SetHPBarActive(bool bActive)
{
	if (bActive)
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}