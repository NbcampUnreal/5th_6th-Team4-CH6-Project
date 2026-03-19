#include "Level/Warp/UK_WarpSubsystem.h"
#include "GameFramework/Character.h"
#include "Level/Warp/UK_WarpPointData.h"

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
	if ( PlayerChar && ActivatedWarpPoints.Contains(PointID) )
	{
		FVector TargetLoc = ActivatedWarpPoints[ PointID ];
		TargetLoc.X += 250.0f;
    
		PlayerChar->SetActorLocation(TargetLoc);
		UE_LOG(LogTemp, Warning, TEXT("Teleported to: %s"), *PointID.ToString());
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