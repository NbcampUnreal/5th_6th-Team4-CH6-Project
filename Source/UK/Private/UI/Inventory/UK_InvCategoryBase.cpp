#include "UI/Inventory/UK_InvCategoryBase.h"
#include "Components/UniformGridPanel.h"

//Slot
#include "UI/Inventory/UK_InvSlot.h"

void UUK_InvCategoryBase::NativeConstruct()
{
	Super::NativeConstruct();

	CreateSlots();
}

void UUK_InvCategoryBase::CreateSlots() //ㅇ 
{
	//지울거
	UE_LOG(LogTemp, Warning, TEXT("Category ItemDataTable: %s"), *GetNameSafe(ItemDataTable));
	UE_LOG(LogTemp, Warning, TEXT("CreateSlots: This=%s Outer=%s World=%s Table=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetOuter()),
		*GetNameSafe(GetWorld()),
		*GetNameSafe(ItemDataTable));
	UE_LOG(LogTemp, Warning, TEXT("[CreateSlots] This=%s SlotGrid=%s InvSlotClass=%s CurrentSlot=%d MaxSlot=%d"),
		*GetNameSafe(this),
		*GetNameSafe(SlotGrid),
		*GetNameSafe(InvSlotClass),
		CurrentSlot,
		MaxSlot);



	if ( !SlotGrid || !InvSlotClass )
	{
		UE_LOG(LogTemp, Error, TEXT("[CreateSlots] RETURN! SlotGrid or InvSlotClass is NULL"));

		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();

	// 현재 슬롯 수는 최대 슬롯을 넘지 못하게 제한
	const int32 SlotCountToCreate = FMath::Clamp(CurrentSlot, 0, MaxSlot);

	for ( int32 i = 0; i < SlotCountToCreate; ++i )
	{
		UUK_InvSlot* SlotWidget = CreateWidget<UUK_InvSlot>(this, InvSlotClass);
		if ( !SlotWidget )
		{
			UE_LOG(LogTemp, Warning, TEXT("[CreateSlots] Created slot widget i=%d -> %s"), i, *GetNameSafe(SlotWidget));//지울거
			continue;
		}

		SlotWidget->SlotIndex = i;
		SlotWidget->ItemDataTable = ItemDataTable;

		SlotGrid->AddChildToUniformGrid(
			SlotWidget,
			i / SlotColumns,
			i % SlotColumns
		);

		SlotWidgets.Add(SlotWidget);
	}
}

void UUK_InvCategoryBase::SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots)
{
	//invUI -> 카테고리 -> 슬롯 위젯, 빈 슬롯은 표시X
	FilteredSlots.Empty();
	int32 PassCount = 0;

	for ( const FInventorySlot& InvSlot : InAllSlots )
	{
		// 1. isEmpty 체크 확인 로그
		if ( InvSlot.isEmpty() )
		{
			// 수량이 0보다 큰데도 isEmpty가 true라면 이게 범인입니다.
			if ( InvSlot.Quantity > 0 )
			{
				UE_LOG(LogTemp, Error, TEXT("심각: 수량이 %d인데 isEmpty()가 true를 반환함!"), InvSlot.Quantity);
			}
			continue;
		}

		// 2. 카테고리 체크 확인 로그
		if ( !IsItemAllowed(InvSlot) )
		{
			UE_LOG(LogTemp, Error, TEXT("카테고리 거부됨: %s"), *InvSlot.ItemID.ToString());
			continue;
		}

		FilteredSlots.Add(InvSlot);
		PassCount++;

		if (FilteredSlots.Num() >= CurrentSlot ) break;
	}

	UE_LOG(LogTemp, Error, TEXT("최종 필터링 통과 개수: %d"), PassCount); // 지울거
	UpdateSlots();
}

void UUK_InvCategoryBase::UpdateSlots()
{

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (!SlotWidgets[i] ) continue;

		if (FilteredSlots.IsValidIndex(i))
		{
			SlotWidgets[i]->SetSlotData(i, FilteredSlots[i]);
		}
		else
		{
			FInventorySlot EmptySlot;
			SlotWidgets[i]->SetSlotData(i, EmptySlot);
		}
	}
}

void UUK_InvCategoryBase::AddSlot(int32 AddCount) //ㅇ
{

	int32 OldSlotCount = CurrentSlot;

	if ( AddCount <= 0 )
		return;

	CurrentSlot = FMath::Clamp(
		CurrentSlot + AddCount,
		0,
		MaxSlot
	);

	if (CurrentSlot > OldSlotCount)
	{
		CreateSlots();
	}

	UpdateSlots();
}

bool UUK_InvCategoryBase::IsItemAllowed(const FInventorySlot& InSlot) const
{
	//현재 모든 아이템들 들어오게 허용
	return true;
}