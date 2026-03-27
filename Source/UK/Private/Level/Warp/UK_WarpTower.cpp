#include "Level/Warp/UK_WarpTower.h"
#include "Components/SphereComponent.h"
#include "Character/UK_CharacterBase.h" 
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/AttibuteSet/UK_PlayerStatusAttributeSet.h"
#include "Quest/UKQuestManagerSubsystem.h"
#include "Level/Warp/UK_WarpSubsystem.h"
#include "Components/ChildActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Dialogue/UKQuestUIManagerSubsystem.h"
#include "UI/InGame/UK_CheckPoint.h"

AUK_WarpTower::AUK_WarpTower()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(200.f);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootComponent);

	MarkerComp = CreateDefaultSubobject<UChildActorComponent>(TEXT("MarkerComp"));
	MarkerComp->SetupAttachment(RootComponent);
	MarkerComp->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
}

void AUK_WarpTower::BeginPlay()
{
	Super::BeginPlay();

	// 델리게이트 바인딩
	CollisionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &AUK_WarpTower::OnOverlapBegin);
	CollisionSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &AUK_WarpTower::OnOverlapEnd);

	// 게임 시작 시 서브시스템에서 활성화 여부 확인
		if ( UWorld* World = GetWorld() )
		{
			if ( UUK_WarpSubsystem* WarpSubsystem = World->GetSubsystem<UUK_WarpSubsystem>() )
			{
				// 서브시스템에 이미 내 ID가 등록되어 있을 시
				if ( WarpSubsystem->GetActivatedPointIDs().Contains(WarpPointID) )
				{
					bIsActivated = true;

					// 블루프린트에서 만든 시각 효과 로직 실행
					OnTowerActivated();

					UE_LOG(LogTemp, Log, TEXT("WarpTower [%s]: Already Activated in Load Data."), *WarpPointID.ToString());
				}
			}
		}

		// === Warp Marker Terminal Setup ===
		if ( SelectMarker )
		{
			MarkerComp->SetChildActorClass(SelectMarker);
			MarkerComp->CreateChildActor();

			WarpMarker = Cast<AUK_CheckPoint>(MarkerComp->GetChildActor());
			if ( WarpMarker )
			{
				MarkerComp->SetVisibility(false, true);
				WarpMarker->SetActorHiddenInGame(true);
			}
		}

		if ( UGameInstance* GI = GetGameInstance() )
		{
			if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
			{
				QuestSys->OnQuestMarkerResetRequested.AddDynamic(this, &AUK_WarpTower::HandleQuestMarkerResetRequested);
				QuestSys->OnQuestMarkerRouteResolved.AddDynamic(this, &AUK_WarpTower::HandleQuestMarkerRouteResolved);

				UE_LOG(LogTemp, Warning,
					TEXT("[QuestMarker] Bound Warp terminal broadcasts | WarpTower=%s | WarpPointID=%s"),
					*GetName(),
					*WarpPointID.ToString());
			}
		}

		RefreshWarpMarker();
}

