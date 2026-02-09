#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AUK_QuestNPC::AUK_QuestNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	bPlayerInRange = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(2300.f); // 마커 띄울 범위 임시 설정

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerExit);

	QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
	QuestMarker->SetupAttachment(RootComponent);
	QuestMarker->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	QuestMarker->SetVisibility(false);
}

void AUK_QuestNPC::BeginPlay()
{
	Super::BeginPlay();

	bPlayerInRange = false;
}

void AUK_QuestNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//CheckPlayerDistance();
}

void AUK_QuestNPC::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
	AUK_CharacterBase* PlayerChar = Cast<AUK_CharacterBase>(OtherActor);
	if ( PlayerChar )
	{
		bPlayerInRange = true;

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(true);
			GetWorldTimerManager().SetTimer(MarkerTimerHandle, this, &AUK_QuestNPC::UpdateMarkerRotation, 0.03f, true);
		}

		UE_LOG(LogTemp, Log, TEXT("플레이어가 NPC 범위 안으로 들어왔습니다"));
	}
}

void AUK_QuestNPC::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* PlayerChar = Cast<AUK_CharacterBase>(OtherActor);
	if ( PlayerChar )
	{
		bPlayerInRange = false;

		if ( QuestMarker )
		{
			QuestMarker->SetVisibility(false);
			GetWorldTimerManager().ClearTimer(MarkerTimerHandle);
		}

		UE_LOG(LogTemp, Log, TEXT("플레이어가 NPC 범위에서 벗어났습니다"));
	}
}

void AUK_QuestNPC::UpdateMarkerRotation()
{
	if (!QuestMarker) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	FVector ToPlayer = PlayerChar->GetActorLocation() - QuestMarker->GetComponentLocation();
	FRotator LookAtRotation = ToPlayer.Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Yaw += 90.f;
	QuestMarker->SetWorldRotation(LookAtRotation);
}

void AUK_QuestNPC::CheckPlayerDistance() //지금은 딱히 사용하고 있지 않음 퀘스트용으로 사용될 예정
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	float Distance = FVector::Dist(PlayerChar->GetActorLocation(), GetActorLocation());

	if (Distance <= InteractionRadius)
	{
		if (!bPlayerInRange)
		{
			bPlayerInRange = true;
			UE_LOG(LogTemp, Log, TEXT("플레이어가 NPC 근처에 있습니다! 상호작용 가능."));
		}
	}
	else
	{
		if (bPlayerInRange)
		{
			bPlayerInRange = false;
			UE_LOG(LogTemp, Log, TEXT("플레이어가 NPC에서 멀어졌습니다."));
		}
	}

	if (QuestMarker)
	{
		QuestMarker->SetVisibility(true);
	}
}

void AUK_QuestNPC::Interact_Implementation(AActor* Interactor)
{

}

