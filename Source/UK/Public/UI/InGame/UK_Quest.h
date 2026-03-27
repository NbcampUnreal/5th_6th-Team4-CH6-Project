#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UK_Quest.generated.h"

class UUKQuestUIManagerSubsystem;
class UUK_QuestMain;
class USoundCue;

UCLASS()
class UK_API UUK_Quest : public UUserWidget
{
	GENERATED_BODY()
public:
	UUK_Quest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetQuestUI(FName QuestID,const FText& NPCName,const FText& Dialogue,const FText& QuestDesc,const FText& AcceptText,const FText& ExitText);

	// 현재 대화 데이터를 읽어 UI에 반영
	UFUNCTION(BlueprintCallable, Category = "Quest|Dialogue")
	void RefreshDialogueUI();

	// 이름/대사만 대화 기준으로 갱신
	UFUNCTION(BlueprintCallable, Category = "Quest|Dialogue")
	void SetDialogueOnly(
		const FText& NPCName,
		const FText& Dialogue
	);

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();
	
	UFUNCTION()
	void OnPlayButtonHovered();
	
	UFUNCTION()
	void OnPlayButtonUnHovered();

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
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* ClickSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* HoveredSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* UnHoveredSound;

};
