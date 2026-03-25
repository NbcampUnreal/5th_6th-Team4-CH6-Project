// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGame/CreftWeaponUI/UK_Crafting.h"
#include "Character/UK_PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/Inventory/UK_MoneyWidget.h"
#include "Character/UK_CharacterBase.h"


void UUK_Crafting::NativeConstruct()
{
	Super::NativeConstruct();

	AUK_CharacterBase* CB = Cast<AUK_CharacterBase>(GetOwningPlayerPawn());
	if ( CB )
	{
		UUK_InventoryComponent* InvComp = CB->GetInventoryComponent();
		if ( InvComp )
		{
			if ( MoneyWidget )
			{
				MoneyWidget->BindInventoryComponent(InvComp);
			}
		}
	}
}

FReply UUK_Crafting::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if ( Key == EKeys::Tab )
	{
		if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
		{
			PC->OpenSettingAndCloseOtherUI();
			return FReply::Handled();
		}
	}
	if ( Key == EKeys::Escape )
	{
		if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
		{
			PC->CloseWeaponCraftingUI();
			return FReply::Handled();
		}
	}
	if ( Key == EKeys::B )
	{
		if ( AUK_PlayerController* PC = Cast<AUK_PlayerController>(GetOwningPlayer()) )
		{
			PC->Inventory_UI();
			return FReply::Handled();
		}
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}