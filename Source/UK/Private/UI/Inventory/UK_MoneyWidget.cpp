#include "UI/Inventory/UK_MoneyWidget.h"
#include "Components/TextBlock.h"
#include "ActorComponent/UK_InventoryComponent.h"

void UUK_MoneyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateMoney(0);
}

void UUK_MoneyWidget::BindInventoryComponent(UUK_InventoryComponent* InInvComp)
{
	if (!InInvComp) return;

	InvComp = InInvComp;

	InvComp->OnChangedGold.AddDynamic(this, &UUK_MoneyWidget::UpdateMoney);

	UpdateMoney(InvComp->GetGold());
}

void UUK_MoneyWidget::UpdateMoney(int32 NewGold)
{
	if (MoneyText)
	{
		MoneyText->SetText(FText::AsNumber(NewGold));
	}
}