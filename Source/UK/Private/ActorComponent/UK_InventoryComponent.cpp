// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_InventoryComponent.h"
#include "DataAsset/Data/UK_ItemData.h"
#include "DataAsset/Data/UK_WeaponItemData.h"
#include "UI/Inventory/UK_InvUI.h"
#include "Systems/Data/UK_InGameSave.h"

// Sets default values for this component's properties
UUK_InventoryComponent::UUK_InventoryComponent() :
	InventoryCapacity(1000),
	WeaponCapacity(3),
	Gold(0) //수정
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUK_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventorySlots.SetNum(InventoryCapacity);
	WeaponSlots.SetNum(WeaponCapacity);
}

void UUK_InventoryComponent::BroadcastInventoryUpdate() const
{
	const_cast< UUK_InventoryComponent* >( this )->OnInventoryUpdate.Broadcast(); //수정
}

int32 UUK_InventoryComponent::AddItem(FName ItemID, int32 Amount)
{
	return AddItem(ItemID, Amount, true, true); //수정
}

int32 UUK_InventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	return RemoveItem(ItemID, Amount, true); //수정
}

FInventorySlot* UUK_InventoryComponent::FindItemSlot(FName ItemID, const FUK_ItemData* ItemData,
	const FUK_WeaponItemData* WeaponData)
{
	if ( ItemData )
	{
		for ( FInventorySlot& Slot : InventorySlots )
		{
			if ( !Slot.isEmpty() && Slot.ItemID == ItemID && Slot.Quantity < ItemData->MaxItemStack )
			{
				return &Slot;
			}
		}
	}
	else if ( WeaponData )
	{
		for ( FInventorySlot& Slot : InventorySlots )
		{
			if ( !Slot.isEmpty() && Slot.ItemID == ItemID && Slot.Quantity < WeaponData->MaxItemStack )
			{
				return &Slot;
			}
		}

	}
	return nullptr;
}

FInventorySlot* UUK_InventoryComponent::FindEmptyItemSlot()
{
	for ( FInventorySlot& Slot : InventorySlots )
	{
		if ( Slot.isEmpty() == true )
		{
			return &Slot;
		}
	}
	return nullptr;
}

int32 UUK_InventoryComponent::GetItemTotalQuantity(FName ItemID) const
{
	int32 Total = 0;

	for ( const FInventorySlot& Slot : InventorySlots )
	{
		if ( !Slot.isEmpty() && Slot.ItemID == ItemID )
		{
			Total += Slot.Quantity;
		}
	}

	return Total;
}

bool UUK_InventoryComponent::AddWeapon(FName ItemID, int32 index)
{
	if ( index == -1 ) /*자동 으로 빈자리에 추가*/
	{
		FInventorySlot* EmptySlot = FindEmptyWeaponSlot();
		if ( EmptySlot == nullptr )
		{
			UE_LOG(LogTemp, Warning, TEXT("남은 무기 슬롯이 없습니다.")); //수정
			return false;
		}

		if ( RemoveItem(ItemID, 1, false) != 0 ) //수정
		{
			UE_LOG(LogTemp, Warning, TEXT("장착 실패 - 인벤토리에서 아이템 제거 실패 : %s"), *ItemID.ToString()); //수정
			return false;
		}

		EmptySlot->ItemID = ItemID;
		EmptySlot->Quantity = 1;

		const int32 ChangedIndex = static_cast< int32 >( EmptySlot - WeaponSlots.GetData() ); //수정
		OnChangedWeapon.Broadcast(ChangedIndex); //수정
		OnInventoryUpdate.Broadcast(); //수정
		return true;
	}

	if ( !WeaponSlots.IsValidIndex(index) ) //수정
	{
		UE_LOG(LogTemp, Warning, TEXT("잘못된 무기 슬롯 index 입니다 : %d"), index); //수정
		return false;
	}

	FInventorySlot* Slot = &WeaponSlots[ index ];

	if ( !Slot ) //수정
	{
		return false;
	}

	if ( !Slot->isEmpty() && Slot->ItemID == ItemID ) //수정
	{
		UE_LOG(LogTemp, Warning, TEXT("같은 무기를 같은 슬롯에 다시 장착 시도 : %s"), *ItemID.ToString()); //수정
		return false;
	}

	if ( Slot->isEmpty() == true ) /*지정시 자리에 있던 무기와 교환 or 지정된 자리에 추가*/
	{
		if ( RemoveItem(ItemID, 1, false) != 0 ) //수정
		{
			UE_LOG(LogTemp, Warning, TEXT("장착 실패 - 인벤토리에서 아이템 제거 실패 : %s"), *ItemID.ToString()); //수정
			return false;
		}

		Slot->ItemID = ItemID;
		Slot->Quantity = 1;
		OnChangedWeapon.Broadcast(index);
		OnInventoryUpdate.Broadcast(); //수정
		return true;
	}
	else
	{
		const FName OldItemID = Slot->ItemID; //수정

		if ( AddItem(OldItemID, 1, false, false) != 0 ) //수정
		{
			UE_LOG(LogTemp, Warning, TEXT("교체 실패 - 기존 장착 무기를 인벤토리로 되돌리지 못했습니다 : %s"), *OldItemID.ToString()); //수정
			return false;
		}

		if ( RemoveItem(ItemID, 1, false) != 0 ) //수정
		{
			UE_LOG(LogTemp, Warning, TEXT("교체 실패 - 새 무기를 인벤토리에서 제거하지 못했습니다 : %s"), *ItemID.ToString()); //수정

			RemoveItem(OldItemID, 1, false); //수정
			Slot->ItemID = OldItemID; //수정
			Slot->Quantity = 1; //수정

			OnChangedWeapon.Broadcast(index); //수정
			OnInventoryUpdate.Broadcast(); //수정
			return false;
		}

		Slot->ItemID = ItemID;
		Slot->Quantity = 1;
	}

	OnChangedWeapon.Broadcast(index);
	OnInventoryUpdate.Broadcast(); //수정
	return true;
}

