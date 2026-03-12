#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "GameplayTagContainer.h"
#include "UK_InvCategoryBase.generated.h"

class UUniformGridPanel;
class UUK_InvSlot;

UENUM(BlueprintType)
enum class EInvCategory : uint8
{
	All,
	Weapon,
	Food,
	Material
};
//info
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategorySlotHovered, const FInventorySlot&, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCategorySlotUnhovered);

UCLASS()
class UK_API UUK_InvCategoryBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//카테고리 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inv")
	EInvCategory CategoryType = EInvCategory::All;
	//인벤토리 슬롯 배열 생성
	UFUNCTION(BlueprintCallable, Category = "Inv")
	void SetInvArraySlots(const TArray<FInventorySlot>& InAllSlots);
	//슬롯 갯수 추가
	UFUNCTION(BlueprintCallable, Category = "Inv")
	void AddSlot(int32 AddCount);

	virtual void NativeConstruct() override;
	//슬롯이 허용되는지 여부
	virtual bool IsItemAllowed(const FInventorySlot& InSlot) const;
	//슬롯 생성
	void CreateSlots();
	//슬롯 정보 업데이트
	void UpdateSlots();
	//info
	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnCategorySlotHovered OnCategorySlotHovered;

	UPROPERTY(BlueprintAssignable, Category = "InvHover")
	FOnCategorySlotUnhovered OnCategorySlotUnhovered;

	UFUNCTION()
	void HandleSlotHovered(const FInventorySlot& SlotData);

	UFUNCTION()
	void HandleSlotUnhovered();

	//바인드
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UUniformGridPanel* SlotGrid;

	//인벤토리 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inv")
	TSubclassOf<UUK_InvSlot> InvSlotClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inv")
	UDataTable* ItemDataTable;

	//==슬롯 갯수 한 페이지당==
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotColumns = 7; //슬롯 배열 가로
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InvSlot")
	int32 SlotRows = 4; //슬롯 배열 세로
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 CurrentSlot = 7; //슬롯 현재 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InvSlotCount")
	int32 MaxSlot = 999; //슬롯 최대 갯수
	//=========================
	
	//인벤토리 전체 슬롯 배열
	TArray<FInventorySlot> FilteredSlots;
	TArray<UUK_InvSlot*> SlotWidgets;

	//드래그
	//카테고리별 드래그 허용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inv|Drag")
	bool bAllowSlotDrag = false;

	//무기 루트 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inv")
	FGameplayTag WeaponRootTag;
};
