#include "Level/Warp/UK_WarpSubsystem.h"
#include "GameFramework/Character.h"
#include "Level/Warp/UK_WarpPointData.h"
#include "UI/OutGame/UK_Out_Loading.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/Data/UK_InGameSave.h"

const FString WARP_SAVE_SLOT = TEXT("UK_InGameSave");

void UUK_WarpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 서브시스템이 생성될 때 저장된 데이터를 자동으로 불러옴
	LoadWarpData();
}

void UUK_WarpSubsystem::SaveWarpData()
{
	UUK_InGameSave* SaveInstance = nullptr;

	// 기존 세이브가 있다면 먼저 불러와서 다른 데이터(인벤토리 등)를 보존함
	if ( UGameplayStatics::DoesSaveGameExist(WARP_SAVE_SLOT, 0) )
	{
		SaveInstance = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(WARP_SAVE_SLOT, 0));
	}

	// 세이브가 없었다면 새로 생성
	if ( !SaveInstance )
	{
		SaveInstance = Cast<UUK_InGameSave>(UGameplayStatics::CreateSaveGameObject(UUK_InGameSave::StaticClass()));
	}

	if ( SaveInstance )
	{
		// 워프 목록만 업데이트 (나머지 데이터는 유지됨)
		ActivatedWarpPoints.GetKeys(SaveInstance->ActivatedWarpIDs);

		// 통합 슬롯에 저장
		UGameplayStatics::SaveGameToSlot(SaveInstance, WARP_SAVE_SLOT, 0);
		UE_LOG(LogTemp, Warning, TEXT("Warp Data Saved to %s"), *WARP_SAVE_SLOT);
	}
}

void UUK_WarpSubsystem::LoadWarpData()
{
	// 저장된 파일이 있는지 확인 후 로드
	if ( UGameplayStatics::DoesSaveGameExist(WARP_SAVE_SLOT, 0) )
	{
		UUK_InGameSave* LoadInstance = Cast<UUK_InGameSave>(UGameplayStatics::LoadGameFromSlot(WARP_SAVE_SLOT, 0));
		if ( LoadInstance )
		{
			// 기존 맵을 비우고 새로 채움
			ActivatedWarpPoints.Empty();

			for ( const FName& ID : LoadInstance->ActivatedWarpIDs )
			{
				// 데이터테이블에서 실시간 좌표를 가져와서 맵 구성
				FWarpPointRow Row = GetWarpRowByID(ID);

				// DataTable이 아직 로드 전이라도 ID는 등록해둬야 나중에 UI가 활성화
				FVector Loc = Row.WarpPointID.IsNone() ? FVector::ZeroVector : Row.WorldLocation;
				ActivatedWarpPoints.Add(ID, Loc);
			}
			UE_LOG(LogTemp, Warning, TEXT("Warp Data Loaded! Total: %d"), ActivatedWarpPoints.Num());
		}
	}
}

void UUK_WarpSubsystem::RegisterWarpPoint(FName PointID, FVector Location)
{
	if ( !ActivatedWarpPoints.Contains(PointID) )
	{
		ActivatedWarpPoints.Add(PointID, Location);
		UE_LOG(LogTemp, Warning, TEXT("Warp Point Registered: %s"), *PointID.ToString());

		// 등록 즉시 저장 실행
		SaveWarpData();
	}
}

void UUK_WarpSubsystem::TeleportToWarpPoint(ACharacter* PlayerChar, FName PointID)
{
	// 활성화된 포인트인지 먼저 체크
	if ( !PlayerChar || !ActivatedWarpPoints.Contains(PointID) )
	{
		UE_LOG(LogTemp, Error, TEXT("Warp: 비활성화된 포인트이거나 캐릭터가 없습니다."));
		return;
	}

	// 맵에 저장된 좌표 대신, 데이터테이블에서 '진짜' 좌표를 직접 가져옴
	FWarpPointRow RowData = GetWarpRowByID(PointID);

	if ( RowData.WarpPointID.IsNone() )
	{
		UE_LOG(LogTemp, Error, TEXT("Warp: 데이터테이블에서 ID(%s)를 찾을 수 없습니다!"), *PointID.ToString());
		return;
	}

	// 로딩창 띄우기 로직
	if ( LoadingWidgetClass )
	{
		UUK_Out_Loading* LoadingWidget = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
		if ( LoadingWidget )
		{
			LoadingWidget->AddToViewport(999);
			LoadingWidget->TargetValue = 0.8f;

			// 실제 워프 좌표 적용 
			FVector TargetLoc = RowData.WorldLocation; // 데이터테이블의 좌표 사용
			TargetLoc.X += 250.0f; 

			PlayerChar->SetActorLocation(TargetLoc);

			FTimerHandle WarpTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(WarpTimerHandle, [ LoadingWidget ] ()
				{
					if ( LoadingWidget ) LoadingWidget->TargetValue = 1.0f;
				}, 1.0f, false);
		}
	}
}

TArray<FName> UUK_WarpSubsystem::GetActivatedPointIDs() const
{
	TArray<FName> OutIDs;
	ActivatedWarpPoints.GetKeys(OutIDs);
	return OutIDs;
}

FWarpPointRow UUK_WarpSubsystem::GetWarpRowByID(FName PointID) const
{
	if ( WarpDataTable )
	{
		// 데이터테이블의 RowName을 기준으로 직접 찾음.
		FWarpPointRow* FoundRow = WarpDataTable->FindRow<FWarpPointRow>(PointID, TEXT("WarpContext"));
		if ( FoundRow )
		{
			return *FoundRow;
		}
	}
	return FWarpPointRow();
}