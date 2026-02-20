#include "UI/InGame/UK_MonsterHealthBar.h"
#include "Components/ProgressBar.h"
#include "AIMonster/Component/AI_MonsterStatComponent.h"

void UUK_MonsterHealthBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 컴포넌트가 연결되어 있다면 매 프레임 값을 가져와서 UI 갱신
	if ( TargetStatComp )
	{
		UpdateHPBar(TargetStatComp->GetHP(), TargetStatComp->GetMaxHP());
	}
}

void UUK_MonsterHealthBar::UpdateHPBar(float CurrentHP, float MaxHP)
{
	if ( MonsterHPBar && MaxHP > 0.f )
	{
		// 현재 UI의 Percent 값과 목표 값이 다를 때만 갱신 (최적화)
		float TargetPercent = CurrentHP / MaxHP;
		if ( MonsterHPBar->GetPercent() != TargetPercent )
		{
			MonsterHPBar->SetPercent(TargetPercent);
		}
	}
}

void UUK_MonsterHealthBar::BindMonsterStats(UAI_MonsterStatComponent* StatComp)
{
	// 에러가 나던 AddDynamic 라인을 삭제하고 컴포넌트 주소만 저장
	if ( StatComp )
	{
		TargetStatComp = StatComp;

		// 즉시 초기화
		UpdateHPBar(TargetStatComp->GetHP(), TargetStatComp->GetMaxHP());
	}
}