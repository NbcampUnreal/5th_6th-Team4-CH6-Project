#include "NPC/Component/UK_QuestGuideComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UUK_QuestGuideComponent::UUK_QuestGuideComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TargetActor = nullptr;
	GuideSpline = nullptr;
}

void UUK_QuestGuideComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()) return;

	GuideSpline =
		NewObject<USplineComponent>(GetOwner(), TEXT("GuideSpline"));

	GetOwner()->AddInstanceComponent(GuideSpline);

	GuideSpline->RegisterComponent();

	GuideSpline->SetupAttachment(
		GetOwner()->GetRootComponent()
	);

	GuideSpline->SetMobility(EComponentMobility::Movable);
}

void UUK_QuestGuideComponent::SetTarget(AActor* NewTarget)
{
	if ( !NewTarget ) return;

	TargetActor = NewTarget;

	// 0.5초마다 길 갱신
	GetWorld()->GetTimerManager().SetTimer(
		GuideTimer,
		this,
		&UUK_QuestGuideComponent::UpdatePath,
		0.5f,
		true
	);
}

void UUK_QuestGuideComponent::ClearGuide()
{
	TargetActor = nullptr;

	GetWorld()->GetTimerManager().ClearTimer(GuideTimer);
	ClearSpline();
}

void UUK_QuestGuideComponent::UpdatePath()
{
	if ( !TargetActor ) return;

	AActor* Owner = GetOwner();
	if ( !Owner ) return;

	UNavigationSystemV1* NavSys =
		UNavigationSystemV1::GetCurrent(GetWorld());

	if ( !NavSys ) return;

	UNavigationPath* Path =
		NavSys->FindPathToActorSynchronously(
			GetWorld(),
			Owner->GetActorLocation(),
			TargetActor
		);

	if ( Path )
	{
		DrawPath(Path);
	}
}

void UUK_QuestGuideComponent::DrawPath(UNavigationPath* Path)
{
	if ( !Path || !GuideSpline ) return;

	ClearSpline();

	const TArray<FVector>& Points = Path->PathPoints;

	for ( const FVector& Point : Points )
	{
		FVector GroundPos = ProjectToGround(Point);

		FVector LocalPos =
			GuideSpline->GetComponentTransform().InverseTransformPosition(GroundPos);

		GuideSpline->AddSplinePoint(LocalPos, ESplineCoordinateSpace::Local);
	}

	GuideSpline->UpdateSpline();

	BuildSplineMesh();
}

void UUK_QuestGuideComponent::BuildSplineMesh()
{
	if (!GuideSpline || !GuideMesh) return;

	const float TileLength = 300.f;

	const float SplineLength = GuideSpline->GetSplineLength();

	for ( float Dist = 0; Dist < SplineLength; Dist += TileLength )
	{
		float StartDist = Dist;
		float EndDist = FMath::Min(Dist + TileLength, SplineLength);

		USplineMeshComponent* Mesh =
			NewObject<USplineMeshComponent>(GetOwner());

		if (!Mesh) continue;

		GetOwner()->AddInstanceComponent(Mesh);

		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->AttachToComponent(GuideSpline,FAttachmentTransformRules::KeepRelativeTransform);
		Mesh->RegisterComponent();
		Mesh->SetStaticMesh(GuideMesh);
		Mesh->SetForwardAxis(ESplineMeshAxis::X);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		FVector StartPos = GuideSpline->GetLocationAtDistanceAlongSpline(
			StartDist,
			ESplineCoordinateSpace::Local);

		FVector StartTan = GuideSpline->GetTangentAtDistanceAlongSpline(
			StartDist,
			ESplineCoordinateSpace::Local);

		FVector EndPos = GuideSpline->GetLocationAtDistanceAlongSpline(
			EndDist,
			ESplineCoordinateSpace::Local);

		FVector EndTan = GuideSpline->GetTangentAtDistanceAlongSpline(
			EndDist,
			ESplineCoordinateSpace::Local);

		Mesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
		Mesh->SetStartScale(FVector2D(0.7f, 0.7f));
		Mesh->SetEndScale(FVector2D(0.7f, 0.7f));

		MeshSegments.Add(Mesh);
	}
}

void UUK_QuestGuideComponent::ClearSpline()
{
	if (!GuideSpline) return;

	GuideSpline->ClearSplinePoints();

	for (auto Mesh : MeshSegments)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	MeshSegments.Empty();
}

FVector UUK_QuestGuideComponent::ProjectToGround(const FVector& Point)
{
	FHitResult Hit;

	FVector Start = Point + FVector(0, 0, 500.f);
	FVector End = Point - FVector(0, 0, 5000.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Params))
	{
		return Hit.Location + FVector(0, 0, 1.f);
	}

	return Point;
}