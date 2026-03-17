#include "UI/InGame/UK_FloatingDamageActor.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveFloat.h"
#include "TimerManager.h"
#include "UI/InGame/UK_FloatingDamage.h"

AUK_FloatingDamageActor::AUK_FloatingDamageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(Root);

	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetTwoSided(true);
	WidgetComponent->SetDrawSize(FVector2D(200.f, 100.f));
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	WidgetComponent->SetRelativeScale3D(FVector(1.f));
}

void AUK_FloatingDamageActor::BeginPlay()
{
	Super::BeginPlay();

	if ( WidgetComponent )
	{
		WidgetComponent->InitWidget();
	}

	InitWidget();
	InitMoveData();
	StartMoveTimer();
}

void AUK_FloatingDamageActor::SetDamageAmount(float InDamage)
{
	ActorDamageAmount = InDamage;

	UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageActor] SetDamageAmount = %.1f"), InDamage);

	if ( FloatingDamageWidget )
	{
		FloatingDamageWidget->SetDamageAmount(ActorDamageAmount);
	}
}

void AUK_FloatingDamageActor::InitWidget()
{
	if ( !WidgetComponent )
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageActor] WidgetComponent is null"));
		return;
	}

	UUserWidget* UserWidget = WidgetComponent->GetUserWidgetObject();
	if ( !UserWidget )
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageActor] UserWidget is null"));
		return;
	}

	FloatingDamageWidget = Cast<UUK_FloatingDamage>(UserWidget);
	if ( !FloatingDamageWidget )
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageActor] Cast to UUK_FloatingDamage failed"));
		return;
	}

	FloatingDamageWidget->SetDamageAmount(ActorDamageAmount);

	UE_LOG(LogTemp, Warning, TEXT("[FloatingDamageActor] InitWidget Success / Damage = %.1f"), ActorDamageAmount);
}

void AUK_FloatingDamageActor::InitMoveData()
{
	StartLocation = GetActorLocation();

	const float RandX = FMath::FRandRange(MinOffsetX, MaxOffsetX);
	const float RandY = FMath::FRandRange(MinOffsetY, MaxOffsetY);
	const float RandZ = FMath::FRandRange(MinOffsetZ, MaxOffsetZ);

	EndLocation = StartLocation + FVector(RandX, RandY, RandZ);
	ElapsedTime = 0.f;
}

void AUK_FloatingDamageActor::StartMoveTimer()
{
	if ( MoveDuration <= 0.f )
	{
		FinishMove();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&AUK_FloatingDamageActor::UpdateMove,
		TimerInterval,
		true
	);
}

void AUK_FloatingDamageActor::UpdateMove()
{
	ElapsedTime += TimerInterval;

	float Alpha = ElapsedTime / MoveDuration;
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	float CurveAlpha = Alpha;
	if ( FloatCurve )
	{
		CurveAlpha = FloatCurve->GetFloatValue(Alpha);
	}

	const FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, CurveAlpha);
	SetActorLocation(NewLocation);

	if ( Alpha >= 1.f )
	{
		FinishMove();
	}
}

void AUK_FloatingDamageActor::FinishMove()
{
	GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	Destroy();
}