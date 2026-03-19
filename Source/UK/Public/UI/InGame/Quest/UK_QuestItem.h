#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_QuestItem.generated.h"

class UTextBlock;


UCLASS()
class UK_API UUK_QuestItem : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "QuestUI")
	void SetQusetData(const FText& InQuestTitle, const FText& InQuestDescription);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestTitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuestDescriptionText;


};
