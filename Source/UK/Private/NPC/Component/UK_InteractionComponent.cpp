#include "NPC/Component/UK_InteractionComponent.h"
#include "NPC/UK_QuestNPC.h"
#include "NPC/UK_Shop_NPC.h"
#include "GameFramework/Actor.h"
#include "Actor/UK_TreasureBox.h"
#include "Character/UK_CharacterBase.h"
#include "Character/UK_PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

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
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(GetOwner());
	if (!Player) return;
	
	TArray<AActor*> OverlappingActors;
	Player->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (AUK_QuestNPC* QuestNPC = Cast<AUK_QuestNPC>(Actor))
		{
			QuestNPC->Interact(Player); 
			return;
		}
		if (AUK_Shop_NPC* ShopNPC = Cast<AUK_Shop_NPC>(Actor))
		{
			ShopNPC->Interact(Player);
			return;
		}
		if (AUK_TreasureBox* TBox = Cast<AUK_TreasureBox>(Actor))
		{
			TBox->TryOpen(Player);
			return;
		}
	}
	
	if (NearActor)
	{
		if (AUK_QuestNPC* NPC = Cast<AUK_QuestNPC>(NearActor))
		{
			NPC->Interact(Player);
			return;
		}
		if (AUK_Shop_NPC* ShopNPC = Cast<AUK_Shop_NPC>(NearActor))
		{
			ShopNPC->Interact(Player);
			return;
		}
		if (AUK_TreasureBox* TreasureBox = Cast<AUK_TreasureBox>(NearActor))
		{
			TreasureBox->TryOpen(Player);
			return;
		}
	}
}