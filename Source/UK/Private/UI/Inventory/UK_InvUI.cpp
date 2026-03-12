#include "UI/Inventory/UK_InvUI.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Inventory/UK_CategoryTap.h"
#include "Components/TextBlock.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "UI/Inventory/UK_InvCategoryBase.h"
#include "Character/UK_CharacterBase.h"
#include "UI/Inventory/UK_DragEquipSlot.h"
#include "Tags/UK_GameplayTags.h"

void UUK_InvUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (TapALL)
	{
		TapALL->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
		TapALL->SetSelected(true);
	}

	if (TapWeapon)
	{
		TapWeapon->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if (TapFood)
	{
		TapFood->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	if (TapMaterial)
	{
		TapMaterial->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
	}

	UpdateTabTextOpacity(TapALL);

	if (CategoryALL)
	{
		CategoryALL->ItemDataTables = ItemDataTables;
		CategoryALL->CreateSlots();
	}

	if (CategoryWeapon)
	{
		CategoryWeapon->ItemDataTables = ItemDataTables;
		CategoryWeapon->CreateSlots();
	}

	if (CategoryFood)
	{
		CategoryFood->ItemDataTables = ItemDataTables;
		CategoryFood->CreateSlots();
	}

	if (CategoryMaterial)
	{
		CategoryMaterial->ItemDataTables = ItemDataTables;
		CategoryMaterial->CreateSlots();
	}

	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (IsValid(CB))
	{
		BindInventoryComponent(CB->GetInventoryComponent());

		if (EquipSlot1)
		{
			EquipSlot1->EquipIndex = 0;
			EquipSlot1->ItemDataTables = ItemDataTables;
			EquipSlot1->WeaponRootTag = UK_GameplayTags::Weapon::WeaponRoot;
			EquipSlot1->BindInventory(CB->GetInventoryComponent());
		}

		if (EquipSlot2)
		{
			EquipSlot2->EquipIndex = 1;
			EquipSlot2->ItemDataTables = ItemDataTables;
			EquipSlot2->WeaponRootTag = UK_GameplayTags::Weapon::WeaponRoot;
			EquipSlot2->BindInventory(CB->GetInventoryComponent());
		}

		if (EquipSlot3)
		{
			EquipSlot3->EquipIndex = 2;
			EquipSlot3->ItemDataTables = ItemDataTables;
			EquipSlot3->WeaponRootTag = UK_GameplayTags::Weapon::WeaponRoot;
			EquipSlot3->BindInventory(CB->GetInventoryComponent());
		}
	}

	auto BindCategory = [this] (UUK_InvCategoryBase* Cat)
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
	if (!InInvComp) return;

	InvComp = InInvComp;
	InvComp->OnInventoryUpdate.AddDynamic(this, &UUK_InvUI::OnInvCompUpdated);

	OnInvCompUpdated();
}

void UUK_InvUI::OnInvCompUpdated()
{
	if (!InvComp) return;

	const TArray<FInventorySlot>& AllSlots = InvComp->GetItemSlot();

	if (CategoryALL)
	{
		int32 NewAllSlotCount = 0;

		if (CategoryWeapon)   NewAllSlotCount += CategoryWeapon->CurrentSlot;
		if (CategoryFood)     NewAllSlotCount += CategoryFood->CurrentSlot;
		if (CategoryMaterial) NewAllSlotCount += CategoryMaterial->CurrentSlot;

		NewAllSlotCount = FMath::Clamp(NewAllSlotCount, 0, CategoryALL->MaxSlot);

		if ( CategoryALL->CurrentSlot != NewAllSlotCount )
		{
			CategoryALL->CurrentSlot = NewAllSlotCount;
			CategoryALL->CreateSlots();
		}
	}

	if (CategoryALL) CategoryALL->SetInvArraySlots(AllSlots);
	if (CategoryWeapon) CategoryWeapon->SetInvArraySlots(AllSlots);
	if (CategoryFood) CategoryFood->SetInvArraySlots(AllSlots);
	if (CategoryMaterial) CategoryMaterial->SetInvArraySlots(AllSlots);

	if (CategoryALL) CategoryALL->UpdateSlots();
	if (CategoryWeapon) CategoryWeapon->UpdateSlots();
	if (CategoryFood) CategoryFood->UpdateSlots();
	if (CategoryMaterial) CategoryMaterial->UpdateSlots();
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
	if (!InvCateSwitcher || !CategoryTap) return;

	if (TapALL) TapALL->SetSelected(false);
	if (TapWeapon) TapWeapon->SetSelected(false);
	if (TapFood) TapFood->SetSelected(false);
	if (TapMaterial) TapMaterial->SetSelected(false);

	UpdateTabTextOpacity(CategoryTap);
	CategoryTap->SetSelected(true);

	if (CategoryTap == TapALL)
	{
		InvCateSwitcher->SetActiveWidgetIndex(0);
	}
	else if (CategoryTap == TapWeapon)
	{
		InvCateSwitcher->SetActiveWidgetIndex(1);
	}
	else if (CategoryTap == TapFood)
	{
		InvCateSwitcher->SetActiveWidgetIndex(2);
	}
	else if (CategoryTap == TapMaterial)
	{
		InvCateSwitcher->SetActiveWidgetIndex(3);
	}
}

void UUK_InvUI::UpdateTabTextOpacity(UUK_CategoryTap* SelectedTap)
{
	auto SetOpacity = [] (UTextBlock* Text, float Alpha)
		{
			if (!Text) return;

			FSlateColor Color = Text->GetColorAndOpacity();
			FLinearColor Linear = Color.GetSpecifiedColor();
			Linear.A = Alpha;
			Text->SetColorAndOpacity(Linear);
		};

	SetOpacity(TextBlock_ALL, SelectedTap == TapALL ? ActiveAlpha : InactiveAlpha);
	SetOpacity(TextBlock_Weapon, SelectedTap == TapWeapon ? ActiveAlpha : InactiveAlpha);
	SetOpacity(TextBlock_Food, SelectedTap == TapFood ? ActiveAlpha : InactiveAlpha);
	SetOpacity(TextBlock_Material, SelectedTap == TapMaterial ? ActiveAlpha : InactiveAlpha);
}

void UUK_InvUI::ApplyItemDataTables()
{
	if (CategoryALL)
	{
		CategoryALL->ItemDataTables = ItemDataTables;
		CategoryALL->CreateSlots();
	}

	if (CategoryWeapon)
	{
		CategoryWeapon->ItemDataTables = ItemDataTables;
		CategoryWeapon->CreateSlots();
	}

	if (CategoryFood)
	{
		CategoryFood->ItemDataTables = ItemDataTables;
		CategoryFood->CreateSlots();
	}

	if (CategoryMaterial)
	{
		CategoryMaterial->ItemDataTables = ItemDataTables;
		CategoryMaterial->CreateSlots();
	}

	if (EquipSlot1)
	{
		EquipSlot1->ItemDataTables = ItemDataTables;
		EquipSlot1->UpdateEquipSlotVisual();
	}

	if (EquipSlot2)
	{
		EquipSlot2->ItemDataTables = ItemDataTables;
		EquipSlot2->UpdateEquipSlotVisual();
	}

	if (EquipSlot3)
	{
		EquipSlot3->ItemDataTables = ItemDataTables;
		EquipSlot3->UpdateEquipSlotVisual();
	}

	if (InvComp)
	{
		OnInvCompUpdated();
	}
}