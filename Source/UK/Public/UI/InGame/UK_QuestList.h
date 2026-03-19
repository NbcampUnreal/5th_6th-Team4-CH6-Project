// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_QuestList.generated.h"

/**
 * 
 */
UCLASS()
class UK_API UUK_QuestList : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// --- UI 컴포넌트 ---
	UPROPERTY(meta = ( BindWidget ))
	class UButton* OpenButton;

	// --- 애니메이션 ---
	UPROPERTY(meta = ( BindWidgetAnim ), Transient)
	UWidgetAnimation* SlideAnimation; // 열리고 닫히는 하나의 애니메이션

	// --- 상태 변수 ---
	bool bIsOpen = false;

	// --- 함수 ---
	UFUNCTION()
	void OnOpenButtonClicked();

	UFUNCTION()
	void OnSlideAnimFinished();
	
};
