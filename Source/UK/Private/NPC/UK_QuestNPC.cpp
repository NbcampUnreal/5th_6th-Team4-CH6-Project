#include "NPC/UK_QuestNPC.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/UK_CharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AUK_QuestNPC::AUK_QuestNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	bPlayerInRange = false;
	QuestState = EQuestState::None;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(1000.f); // 마커 띄울 범위 임시 설정

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerEnter);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUK_QuestNPC::OnPlayerExit);

	QuestMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarker"));
	QuestMarker->SetupAttachment(RootComponent);
	QuestMarker->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	QuestMarker->SetVisibility(false);

	QuestID = 0;
}

void AUK_QuestNPC::BeginPlay()
{
	Super::BeginPlay();

	bPlayerInRange = false;
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

void AUK_QuestNPC::Interact_Implementation(AActor* Interactor)
{
	if ( !bPlayerInRange ) return;


	AUK_CharacterBase* Player =
		Cast<AUK_CharacterBase>(Interactor);

	if ( !Player ) return;


	switch ( QuestState )
	{
	case EQuestState::None:
		GiveQuest(Player);
		break;

	case EQuestState::InProgress:
		CheckQuest(Player);
		break;

	case EQuestState::Completed:
		GiveReward(Player);
		break;

	case EQuestState::Rewarded:
		AlreadyClear();
		break;
	}
	// 여기서 나중에
	// - 대화 UI
	// - 퀘스트 지급
	// - JSON 연동
}

void AUK_QuestNPC::GiveQuest(AUK_CharacterBase* Player)
{
	UE_LOG(LogTemp, Log, TEXT("퀘스트 시작"));

	QuestState = EQuestState::InProgress;

	//퀘스트 매니저 연동
}



void AUK_QuestNPC::CheckQuest(AUK_CharacterBase* Player)
{
	UE_LOG(LogTemp, Log, TEXT("퀘스트 진행중"));

	// 임시로 바로 완료 처리
	QuestState = EQuestState::Completed;
}



void AUK_QuestNPC::GiveReward(AUK_CharacterBase* Player)
{
	UE_LOG(LogTemp, Log, TEXT("보상 지급"));

	QuestState = EQuestState::Rewarded;

	//아이템, 경험치 지급
}



void AUK_QuestNPC::AlreadyClear()
{
	UE_LOG(LogTemp, Log, TEXT("이미 완료된 퀘스트"));
}

