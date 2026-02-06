// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponent/UK_InventoryComponent.h"
#include "DataAsset/Data/UK_ItemData.h"

// Sets default values for this component's properties
UUK_InventoryComponent::UUK_InventoryComponent()
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
	InventorySlots.SetNum(Capacity);
}

void UUK_InventoryComponent::BroadcastInventoryUpdate() const
{
	OnInventoryUpdate.Broadcast();
}

void UUK_InventoryComponent::AddItem(FName ItemID, int32 Amount)
{

}

void UUK_InventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
}

FInventorySlot* UUK_InventoryComponent::FindItemSlot(FName ItemID, const FUK_ItemData* ItemData)
{
	return nullptr;
}

FInventorySlot* UUK_InventoryComponent::FindEmptySlot()
{
	return nullptr;
}