bool UUK_InventoryComponent::RemoveWeapon(FName ItemID, int32 index)
{
	if ( !WeaponSlots.IsValidIndex(index) ) //수정
	{
		return false;
	}

	FInventorySlot* WeaponSlot = &WeaponSlots[ index ];

	if ( WeaponSlot == nullptr || WeaponSlot->isEmpty() ) //수정
	{
		return false;
	}

	if ( AddItem(WeaponSlot->ItemID, 1, false, false) != 0 ) //수정
	{
		return false;
	}

	OnChangedWeapon.Broadcast(index);
	WeaponSlot->Clear();
	OnInventoryUpdate.Broadcast(); //수정
	return true;
}

int32 UUK_InventoryComponent::subtractionGold(int32 cost)
{
	if ( Gold - cost < 0 )
	{
		return Gold - cost; // 부족한 값을 리턴  
	}

	Gold -= cost;
	OnChangedGold.Broadcast(Gold);
	return Gold;
}

bool UUK_InventoryComponent::AddGold(int32 Value)
{
	// 오버플로우 방지
	if ( Gold + Value < 0 )
	{
		return false;
	}
	Gold += Value;
	OnChangedGold.Broadcast(Gold);
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
	if ( !WeaponSlots.IsValidIndex(index) ) //수정
	{
		return nullptr;
	}

	return &WeaponSlots[ index ];
}

FInventorySlot* UUK_InventoryComponent::FindEmptyWeaponSlot()
{
	for ( FInventorySlot& Slot : WeaponSlots )
	{
		if ( Slot.isEmpty() == true )
		{
			return &Slot;
		}
	}
	return nullptr;
}

