#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UK_InvDragDropOperation.generated.h"

UCLASS()
class UK_API UUK_InvDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	//드래그된 슬롯의 데이터
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DraggedSlotdata = "true"))
	FInventorySlot DraggedSlotData;

	//원본슬롯의 인덱스
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DraggedSlotdata = "true"))
	int32 SourceIndex = INDEX_NONE;
};
