#include "UI/InGame/UK_CheckPoint.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "UI/InGame/UK_MarkerWidget.h"


AUK_CheckPoint::AUK_CheckPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	RootComponent = Sphere;

	Sphere->SetSphereRadius(300.f);
	Sphere->SetCollisionProfileName(TEXT("Trigger"));
	
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));

	WidgetComp->SetupAttachment(RootComponent);
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetDrawSize(FVector2D(200.f, 50.f));
}

void AUK_CheckPoint::BeginPlay()
{
	Super::BeginPlay();

	if (WidgetComp)
	{
		UUserWidget* Widget = WidgetComp->GetUserWidgetObject();
		MarkerWidget = Cast<UUK_MarkerWidget>(Widget);

		if (MarkerWidget)
		{
			MarkerWidget->SetOriginLocation(GetActorLocation());
		}
	}
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AUK_CheckPoint::OnOverlapBegin);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AUK_CheckPoint::OnOverlapEnd);
}

void AUK_CheckPoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	if (MarkerWidget)
	{
		MarkerWidget->PlayFadeOut();
	}
}

void AUK_CheckPoint::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	if (MarkerWidget)
	{
		MarkerWidget->PlayFadeIn();
	}
}