//추가
int32 UUK_InventoryComponent::AddItem(FName ItemID, int32 Amount, bool bBroadcastItemAdded, bool bBroadcastInventoryUpdate)
{
	if ( (!IsValid(ItemDataTable) && !IsValid(WeaponDataTable) ) || Amount <= 0 )
	{
		return Amount;
	}

	const FUK_ItemData* ItemData = nullptr;
	if ( IsValid(ItemDataTable) )
	{
		ItemData = ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("AddItem"));
	}

	const FUK_WeaponItemData* WeaponData = nullptr;
	if ( IsValid(WeaponDataTable) )
	{
		WeaponData = WeaponDataTable->FindRow<FUK_WeaponItemData>(ItemID, TEXT("AddItem"));
	}

	if ( ItemData == nullptr && WeaponData == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다 : %s"), *ItemID.ToString());
		return Amount;
	}

	int32 AmountToAdd = Amount;

	if ( ItemData && ItemData->MaxItemStack > 1 )
	{
		while ( AmountToAdd > 0 )
		{
			FInventorySlot* ExistingSlot = FindItemSlot(ItemID, ItemData, nullptr);
			if ( ExistingSlot == nullptr )
			{
				break;
			}

			const int32 SpaceLeft = ItemData->MaxItemStack - ExistingSlot->Quantity;
			if ( SpaceLeft <= 0 )
			{
				break;
			}

			const int32 AmountToFill = FMath::Min(AmountToAdd, SpaceLeft);
			ExistingSlot->Quantity += AmountToFill;
			AmountToAdd -= AmountToFill;
		}
	}
	else if ( WeaponData && WeaponData->MaxItemStack > 1 )
	{
		while ( AmountToAdd > 0 )
		{
			FInventorySlot* ExistingSlot = FindItemSlot(ItemID, nullptr, WeaponData);
			if ( ExistingSlot == nullptr )
			{
				break;
			}

			const int32 SpaceLeft = WeaponData->MaxItemStack - ExistingSlot->Quantity;
			if ( SpaceLeft <= 0 )
			{
				break;
			}

			const int32 AmountToFill = FMath::Min(AmountToAdd, SpaceLeft);
			ExistingSlot->Quantity += AmountToFill;
			AmountToAdd -= AmountToFill;
		}
	}

	while ( AmountToAdd > 0 )
	{
		FInventorySlot* EmptySlot = FindEmptyItemSlot();
		if ( EmptySlot == nullptr )
		{
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 공간이 부족합니다."));

			if ( bBroadcastInventoryUpdate )
			{
				OnInventoryUpdate.Broadcast();
			}

			return AmountToAdd;
		}

		const int32 CurrentMaxStack = ItemData ? ItemData->MaxItemStack : ( WeaponData ? WeaponData->MaxItemStack : 1 );
		const int32 AmountToFill = FMath::Min(CurrentMaxStack, AmountToAdd);

		EmptySlot->ItemID = ItemID;
		EmptySlot->Quantity = AmountToFill;
		AmountToAdd -= AmountToFill;
	}

	if ( bBroadcastItemAdded )
	{
		OnItemAdded.Broadcast(ItemID, Amount);
	}

	if ( bBroadcastInventoryUpdate )
	{
		OnInventoryUpdate.Broadcast();
	}

	return 0;
}

//추가
int32 UUK_InventoryComponent::RemoveItem(FName ItemID, int32 Amount, bool bBroadcastInventoryUpdate)
{
	if ( ( !IsValid(ItemDataTable) && !IsValid(WeaponDataTable) ) || Amount <= 0 )
	{
		return Amount;
	}

	const FUK_ItemData* ItemData = nullptr;
	if ( IsValid(ItemDataTable) )
	{
		ItemData = ItemDataTable->FindRow<FUK_ItemData>(ItemID, TEXT("RemoveItem"));
	}

	const FUK_WeaponItemData* WeaponData = nullptr;
	if ( IsValid(WeaponDataTable) )
	{
		WeaponData = WeaponDataTable->FindRow<FUK_WeaponItemData>(ItemID, TEXT("RemoveItem"));
	}

	if ( ItemData == nullptr && WeaponData == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다 : %s"), *ItemID.ToString());
		return Amount;
	}

	int32 AmountToRemove = Amount;

	while ( AmountToRemove > 0 )
	{
		FInventorySlot* ExistingSlot = FindAnyItemSlot(ItemID);

		if ( ExistingSlot == nullptr )
		{
			UE_LOG(LogTemp, Warning, TEXT("제거될 아이템이 부족합니다 : %s"), *ItemID.ToString());

			if ( bBroadcastInventoryUpdate )
			{
				OnInventoryUpdate.Broadcast();
			}

			return AmountToRemove;
		}

		const int32 AmountToTake = FMath::Min(AmountToRemove, ExistingSlot->Quantity);
		ExistingSlot->Quantity -= AmountToTake;
		AmountToRemove -= AmountToTake;

		if ( ExistingSlot->Quantity <= 0 )
		{
			ExistingSlot->Clear();
		}
	}

	if ( bBroadcastInventoryUpdate )
	{
		OnInventoryUpdate.Broadcast();
	}

	return 0;
}

//추가
FInventorySlot* UUK_InventoryComponent::FindAnyItemSlot(FName ItemID)
{
	for ( FInventorySlot& Slot : InventorySlots )
	{
		if ( !Slot.isEmpty() && Slot.ItemID == ItemID )
		{
			return &Slot;
		}
	}

	return nullptr;
}

