#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Level/Warp/UK_WarpPointData.h" 
#include "UK_WarpIcon.generated.h"

class UButton;
class UImage;

UCLASS()
class UK_API UUK_WarpIcon : public UUserWidget
{
	GENERATED_BODY()

public:
	// 지도가 생성할 때 데이터를 넣어주는 초기화 함수
	void InitWarpIcon(FName InPointID, bool bInActivated);

	// Data 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warp")
	FName WarpPointID;

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 시 호출될 함수
	UFUNCTION()
	void OnWarpButtonClicked();

protected:
	// UI Binding (블루프린트 위젯 이름과 일치해야 함) 
	UPROPERTY(meta = ( BindWidget ))
	TObjectPtr<UButton> WarpButton;

	UPROPERTY(meta = ( BindWidget ))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warp")
	bool bIsActivated = false;

	// 활성화/비활성화 시 색상 (에디터 상세창에서 수정 가능)
	UPROPERTY(EditAnywhere, Category = "Warp|Visual")
	FLinearColor ActiveColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, Category = "Warp|Visual")
	FLinearColor InactiveColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);
};