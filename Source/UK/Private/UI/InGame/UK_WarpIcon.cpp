#include "UI/InGame/UK_WarpIcon.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Level/Warp/UK_WarpSubsystem.h"
#include "ActorComponent/UK_InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "UI/Inventory/UK_InvMain.h"

void UUK_WarpIcon::NativeConstruct()
{
	Super::NativeConstruct();

	if ( WarpButton )
	{
		WarpButton->OnClicked.AddDynamic(this, &UUK_WarpIcon::OnWarpButtonClicked);
	}

	if ( WarpInfoText )
	{
		WarpInfoText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (!OwnerInvMain)
	{
		OwnerInvMain = FindOwnerInvMain();
	}
}

UUK_InvMain* UUK_WarpIcon::FindOwnerInvMain() const
{
	return GetTypedOuter<UUK_InvMain>();
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

		FLinearColor IconTint = FLinearColor::White;
		IconTint.A = bIsActivated ? ActiveOpacity : InactiveOpacity;
		IconImage->SetColorAndOpacity(IconTint);
	}

}

// 마우스 올렸을 때 (Hover)
void UUK_WarpIcon::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if ( !WarpInfoText ) return;

	// 먼저 타워가 활성화되었는지 확인
	if ( !bIsActivated )
	{
		// 비활성 상태 메시지
		WarpInfoText->SetText(FText::FromString(TEXT("워프 타워가 아직 활성화되지 않았습니다.")));
		WarpInfoText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		WarpInfoText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		return;
	}

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	UUK_InventoryComponent* InvComp = PlayerChar ? PlayerChar->FindComponentByClass<UUK_InventoryComponent>() : nullptr;

	if ( InvComp )
	{
		// 
		if ( InvComp->GetGold() >= WarpCost )
		{
			// 성공 메시지: "?? 골드를 소비하여 워프합니다."
			FText SuccessMsg = FText::Format(
				FText::FromString(TEXT("{0} 골드를 소비하여 워프합니다.")),
				FText::AsNumber(WarpCost)
			);
			WarpInfoText->SetText(SuccessMsg);
			WarpInfoText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		else
		{
			// 실패 메시지: "골드가 부족합니다. (필요 골드: ??)"
			FText FailMsg = FText::Format(
				FText::FromString(TEXT("골드가 부족합니다. (필요 골드: {0})")),
				FText::AsNumber(WarpCost)
			);
			WarpInfoText->SetText(FailMsg);
			WarpInfoText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}

		WarpInfoText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

// 마우스 뗐을 때 (Unhover)
void UUK_WarpIcon::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	if ( WarpInfoText ) WarpInfoText->SetVisibility(ESlateVisibility::Hidden);
}

void UUK_WarpIcon::OnWarpButtonClicked()
{
	if ( !bIsActivated )
	{
		UE_LOG(LogTemp, Warning, TEXT("이 워프 타워는 아직 활성화되지 않았습니다!"));
		return;
	}

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if ( !PlayerChar ) return;

	UUK_InventoryComponent* InvComp = PlayerChar->FindComponentByClass<UUK_InventoryComponent>();
	UUK_WarpSubsystem* WarpSubsystem = GetWorld()->GetSubsystem<UUK_WarpSubsystem>();

	if ( !InvComp || !WarpSubsystem ) return;

	// subtractionGold(3000) 실행. 성공 시 0 이상의 값 반환.
	if ( InvComp->subtractionGold(WarpCost) >= 0 )
	{
		if (OwnerInvMain)
		{
			OwnerInvMain->CloseInvMain();
		}
		// 골드 차감 성공 시 세이브 및 워프 실행
		WarpSubsystem->SaveWarpData();
		WarpSubsystem->TeleportToWarpPoint(PlayerChar, WarpPointID);
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("골드가 부족하여 워프할 수 없습니다!"));
	}
}

void UUK_WarpIcon::SetOwnerInvMain(UUK_InvMain* InInvMain)
{
	OwnerInvMain = InInvMain;
}