//추가
bool UUK_InventoryComponent::RemoveItemByInventoryIndex(int32 InventoryIndex, int32 Amount, bool bBroadcastInventoryUpdate)
{
	if ( !InventorySlots.IsValidIndex(InventoryIndex) || Amount <= 0 )
	{
		return false;
	}

	FInventorySlot& Slot = InventorySlots[ InventoryIndex ];

	if ( Slot.isEmpty() || Slot.Quantity < Amount )
	{
		return false;
	}

	Slot.Quantity -= Amount;

	if ( Slot.Quantity <= 0 )
	{
		Slot.Clear();
	}

	if ( bBroadcastInventoryUpdate )
	{
		OnInventoryUpdate.Broadcast();
	}

	return true;
}

//추가
bool UUK_InventoryComponent::AddWeaponFromInventoryIndex(FName ItemID, int32 EquipIndex, int32 SourceInventoryIndex)
{
	if ( !WeaponSlots.IsValidIndex(EquipIndex) )
	{
		return false;
	}

	if ( !InventorySlots.IsValidIndex(SourceInventoryIndex) )
	{
		return false;
	}

	FInventorySlot& SourceSlot = InventorySlots[ SourceInventoryIndex ];
	if ( SourceSlot.isEmpty() || SourceSlot.ItemID != ItemID || SourceSlot.Quantity <= 0 )
	{
		return false;
	}

	FInventorySlot& EquipSlot = WeaponSlots[ EquipIndex ];

	if ( !EquipSlot.isEmpty() && EquipSlot.ItemID == ItemID )
	{
		return false;
	}

	if ( EquipSlot.isEmpty() )
	{
		if ( !RemoveItemByInventoryIndex(SourceInventoryIndex, 1, false) )
		{
			return false;
		}

		EquipSlot.ItemID = ItemID;
		EquipSlot.Quantity = 1;

		OnChangedWeapon.Broadcast(EquipIndex);
		OnInventoryUpdate.Broadcast();
		return true;
	}

	const FName OldItemID = EquipSlot.ItemID;

	if ( AddItem(OldItemID, 1, false, false) != 0 )
	{
		return false;
	}

	if ( !RemoveItemByInventoryIndex(SourceInventoryIndex, 1, false) )
	{
		RemoveItem(OldItemID, 1, false);
		EquipSlot.ItemID = OldItemID;
		EquipSlot.Quantity = 1;

		OnChangedWeapon.Broadcast(EquipIndex);
		OnInventoryUpdate.Broadcast();
		return false;
	}

	EquipSlot.ItemID = ItemID;
	EquipSlot.Quantity = 1;

	OnChangedWeapon.Broadcast(EquipIndex);
	OnInventoryUpdate.Broadcast();
	return true;
}

void UUK_InventoryComponent::ExportInventory(struct FInventorySaveData& OutData)
{
	OutData.ItemIds.Empty();
	OutData.ItemCounts.Empty();
	
	for (const FInventorySlot& Slot : InventorySlots)
	{
		if (!Slot.isEmpty())
		{
			OutData.ItemIds.Add(Slot.ItemID);
			OutData.ItemCounts.Add(Slot.Quantity);
		}
	}
	OutData.EquippedWeaponIds.Empty();
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (WeaponSlots.IsValidIndex(i) && !WeaponSlots[i].isEmpty())
		{
			OutData.EquippedWeaponIds.Add(WeaponSlots[i].ItemID);
		}
		else
		{
			OutData.EquippedWeaponIds.Add(FName("None"));
		}
	}
	
	OutData.Gold = this->Gold;
}

void UUK_InventoryComponent::ImportInventory(const struct FInventorySaveData& InData)
{
	for (FInventorySlot& Slot : InventorySlots) { Slot.Clear(); }
	for (FInventorySlot& Slot : WeaponSlots) { Slot.Clear(); }
	
	for (int32 i = 0; i < InData.ItemIds.Num(); ++i)
	{
		AddItem(InData.ItemIds[i], InData.ItemCounts[i], false, false);
	}
	
	for (int32 i = 0; i < InData.EquippedWeaponIds.Num(); ++i)
	{
		if (i >= 3) break;
		
		if (WeaponSlots.IsValidIndex(i) && InData.EquippedWeaponIds[i] != FName("None"))
		{
			WeaponSlots[i].ItemID = InData.EquippedWeaponIds[i];
			WeaponSlots[i].Quantity = 1;
			
			OnChangedWeapon.Broadcast(i);
		}
	}
	this->Gold = InData.Gold;
	
	BroadcastInventoryUpdate();
	OnChangedGold.Broadcast(this->Gold);
}
