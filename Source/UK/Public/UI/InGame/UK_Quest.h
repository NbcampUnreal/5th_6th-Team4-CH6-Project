// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UK_Quest.generated.h"

class UUKQuestUIManagerSubsystem;
class UUK_QuestMain;

UCLASS()
class UK_API UUK_Quest : public UUserWidget
{
	GENERATED_BODY()
public:
	UUK_Quest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetQuestUI(FName QuestID,const FText& NPCName,const FText& Dialogue,const FText& QuestDesc,const FText& AcceptText,const FText& ExitText);

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:
	// 현재 표시 중인 퀘스트 ID
	FName CurrentQuestId;

	UPROPERTY(meta = (BindWidget))
	UButton* Accept_Button;

	UPROPERTY(meta = (BindWidget))
	UButton* Exit_Button;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NPCNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestDescText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AcceptTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExitTextBlock;

};
