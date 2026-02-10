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

	if (TapALL)
	{
		TapALL->OnCategoryTap.AddDynamic(this, &UUK_InvUI::CategoryTap);
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
	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if (IsValid(CB))
	{
		BindInventoryComponent(CB->GetInventoryComponent());
	}
}

void UUK_InvUI::BindInventoryComponent(UUK_InventoryComponent* InInvComp)
{
	if (!InInvComp)
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

	if (!InvComp) return;

	TArray<FInventorySlot> AllSlots = InvComp->GetItemSlot();

	//CategoryBase에 모든 슬롯 배열 전달
	if (CategoryALL) CategoryALL->SetInvArraySlots(AllSlots);
	if (CategoryWeapon) CategoryWeapon->SetInvArraySlots(AllSlots);
	if (CategoryFood) CategoryFood->SetInvArraySlots(AllSlots);
	if (CategoryMaterial) CategoryMaterial->SetInvArraySlots(AllSlots);
}

void UUK_InvUI::CategoryTap(UUK_CategoryTap* CategoryTap)
{
	if (!InvCateSwitcher) return;

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
