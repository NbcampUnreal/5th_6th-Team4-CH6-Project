#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/UK_QuestNPC.h"
#include "NPC/UK_Shop_NPC.h"
#include "GameFramework/Actor.h"
#include "Actor/UK_TreasureBox.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"

UUK_InteractionComponent::UUK_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	NearActor = nullptr;
}

void UUK_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UUK_InteractionComponent::SetNearActor(AActor* NewActor)
{
	if ( !NewActor ) return;
	NearActor = NewActor;

	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if ( !Player ) return;

}

void UUK_InteractionComponent::ClearNearActor()
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());

	if ( Player )
	{
		AUK_PlayerController* PlayerCtl = Cast<AUK_PlayerController>(Player->GetController());

		if ( PlayerCtl )
		{
			PlayerCtl->HideQuestUI();
			PlayerCtl->HideShopUI();
		}
	}

	NearActor = nullptr;
}

void UUK_InteractionComponent::TryInteract()
{
	if (!NearActor)	return;


	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if (!Player) return;

	AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(NearActor);
	if (NPC)
	{
		NPC->Interact(Player);
		return;
	}

	AUK_Shop_NPC* ShopNPC = Cast<AUK_Shop_NPC>(NearActor);
	if (ShopNPC)
	{
		ShopNPC->Interact(Player);
		return;
	}
	AUK_TreasureBox* TreasureBox = Cast<AUK_TreasureBox>(NearActor);
	if (TreasureBox)
	{	
		TreasureBox->TryOpen(Player);
		return;
	}
}