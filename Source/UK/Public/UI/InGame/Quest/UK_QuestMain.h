#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_QuestMain.generated.h"

class UUKQuestUIManagerSubsystem;
class UUK_QuestLists;

UCLASS()
class UK_API UUK_QuestMain : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UUK_QuestLists* QuestLists;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUKQuestUIManagerSubsystem> QuestUIManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FName> QuestIds;
	
};
