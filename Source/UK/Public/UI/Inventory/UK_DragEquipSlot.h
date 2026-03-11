#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UK_DragEquipSlot.generated.h"

class UImage;;

UCLASS()
class UK_API UUK_DragEquipSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* EquipSlot;

protected:

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry, 
		const FDragDropEvent& InDragDropEvent, 
		UDragDropOperation* InOperation) override;
	
};
