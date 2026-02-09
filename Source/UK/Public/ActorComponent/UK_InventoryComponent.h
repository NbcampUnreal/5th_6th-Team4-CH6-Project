// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UK_InventoryComponent.generated.h"

struct FUK_ItemData;
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
public:
	FInventorySlot() : ItemID(EName::None), Quantity(0) {  }
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	bool isEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
	void Clear() { ItemID = NAME_None; Quantity = 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdate);

UCLASS(ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ))
class UK_API UUK_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUK_InventoryComponent();

	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void BroadcastInventoryUpdate() const;

	UFUNCTION(BlueprintCallable)
	TArray<FInventorySlot>  GetItemSlot() const { return InventorySlots; }
	UFUNCTION(BlueprintCallable)
	TArray<FInventorySlot>  GetWeaponSlot() const { return WeaponSlots; }

	UFUNCTION(BlueprintCallable)
	int32 AddItem(FName ItemID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable)
	int32 RemoveItem(FName ItemID, int32 Amount = 1);

	FInventorySlot* FindItemSlot(FName ItemID, const FUK_ItemData* ItemData);
	FInventorySlot* FindEmptyItemSlot();

	UFUNCTION(BlueprintCallable)
	bool AddWeapon(FName ItemID, int32 index = -1);
	bool RemoveWeapon(FName ItemID, int32 index);

	FInventorySlot* FindWeaponSlot(FName ItemID);
	FInventorySlot* FindWeaponSlotbyIndex(int32 index);
	FInventorySlot* FindEmptyWeaponSlot();


	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdate OnInventoryUpdate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItemDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FInventorySlot> WeaponSlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 InventoryCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 WeaponCapacity;

};
