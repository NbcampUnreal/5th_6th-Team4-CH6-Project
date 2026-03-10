#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_DraggedItem.generated.h"

class UImage;

UCLASS()
class UK_API UUK_DraggedItem : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = ( BindWidget ))
	UImage* ItemIcon;

	UFUNCTION(BlueprintCallable)
	void SetIcon(UTexture2D* IconTexture);
	
};
