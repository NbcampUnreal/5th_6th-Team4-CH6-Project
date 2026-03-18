#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // 데이터테이블 사용을 위해 필수
#include "UK_WarpPointData.generated.h"

USTRUCT(BlueprintType)
struct FWarpPointRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 타워 고유 ID (액터의 WarpPointID와 매칭용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warp")
	FName WarpPointID;

	// UI에 표시될 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warp")
	FText DisplayName;

	// 지도 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Warp")
	TObjectPtr<UTexture2D> MapIcon;

};
