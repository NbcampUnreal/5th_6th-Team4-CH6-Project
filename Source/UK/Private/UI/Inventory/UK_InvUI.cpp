#include "UI/Inventory/UK_InvUI.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Inventory/UK_CategoryTap.h"

//인벤토리 컴포넌트
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_InvCategoryBase.h"
#include "Character/UK_CharacterBase.h"

void UUK_InvUI::NativeConstruct()
{
	Super::NativeConstruct();

	if ( TapALL )
	{
		TapALL->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if ( TapWeapon )
	{
		TapWeapon->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if ( TapFood )
	{
		TapFood->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if ( TapMaterial )
	{
		TapMaterial->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}
	//소유한 플레이어 폰의 인벤토리 컴포넌트 바인드
	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (IsValid(CB))
	{
		BindInventoryComponent(CB->GetInventoryComponent());
	}

	auto BindCategory = [this](UUK_InvCategoryBase* Cat)
		{
			if (!Cat) return;
			Cat->OnCategorySlotHovered.AddDynamic(this, &UUK_InvUI::HandleCategoryHovered);
			Cat->OnCategorySlotUnhovered.AddDynamic(this, &UUK_InvUI::HandleCategoryUnhovered);
		};

	BindCategory(CategoryALL);
	BindCategory(CategoryWeapon);
	BindCategory(CategoryFood);
	BindCategory(CategoryMaterial);
}

void UUK_InvUI::BindInventoryComponent(UUK_InventoryComponent* InInvComp)
{
	if ( !InInvComp )
		return;
	//인벤토리 컴포넌트 저장
	InvComp = InInvComp;
	//인벤토리 컴포넌트의 OnInventoryUpdate 델리게이트에 바인드
	InvComp->OnInventoryUpdate.AddDynamic
	(
		this,
		&UUK_InvUI::OnInvCompUpdated
	);

	OnInvCompUpdated();
}

void UUK_InvUI::OnInvCompUpdated()
{

	if ( !InvComp ) return;
	//인벤토리 컴포넌트에서 모든 슬롯 배열 가져오기
	const TArray<FInventorySlot>& AllSlots = InvComp->GetItemSlot();

	if (CategoryALL)
	{
		int32 NewAllSlotCount = 0; //추가

		if ( CategoryWeapon )   NewAllSlotCount += CategoryWeapon->CurrentSlot;  
		if ( CategoryFood )     NewAllSlotCount += CategoryFood->CurrentSlot;   
		if ( CategoryMaterial ) NewAllSlotCount += CategoryMaterial->CurrentSlot;

		NewAllSlotCount = FMath::Clamp(NewAllSlotCount, 0, CategoryALL->MaxSlot); 

		if (CategoryALL->CurrentSlot != NewAllSlotCount) 
		{
			CategoryALL->CurrentSlot = NewAllSlotCount; 
			CategoryALL->CreateSlots();                 
		}
	}

	//CategoryBase에 모든 슬롯 배열 전달
	if (CategoryALL) CategoryALL->SetInvArraySlots(AllSlots);
	if (CategoryWeapon) CategoryWeapon->SetInvArraySlots(AllSlots);
	if (CategoryFood) CategoryFood->SetInvArraySlots(AllSlots);
	if (CategoryMaterial) CategoryMaterial->SetInvArraySlots(AllSlots);
}

void UUK_InvUI::HandleCategoryHovered(const FInventorySlot& SlotData)
{
	OnInvSlotPreview.Broadcast(SlotData);
}

void UUK_InvUI::HandleCategoryUnhovered()
{
	OnInvSlotPreviewCleared.Broadcast();
}

void UUK_InvUI::CategoryTap(UUK_CategoryTap* CategoryTap)
{
	if ( !InvCateSwitcher ) return;

	if ( CategoryTap == TapALL )
	{
		InvCateSwitcher->SetActiveWidgetIndex(0);
	}
	else if ( CategoryTap == TapWeapon )
	{
		InvCateSwitcher->SetActiveWidgetIndex(1);
	}
	else if ( CategoryTap == TapFood )
	{
		InvCateSwitcher->SetActiveWidgetIndex(2);
	}
	else if ( CategoryTap == TapMaterial )
	{
		InvCateSwitcher->SetActiveWidgetIndex(3);
	}
}
