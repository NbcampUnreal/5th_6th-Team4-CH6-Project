#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_MiniMapNorth.generated.h"

class UTextBlock;

UCLASS()
class UK_API UUK_MiniMapNorth : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextLabel;

	UFUNCTION(BlueprintCallable, Category = "Compass")
	void SetDirection(const FString& InText);
};