void AUK_WarpTower::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
		{
			QuestSys->OnQuestMarkerResetRequested.RemoveDynamic(this, &AUK_WarpTower::HandleQuestMarkerResetRequested);
			QuestSys->OnQuestMarkerRouteResolved.RemoveDynamic(this, &AUK_WarpTower::HandleQuestMarkerRouteResolved);
		}
	}

	GetWorldTimerManager().ClearTimer(MarkerTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AUK_WarpTower::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// 캐릭터인지 확인
	AUK_CharacterBase* TargetCharacter = Cast<AUK_CharacterBase>(OtherActor);
	if ( !TargetCharacter ) return;

	bPlayerInRange = true;
	HideMarkerInternal();

	// 최초 활성화 로직
	if ( !bIsActivated )
	{
		bIsActivated = true;
		OnTowerActivated();

		if ( UWorld* World = GetWorld() )
		{
			if ( UUK_WarpSubsystem* WarpSubsystem = World->GetSubsystem<UUK_WarpSubsystem>() )
			{
				WarpSubsystem->RegisterWarpPoint(WarpPointID, GetActorLocation());
			}
			
			// 임시 퀘스트 권한 부여
			if ( UGameInstance* GI = GetGameInstance() )
			{
				if ( UUKQuestManagerSubsystem* QuestSys = GI->GetSubsystem<UUKQuestManagerSubsystem>() )
				{
					QuestSys->EmitQuestEvent(FName(TEXT("QuestEvent.Custom.WarpUnlocked")));
					UE_LOG(LogTemp, Warning, TEXT("[Warp][Quest] Emit WarpUnlocked. Actor=%s"), *GetName());
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Warp Tower Activated & Registered: %s"), *WarpPointID.ToString());
	}

	// 체력/마나 회복 로직 
	if ( UAbilitySystemComponent* ASC = TargetCharacter->GetAbilitySystemComponent() )
	{
		// 최대체력, 마나 가져오기
		const float MaxHP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxHealthAttribute());
		const float MaxMP = ASC->GetNumericAttribute(UUK_PlayerStatusAttributeSet::GetMaxMpAttribute());

		// 현재 값을 최대치로 즉시 변경
		ASC->SetNumericAttributeBase(UUK_PlayerStatusAttributeSet::GetHealthAttribute(), MaxHP);
		ASC->SetNumericAttributeBase(UUK_PlayerStatusAttributeSet::GetCurrentMpAttribute(), MaxMP);

		UE_LOG(LogTemp, Log, TEXT("WarpTower: Full Recovery Success (HP: %.f, MP: %.f)"), MaxHP, MaxMP);
	}
}

void AUK_WarpTower::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUK_CharacterBase* TargetCharacter = Cast<AUK_CharacterBase>(OtherActor);
	if ( !TargetCharacter ) return;

	bPlayerInRange = false;
	RefreshWarpMarker();
}

void AUK_WarpTower::UpdateMarkerRotation()
{
	if ( !WarpMarker )
	{
		return;
	}

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if ( !PlayerChar )
	{
		return;
	}

	FVector ToPlayer = PlayerChar->GetActorLocation() - WarpMarker->GetActorLocation();
	FRotator LookAtRotation = ToPlayer.Rotation();
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Yaw += 90.f;

	WarpMarker->SetActorRotation(LookAtRotation);
}

void AUK_WarpTower::HideMarkerInternal()
{
	if ( !MarkerComp || !WarpMarker )
	{
		return;
	}

	MarkerComp->SetVisibility(false, true);
	WarpMarker->SetActorHiddenInGame(true);
	GetWorldTimerManager().ClearTimer(MarkerTimerHandle);
}

void AUK_WarpTower::ShowMarkerInternal(EUKQuestMarkerState MarkerState)
{
	if ( !MarkerComp || !WarpMarker )
	{
		return;
	}

	if ( MarkerState == EUKQuestMarkerState::Hidden )
	{
		HideMarkerInternal();
		return;
	}

	MarkerComp->SetVisibility(true, true);
	WarpMarker->SetActorHiddenInGame(false);

	GetWorldTimerManager().SetTimer(
		MarkerTimerHandle,
		this,
		&AUK_WarpTower::UpdateMarkerRotation,
		0.03f,
		true
	);
}

void AUK_WarpTower::HandleQuestMarkerResetRequested()
{
	HideMarkerInternal();
}

void AUK_WarpTower::HandleQuestMarkerRouteResolved(FName QuestId, EUKQuestMarkerTargetType TargetType, FName TargetId)
{
	UE_LOG(LogTemp, Warning,
		TEXT("[QuestMarker][Warp Terminal] RouteResolved | WarpTower=%s WarpPointID=%s QuestId=%s TargetType=%d TargetId=%s"),
		*GetName(),
		*WarpPointID.ToString(),
		*QuestId.ToString(),
		static_cast< int32 >( TargetType ),
		*TargetId.ToString());

	if ( TargetType != EUKQuestMarkerTargetType::Warp )
	{
		return;
	}

	if ( TargetId.IsNone() || TargetId != WarpPointID )
	{
		return;
	}

	if ( bPlayerInRange )
	{
		HideMarkerInternal();
		return;
	}

	if ( UGameInstance* GI = GetGameInstance() )
	{
		if ( UUKQuestUIManagerSubsystem* QuestUIManager = GI->GetSubsystem<UUKQuestUIManagerSubsystem>() )
		{
			const EUKQuestMarkerState MarkerState = QuestUIManager->GetQuestMarkerState(QuestId);
			ShowMarkerInternal(MarkerState);
		}
	}
}

void AUK_WarpTower::RefreshWarpMarker()
{
	if ( !MarkerComp || !WarpMarker )
	{
		return;
	}

	if ( bPlayerInRange )
	{
		HideMarkerInternal();
		return;
	}

	// 현재 단계에서는 WarpTower가 담당할 수 있는 퀘스트 후보 목록이 없으므로,
	// 로컬 단독 판단 대신 방송 기반을 중심으로 운용
	HideMarkerInternal();
}