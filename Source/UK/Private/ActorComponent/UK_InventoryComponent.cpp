// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_InventoryComponent.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "UI/Inventory/UK_InvUI.h"

// Sets default values for this component's properties
UUK_InventoryComponent::UUK_InventoryComponent() :
	InventoryCapacity(1000),
	WeaponCapacity(3)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;


}

// Called every frame
//void UUK_InventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//}


 //Called when the game starts
void UUK_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventorySlots.SetNum(InventoryCapacity);
	WeaponSlots.SetNum(WeaponCapacity);

}

void UUK_InventoryComponent::BroadcastInventoryUpdate() const
{
}

int32 UUK_InventoryComponent::AddItem(FName ItemID, int32 Amount)
{
	// 유효성 검사
	if ( !IsValid(ItemDataTable) || Amount <= 0 )
		return false;

	// 데이터 테이블에서 데이터 찾아오기
	const FUK_ItemData* ItemData = ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("UUK_InventoryComponent::AddItem"));
	if ( ItemData == nullptr )
	{
		UE_LOG(LogTemp, Display, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다 : %s"), *ItemID.ToString());
		return false;
	}

	//추가될 아이템의 수
	int32 AmountToAdd = Amount;

	// 아이템이 중복으로 중접될수 있다면
	if ( ItemData->MaxItemStack > 1 )
	{
		// 동일 아이템 슬롯 확인
		FInventorySlot* ExistingSlot = FindItemSlot(ItemID, ItemData);
		if ( ExistingSlot != nullptr )
		{
			// 슬롯에 합칠수 있는 남은 공간
			int32 SpaceLeft = ItemData->MaxItemStack - ExistingSlot->Quantity;

			// 슬롯에 추가할 수 있는 아이템의 수 계산
			int32 AmountToFill = FMath::Min(AmountToAdd, SpaceLeft);
			ExistingSlot->Quantity += AmountToFill; /*아이템 추가*/
			AmountToAdd -= AmountToFill; /*추가 되지 못한 아이템의 갯수*/
		}
	}
	while ( AmountToAdd > 0 )
	{
		FInventorySlot* EmptySlot = FindEmptyItemSlot();
		if ( EmptySlot == nullptr )
		{
			UE_LOG(LogTemp, Display, TEXT("인벤토리 공간이 부족합니다."));

			return AmountToAdd; /*추가하지 못하고 남은 아이템의 갯수를 반환해 줌*/
		}

		int32 AmountToFill = FMath::Min(ItemData->MaxItemStack, AmountToAdd);
		EmptySlot->ItemID = ItemID;
		EmptySlot->Quantity = AmountToFill;
		AmountToAdd -= AmountToFill;
	}
	OnItemAdded.Broadcast(ItemID, Amount);
	OnInventoryUpdate.Broadcast();

	return true;
}

int32 UUK_InventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	// 유효성 검사
	if ( IsValid(ItemDataTable) == false || Amount <= 0 )
		return false;

	/*추가와 로직이 비슷함 이하 생략*/
	const FUK_ItemData* ItemData = ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("UUK_InventoryComponent::AddItem"));
	if ( ItemData == nullptr )
	{
		UE_LOG(LogTemp, Display, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다 : %s"), *ItemID.ToString());
		return false;
	}

	int32 AmountToRemove = Amount;

	while ( AmountToRemove > 0 )
	{
		FInventorySlot* ExistingSlot = FindItemSlot(ItemID, ItemData);

		if ( ExistingSlot == nullptr )
		{
			UE_LOG(LogTemp, Display, TEXT("제거될 아이템이 없습니다"));
			return AmountToRemove;
		}

		int32 AmountToTake = FMath::Min(AmountToRemove, ExistingSlot->Quantity);
		ExistingSlot->Quantity -= AmountToTake;
		AmountToRemove -= AmountToTake;

		if ( ExistingSlot->Quantity == 0 )
		{
			ExistingSlot->Clear();
		}
	}
	OnInventoryUpdate.Broadcast();
	return true;
}

FInventorySlot* UUK_InventoryComponent::FindItemSlot(FName ItemID, const FUK_ItemData* ItemData)
{
	for ( FInventorySlot& Slot : InventorySlots )
	{
		if ( !Slot.isEmpty() && Slot.ItemID == ItemID && Slot.Quantity < ItemData->MaxItemStack )
		{
			return &Slot;
		}
	}
	return nullptr;
}

FInventorySlot* UUK_InventoryComponent::FindEmptyItemSlot()
{
	for ( FInventorySlot& Slot : InventorySlots )
	{
		if ( Slot.isEmpty()  == true)
		{
			return &Slot;
		}
	}
	return nullptr;
}

bool UUK_InventoryComponent::AddWeapon(FName ItemID, int32 index)
{
	if ( index == -1 ) /*자동 으로 빈자리에 추가*/
	{
		FInventorySlot* EmptySlot = FindEmptyWeaponSlot();
		if ( EmptySlot == nullptr )
		{
			UE_LOG(LogTemp, Display, TEXT("남은 슬롯이 없습니다."));
			return false;
		}
		EmptySlot->ItemID = ItemID;
		EmptySlot->Quantity = 1;
		RemoveItem(ItemID);
		return true;
	}

	// 기존에 있던 슬롯
	FInventorySlot* Slot = FindWeaponSlotbyIndex(index);

	if ( Slot->isEmpty() == true) /*지정시 자리에 있던 무기와 교환 or 지정된 자리에 추가*/
	{
		Slot->ItemID = ItemID;
		Slot->Quantity = 1;
		RemoveItem(ItemID);
	}
	else
	{
		RemoveItem(ItemID);
		RemoveWeapon(Slot->ItemID, index);
		Slot->ItemID = ItemID;
		Slot->Quantity = 1;
	}
	return true;
}

bool UUK_InventoryComponent::RemoveWeapon(FName ItemID, int32 index)
{
	FInventorySlot* WeaponSlot = &WeaponSlots[ index ];
	
	if ( AddItem(WeaponSlot->ItemID) == false )
	{
		return false;
	}
	WeaponSlot->Clear();
	return true;
}


FInventorySlot* UUK_InventoryComponent::FindWeaponSlot(FName ItemID)
{
	for ( FInventorySlot& Slot : WeaponSlots )
	{
		if ( Slot.isEmpty() == false && Slot.ItemID == ItemID )
		{
			return &Slot;
		}
	}

	return nullptr;
}

FInventorySlot* UUK_InventoryComponent::FindWeaponSlotbyIndex(int32 index)
{
	return &WeaponSlots[ index ];
}

FInventorySlot* UUK_InventoryComponent::FindEmptyWeaponSlot()
{
	for ( FInventorySlot& Slot : WeaponSlots )
	{
		if ( Slot.isEmpty() == true)
		{
			return &Slot;
		}
	}
	return nullptr;
}


