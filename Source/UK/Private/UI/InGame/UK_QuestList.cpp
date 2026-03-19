// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/UK_QuestList.h"
#include "Components/Button.h"

void UUK_QuestList::NativeConstruct()
{
	Super::NativeConstruct();

	if ( OpenButton )
	{
		OpenButton->OnClicked.AddDynamic(this, &UUK_QuestList::OnOpenButtonClicked);
	}
}

void UUK_QuestList::OnOpenButtonClicked()
{
	// 1. 애니메이션이 재생 중이라면 클릭 무시 (중복 실행 방지)
	if ( IsAnimationPlaying(SlideAnimation) )
	{
		return;
	}

	if ( !SlideAnimation ) return;

	// 기존 바인딩 해제
	UnbindAllFromAnimationFinished(SlideAnimation);

	// 끝났을 때 상태를 바꾸기 위한 델리게이트 설정
	FWidgetAnimationDynamicEvent EndEvent;
	EndEvent.BindDynamic(this, &UUK_QuestList::OnSlideAnimFinished);
	BindToAnimationFinished(SlideAnimation, EndEvent);

	if ( !bIsOpen )
	{
		// 열기 애니메이션 실행
		PlayAnimation(SlideAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
		// 열기 시작할 때 바로 true로 바꿀 수도 있고, 
		// 끝날 때 바꾸고 싶다면 아래 OnSlideAnimFinished에서 처리하세요.
	}
	else
	{
		// 닫기 애니메이션 실행 (역재생)
		PlayAnimation(SlideAnimation, 0.0f, 1, EUMGSequencePlayMode::Reverse, 1.0f, false);
	}
}

void UUK_QuestList::OnSlideAnimFinished()
{
	// 애니메이션이 완전히 끝난 시점에 상태값 반전
	bIsOpen = !bIsOpen;

	// 바인딩 해제
	UnbindAllFromAnimationFinished(SlideAnimation);
}
