#include "NPC/UK_Shop_NPC.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Blueprint/UserWidget.h"

AUK_Shop_NPC::AUK_Shop_NPC()
{
	ShopName = FString("Store");
}

void AUK_Shop_NPC::Interact(AActor* Interactor)
{
	if (!Interactor) return;
	
	AUK_CharacterBase* PlayerCharacter = Cast<AUK_CharacterBase>(Interactor);
	if (PlayerCharacter)
	{
		AUK_PlayerController* PlayerController = Cast<AUK_PlayerController>(PlayerCharacter->GetController());
		if (PlayerController && ShopWidetClass)
		{
			PlayerController->ShowShopUI(ShopWidetClass);
		}
	}
}
