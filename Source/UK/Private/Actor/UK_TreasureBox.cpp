#include "Actor/UK_TreasureBox.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "Character/UK_CharacterBase.h"
#include "NPC/Component/UK_InteractionComponent.h"
#include "Systems/Data/UK_InGameSave.h"
#include "Actor/Subsystem/UK_BoxManagerSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AUK_TreasureBox::AUK_TreasureBox()
{
	PrimaryActorTick.bCanEverTick = false;
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	RootComponent = BoxMesh;
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(200.f);
	
	MinGold = 1000;
	MaxGold = 3000;
	MinItemCount = 3;
	MaxItemCount = 5;
	bIsOpened = false;
	
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_TreasureBox::OnOverlapBegin);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_TreasureBox::OnOverlapEnd);
}

void AUK_TreasureBox::BeginPlay()
{
	Super::BeginPlay();
	
	UUK_BoxManagerSubsystem* BoxManager = GetWorld()->GetSubsystem<UUK_BoxManagerSubsystem>();
	if (BoxManager)
	{
		if (!BoxManager->ShouldSpawnChest(FName(*GetName())))
		{
			Destroy();
			return;
		}
	}
}

void AUK_TreasureBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);
	if (Player)
	{
		UUK_InteractionComponent* InteractionComp = Player->FindComponentByClass<UUK_InteractionComponent>();
		if (InteractionComp)
		{
			InteractionComp->SetNearActor(this);
		}
	}
}

void AUK_TreasureBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* Player = Cast<AUK_CharacterBase>(OtherActor);
	if (Player)
	{
		UUK_InteractionComponent* InteractionComp = Player->FindComponentByClass<UUK_InteractionComponent>();
		if (InteractionComp)
		{
			InteractionComp->ClearNearActor();
		}
	}
}

void AUK_TreasureBox::TryOpen(AActor* InteractingPlayer)
{ 
	if (!InteractingPlayer || bIsOpened) return;
	
	if (OpenSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
	}
	
	if (OpenVFX)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f);
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			OpenVFX,
			SpawnLocation,
			GetActorRotation(),
			FVector(15.0f),
			true
			);
	}
	
	UUK_InventoryComponent* InvComp = InteractingPlayer->FindComponentByClass<UUK_InventoryComponent>();
	if (!InvComp) return;
	
	int32 RandomGold = FMath::RandRange(MinGold, MaxGold);
	InvComp->AddGold(RandomGold);
 
	TArray<FName> RowNames = RewardTable->GetRowNames();
	RowNames.Remove(FName("Item_All_Gold"));
	
	if (RowNames.Num() > 0)
	{
		int32 RandomItemCount = FMath::RandRange(MinItemCount, MaxItemCount);
		
		for (int32 i = 0; i < RandomItemCount; ++i)
		{
			FName SelectedID = RowNames[FMath::RandRange(0, RowNames.Num() - 1)];
			
			InvComp->AddItem(SelectedID,1);
		}
	}
	
	if (UUK_BoxManagerSubsystem* ChestManager = GetWorld()->GetSubsystem<UUK_BoxManagerSubsystem>())
	{
		ChestManager->RegisterOpenedChest(FName(*GetName()));
	}
	bIsOpened = true;
	Destroy();
}


