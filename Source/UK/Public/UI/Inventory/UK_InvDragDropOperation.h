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

	//기존 슬롯 인덱스 이름이 애매해서 유지해도 되지만 의미를 명확히 함
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = ( DraggedSlotdata = "true" ))
	int32 SourceIndex = INDEX_NONE;

	//실제 InventorySlots의 원본 인덱스
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = ( DraggedSlotdata = "true" ))
	int32 SourceInventoryIndex = INDEX_NONE;
};
