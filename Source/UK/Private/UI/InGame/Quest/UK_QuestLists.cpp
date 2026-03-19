// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/Quest/UK_QuestLists.h"

#include "Components/ScrollBox.h"
#include "UI/InGame/Quest/UK_QuestItem.h"

void UUK_QuestLists::AddQuestItem(FText QuestName, FText QuestDescription)
{
	if ( !QuestItemClass || !QuestScrollBox ) return;

	UUK_QuestItem* NewItem = CreateWidget<UUK_QuestItem>(GetWorld(), QuestItemClass);

	if ( NewItem )
	{
		// Date 변경시 함수 적용 ( 퀘스트 클리어 등 상태가 변경될 때)

		QuestScrollBox->AddChild(NewItem);

		// 추가된 항목으로 자동 스크롤 (필요시)
		QuestScrollBox->ScrollToEnd();
	}
}
