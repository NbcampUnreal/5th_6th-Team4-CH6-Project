#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Level/Warp/UK_WarpPointData.h"
#include "UK_WarpSubsystem.generated.h"

class UUK_Out_Loading;

UCLASS(BlueprintType)

class UK_API UUK_WarpSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// 외부(MainMap)에서 로딩창 클래스를 넣어줄 변수
		UPROPERTY(BlueprintReadWrite, Category = "Warp")
	TSubclassOf<UUK_Out_Loading> LoadingWidgetClass;

	// 타워가 활성화될 때 호출하여 리스트에 추가
	UFUNCTION(BlueprintCallable, Category = "Warp")
	void RegisterWarpPoint(FName PointID, FVector Location);

	// 특정 포인트로 캐릭터 이동 (텔레포트)
	UFUNCTION(BlueprintCallable, Category = "Warp")
	void TeleportToWarpPoint(ACharacter* PlayerChar, FName PointID);

	// 활성화된 모든 포인트 가져오기 (UI용)
	UFUNCTION(BlueprintPure, Category = "Warp")
	TArray<FName> GetActivatedPointIDs() const;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warp")
	TObjectPtr<UDataTable> WarpDataTable;

	// ID로 해당 행의 데이터를 찾아오는 함수
	UFUNCTION(BlueprintPure, Category = "Warp")
	FWarpPointRow GetWarpRowByID(FName PointID) const;

private:
	// 활성화된 워프 포인트 데이터 (ID, 위치)
	UPROPERTY()
	TMap<FName, FVector> ActivatedWarpPoints;
};