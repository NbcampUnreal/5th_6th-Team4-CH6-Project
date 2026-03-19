#include "Level/Warp/UK_WarpSubsystem.h"
#include "GameFramework/Character.h"
#include "Level/Warp/UK_WarpPointData.h"
#include "UI/OutGame/UK_Out_Loading.h"

void UUK_WarpSubsystem::RegisterWarpPoint(FName PointID, FVector Location)
{
	if ( !ActivatedWarpPoints.Contains(PointID) )
	{
		ActivatedWarpPoints.Add(PointID, Location);
		UE_LOG(LogTemp, Warning, TEXT("Warp Point Registered: %s"), *PointID.ToString());
	}
}

void UUK_WarpSubsystem::TeleportToWarpPoint(ACharacter* PlayerChar, FName PointID)
{
	if ( !PlayerChar || !ActivatedWarpPoints.Contains(PointID) ) return;

	// 클래스가 유효한지 먼저 체크
	if ( LoadingWidgetClass )
	{
		UUK_Out_Loading* LoadingWidget = CreateWidget<UUK_Out_Loading>(GetWorld(), LoadingWidgetClass);
		if ( LoadingWidget )
		{
			LoadingWidget->AddToViewport(999);
			LoadingWidget->TargetValue = 0.8f; // 이동 중 80%

			// 캐릭터 워프
			FVector TargetLoc = ActivatedWarpPoints[ PointID ];
			TargetLoc.X += 250.0f;
			PlayerChar->SetActorLocation(TargetLoc);

			// 1초 뒤에 100%로 만들어서 로딩창이 스스로 닫히게 함
			FTimerHandle WarpTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(WarpTimerHandle, [ LoadingWidget ] ()
				{
					if ( LoadingWidget )
					{
						LoadingWidget->TargetValue = 1.0f;
					}
				}, 1.0f, false);
		}
	}
	else
	{
		// 로딩창 클래스가 없으면 그냥 즉시 이동 (크래시 방지)
		PlayerChar->SetActorLocation(ActivatedWarpPoints[ PointID ] + FVector(0, 0, 100));
		UE_LOG(LogTemp, Error, TEXT("WarpSubsystem: LoadingWidgetClass가 비어있습니다!"));
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