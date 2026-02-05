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
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
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
	void AddItem(FName ItemID, int32 Amount = 1);
	void RemoveItem(FName ItemID, int32 Amount = 1);

	TArray<FInventorySlot>  GetSlot() const { return InventorySlots; }

	FInventorySlot* FindItemSlot(FName ItemID, const FUK_ItemData* ItemData);
	FInventorySlot* FindEmptySlot();
public:
	UPROPERTY(BlueprintAssignable)
	FOnInventoryUpdate OnInventoryUpdate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UDataTable> ItemDataTable;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 Capacity;
};
