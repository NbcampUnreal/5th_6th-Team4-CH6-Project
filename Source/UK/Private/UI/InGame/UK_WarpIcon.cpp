#include "UI/InGame/UK_WarpIcon.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Level/Warp/UK_WarpSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void UUK_WarpIcon::NativeConstruct()
{
	Super::NativeConstruct();

	if ( WarpButton )
	{
		WarpButton->OnClicked.AddDynamic(this, &UUK_WarpIcon::OnWarpButtonClicked);
	}
}

void UUK_WarpIcon::InitWarpIcon(FName InPointID, bool bInActivated)
{
	WarpPointID = InPointID;
	bIsActivated = bInActivated;

	UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>();
	if ( !WarpSubsystem ) return;

	// 데이터테이블에서 해당 타워의 아이콘 정보를 가져와 적용
	FWarpPointRow Data = WarpSubsystem->GetWarpRowByID(WarpPointID);
	if ( IconImage && Data.MapIcon )
	{
		IconImage->SetBrushFromTexture(Data.MapIcon);

		// 활성화 상태에 따라 아이콘 색상 변경 
		IconImage->SetColorAndOpacity(bIsActivated ? ActiveColor : InactiveColor);
	}

}

void UUK_WarpIcon::OnWarpButtonClicked()
{
	if ( !bIsActivated )
	{
		UE_LOG(LogTemp, Warning, TEXT("이 워프 타워는 아직 활성화되지 않았습니다!"));
		return;
	}

	if ( UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>() )
	{
		ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if ( PlayerChar )
		{
			WarpSubsystem->TeleportToWarpPoint(PlayerChar, WarpPointID);

		}
	}
}