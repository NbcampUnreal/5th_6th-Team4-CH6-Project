#include "UI/InGame/UK_MarkerWidget.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UUK_MarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	GetWorld()->GetTimerManager().SetTimer(
		DistanceCheckTimer,
		this,
		&UUK_MarkerWidget::CheckDistance,
		0.1f,
		true
	);
}

void UUK_MarkerWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimer);
	}
}

void UUK_MarkerWidget::SetOriginLocation(FVector InLocation) // 마커 텍스트 위치 초기화
{
	OriginLocation = InLocation;
}

void UUK_MarkerWidget::CheckDistance()
{
	if (!PlayerPawn || !Location) return;
	
	float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), OriginLocation);
	int32 Meter = FMath::RoundToInt(Distance / 100.f);
	
	Location->SetText(FText::FromString(FString::Printf(TEXT("%dm"), Meter)));
}

void UUK_MarkerWidget::PlayFadeIn()
{
	if (FadeInAnim)
	{ 
		PlayAnimation(FadeInAnim);
		// EndOverlap 애님
	}
}

void UUK_MarkerWidget::PlayFadeOut()
{
	if (FadeOutAnim)
	{
		PlayAnimation(FadeOutAnim);
		// BeginOverlap 애님
	}
}
