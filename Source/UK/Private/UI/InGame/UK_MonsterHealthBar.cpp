#include "UI/InGame/UK_MonsterHealthBar.h"
#include "Components/ProgressBar.h"
#include "AIMonster/Component/AI_MonsterStatComponent.h"

void UUK_MonsterHealthBar::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯이 붙어 있는 '주인 몬스터'를 가져옴
	AActor* Owner = GetOwningPlayerPawn(); // 로컬 플레이어 기준 (Screen Space용)

	// WidgetComponent의 Owner를 직접 찾고 싶다면
	AActor* Monster = GetOuter()->GetTypedOuter<AActor>();
}

void UUK_MonsterHealthBar::UpdateHPBar(float CurrentHP, float MaxHP)
{
	if ( MonsterHPBar && MaxHP > 0.f )
	{
		MonsterHPBar->SetPercent(CurrentHP / MaxHP);
	}
}

void UUK_MonsterHealthBar::BindMonsterStats(UAI_MonsterStatComponent* StatComp)
{
	if ( StatComp )
	{
		StatComp->OnHPChanged.AddDynamic(this, &UUK_MonsterHealthBar::UpdateHPBar);
		// 연결되자마자 현재 체력으로 초기화
		UpdateHPBar(StatComp->GetHP(), StatComp->GetMaxHP());
	}
}