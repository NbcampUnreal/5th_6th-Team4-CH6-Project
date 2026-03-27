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
	
	// 1. 반경 내 특정 채널 액터들 긁어오기 (빌드에서 훨씬 안정적)
	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Player->GetActorLocation(),
		200.0f, 
		ObjectTypes,
		AActor::StaticClass(), 
		TArray<AActor*>(),   
		OverlappingActors
	);
	UE_LOG(LogTemp, Log, TEXT("[Interact] Found %d actors around player."), OverlappingActors.Num());

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