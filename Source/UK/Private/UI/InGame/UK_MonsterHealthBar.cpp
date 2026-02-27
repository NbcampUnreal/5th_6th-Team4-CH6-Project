#include "UI/InGame/UK_MonsterHealthBar.h"
#include "Components/ProgressBar.h"
#include "AIMonster/Component/AI_MonsterStatComponent.h"


void UUK_MonsterHealthBar::UpdateHPBar(float NewHP) 
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