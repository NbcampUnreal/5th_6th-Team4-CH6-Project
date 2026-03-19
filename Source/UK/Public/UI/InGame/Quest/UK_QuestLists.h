#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_QuestLists.generated.h"

class UScrollBox;
class UUK_QuestItem;

UCLASS()
class UK_API UUK_QuestLists : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void AddQuestItem(FText QuestName, FText QuestDescription);

protected:
	UPROPERTY(meta = ( BindWidget ))
	UScrollBox* QuestScrollBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUK_QuestItem> QuestItemClass;
	
};
