#include "PowerLine.h"
#include "PowerLineDataAsset.h"
#include "ElectricPillar.h"
#include "Components/SplineMeshComponent.h"

APowerLine::APowerLine()
{
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    RootComponent = SplineComponent;
    SplineComponent->SetClosedLoop(false);

	PrimaryActorTick.bCanEverTick = false;

    SetIsSpatiallyLoaded(true);
}

void APowerLine::BeginPlay()
{
	Super::BeginPlay();
}

void APowerLine::GeneratePowerLine()
{
    if (!SplineComponent || !PowerLineSettings) return;

    float SplineLength = SplineComponent->GetSplineLength();

    if (!PowerLineSettings->PillarSettings.bSupportsWorldPartition)
    {
        bIsSegment = false;
        CreatePowerLineFromSpline();
        return;
    }

	if (bIsSegment || SplineLength <= LengthThreshold)
	{
		bIsSegment = false;
		CreatePowerLineFromSpline();
	}
	else
	{
        bIsSegment = true;
		SplitAndGeneratePowerLines();
	}
}

void APowerLine::RemovePowerLines()
{
    TArray<USceneComponent*> SceneComponents;
    SplineComponent->GetChildrenComponents(true, SceneComponents);
    for (auto& SceneComponent : SceneComponents)
    {
        if (SceneComponent)
        {
            SceneComponent->DestroyComponent();

        }
    }
    GeneratedPillars.Empty();
    GeneratedWires.Empty();
}

void APowerLine::Remove()
{
    RemovePowerLines();
    Destroy();
}

void APowerLine::CreatePowerLineFromSpline()
{
    if (!PowerLineSettings) return;

    if (GeneratedPillars.Num() > 0)
    {
        RemovePowerLines();
    }

    CreatePillarsWithConnections();
}

void APowerLine::CreatePillarsWithConnections()
{
	if (!SplineComponent || !PowerLineSettings) return;

	float SplineLength = SplineComponent->GetSplineLength();
	int32 NumPillars = FMath::Max(2, FMath::CeilToInt(SplineLength / PowerLineSettings->PillarDistance));

	int32 StartIndex = 0;
	int32 EndIndex = NumPillars;

	if (bIsSegment && PreviousSegment.Get())
	{
		StartIndex = 1;
	}
	if (bIsSegment && NextSegment.Get())
	{
		EndIndex -= 1;
	}

	for (int32 PilarIndex = StartIndex; PilarIndex < EndIndex; PilarIndex++)
	{
        float DistanceAlongSpline;

        if (bIsSegment)
        {
            float SegmentSteps = EndIndex - StartIndex;
            DistanceAlongSpline = (PilarIndex - StartIndex) * SplineLength / SegmentSteps;
        }
        else 
        {
            DistanceAlongSpline = PilarIndex * SplineLength / (NumPillars - 1);
        }

		FVector PillarLocation = SplineComponent->GetLocationAtDistanceAlongSpline(
			DistanceAlongSpline, ESplineCoordinateSpace::World);
		FRotator PillarRotation = SplineComponent->GetRotationAtDistanceAlongSpline(
			DistanceAlongSpline, ESplineCoordinateSpace::World);

		AElectricPillar* NewPillar = CreatePillarAtLocation(PillarLocation, PillarRotation);

		if (NewPillar && (PilarIndex > 0 || (PilarIndex == 0 && !PreviousSegment.Get())))
		{
			ConnectPillarWithPrevious(GeneratedPillars.Num() - 1);
		}
	}

	if (bIsSegment && PreviousSegment.Get() && PreviousSegment->GeneratedPillars.Num() > 0 && GeneratedPillars.Num() > 0)
	{
		ConnectTwoPillars(PreviousSegment->GeneratedPillars.Last(), GeneratedPillars[0]);
	}
}

void APowerLine::ConnectTwoPillars(AElectricPillar* PrevPillar, AElectricPillar* CurrPillar)
{
    if (!PrevPillar || !CurrPillar || !PowerLineSettings || !PowerLineSettings->WireSettings.WireStaticMesh) return;

    for (int32 i = 0; i < CurrPillar->PillarPoints.Num(); i++)
    {
        FVector StartPosition = PrevPillar->PillarPoints[i]->GetComponentLocation();
        FVector EndPosition = CurrPillar->PillarPoints[i]->GetComponentLocation();

        FVector MidPosition = (StartPosition + EndPosition) * 0.5f;
        MidPosition.Z -= PowerLineSettings->WireSettings.DeflectionWireValue;

        FVector TangentStart = (MidPosition - StartPosition) * 2.f;
        FVector TangentEnd = (EndPosition - MidPosition) * 2.f;

        USplineMeshComponent* SplineMesh = CreateWireSpline(StartPosition, EndPosition, TangentStart, TangentEnd);

        GeneratedWires.Add(SplineMesh);
    }
}

AElectricPillar* APowerLine::CreatePillarAtLocation(const FVector& Location, const FRotator& Rotation)
{
    if (!PowerLineSettings || !PowerLineSettings->PillarSettings.PillarActorClass) return nullptr;

    UWorld* World = GetWorld();
    if (!ensure(World) || !SplineComponent) return nullptr;

    FRotator UprightRotation = GetAdjustedRotation(Rotation);
    FVector NewLocation = GetGroundSnappedLocation(Location, World);

    AElectricPillar* NewPillar = SpawnPillar(World, NewLocation, UprightRotation);

    if (!NewPillar) return nullptr;

    AttachAndRegisterPillar(NewPillar);

    return NewPillar;
}

FRotator APowerLine::GetAdjustedRotation(const FRotator& OriginalRotation) const
{
    return PowerLineSettings->PillarSettings.bGenerateVertical
        ? FRotator(0.f, OriginalRotation.Yaw, 0.f)
        : OriginalRotation;
}

FVector APowerLine::GetGroundSnappedLocation(const FVector& OriginalLocation, UWorld* World) const
{
    if (!PowerLineSettings->PillarSettings.bSnapToGround || !PowerLineSettings || !World) return OriginalLocation;

    FHitResult HitResult;
    FVector Start = OriginalLocation + FVector(0.f, 0.f, 8000.f);
    FVector End = OriginalLocation - FVector(0.f, 0.f, 8000.f);

    FCollisionQueryParams TraceParams(FName(TEXT("GroundTrace")), true, this);
    TraceParams.bReturnPhysicalMaterial = false;
    TraceParams.bTraceComplex = true;

    if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams))
    {
        return HitResult.Location;
    }

    return OriginalLocation;
}

AElectricPillar* APowerLine::SpawnPillar(UWorld* World, const FVector& Location, const FRotator& Rotation)
{
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    return World->SpawnActor<AElectricPillar>(
        PowerLineSettings->PillarSettings.PillarActorClass,
        Location,
        Rotation,
        SpawnParams
    );
}

void APowerLine::SplitAndGeneratePowerLines()
{
    if (!PowerLineSettings || !SplineComponent) return;

    TArray<APowerLine*> Segments = SplitSplineIntoSegments();
    LinkSegments(Segments);

    for (APowerLine* Segment : Segments)
    {
        Segment->CreatePowerLineFromSpline();
    }

    Destroy();
}

TArray<APowerLine*> APowerLine::SplitSplineIntoSegments()
{
    const float TotalLength = SplineComponent->GetSplineLength();
    int32 NumSegments = FMath::CeilToInt(TotalLength / MaxSegmentLength);

    TArray<APowerLine*> Segments;

    UWorld* World = GetWorld();
    if (!World) return Segments;

    for (int32 i = 0; i < NumSegments; ++i)
    {
        float Start = i * MaxSegmentLength;
        float End = FMath::Min((i + 1) * MaxSegmentLength, TotalLength);

        FVector SpawnLocation = SplineComponent->GetLocationAtDistanceAlongSpline(Start, ESplineCoordinateSpace::World);
        APowerLine* Segment = World->SpawnActor<APowerLine>(GetClass(), SpawnLocation, FRotator::ZeroRotator);

        if (!Segment) continue;

        InitializeSegment(Segment, i == NumSegments - 1);
        CopySplinePointsToSegment(Segment, Start, End);

        Segments.Add(Segment);
    }

    return Segments;
}

void APowerLine::InitializeSegment(APowerLine * Segment, bool bIsLast)
{
    Segment->PowerLineSettings = PowerLineSettings;
    Segment->bIsSegment = true;
    Segment->bIsLastSegment = bIsLast;

    USplineComponent* NewSpline = NewObject<USplineComponent>(Segment, TEXT("SplineComponent"));
    Segment->SetRootComponent(NewSpline);
    NewSpline->RegisterComponent();
    NewSpline->SetClosedLoop(false);
    NewSpline->SetMobility(EComponentMobility::Movable);

    Segment->SplineComponent = NewSpline;
    Segment->SetIsSpatiallyLoaded(true);
}

void APowerLine::CopySplinePointsToSegment(APowerLine * Segment, float Start, float End)
{
    USplineComponent* NewSpline = Segment->SplineComponent;

    for (float Distance = Start; Distance <= End; Distance += SplineSegmentStep)
    {
        FVector Location = SplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FVector Tangent = SplineComponent->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FRotator Rotation = SplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

        int32 PointIndex = NewSpline->GetNumberOfSplinePoints();

        NewSpline->AddSplinePoint(Location, ESplineCoordinateSpace::World);
        NewSpline->SetTangentAtSplinePoint(PointIndex, Tangent, ESplineCoordinateSpace::World);
        NewSpline->SetRotationAtSplinePoint(PointIndex, Rotation, ESplineCoordinateSpace::World);
    }
}

void APowerLine::LinkSegments(const TArray<APowerLine*>&Segments)
{
    for (int32 i = 1; i < Segments.Num(); ++i)
    {
        Segments[i]->PreviousSegment = Segments[i - 1];
        Segments[i - 1]->NextSegment = Segments[i];
    }
}

void APowerLine::AttachAndRegisterPillar(AElectricPillar* Pillar)
{
    if (!Pillar) return;

    Pillar->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
    GeneratedPillars.Add(Pillar);
}

void APowerLine::ConnectPillarWithPrevious(int32 CurrentIndex)
{
    if (CurrentIndex <= 0 || CurrentIndex >= GeneratedPillars.Num()) return;

    AElectricPillar* PrevPillar = GeneratedPillars[CurrentIndex - 1];
    AElectricPillar* CurrPillar = GeneratedPillars[CurrentIndex];

    if (!PrevPillar || !CurrPillar || !PowerLineSettings || !PowerLineSettings->WireSettings.WireStaticMesh) return;

    for (int32 i = 0; i < CurrPillar->PillarPoints.Num(); i++)
    {
        FVector StartPosition = PrevPillar->PillarPoints[i]->GetComponentLocation();
        FVector EndPosition = CurrPillar->PillarPoints[i]->GetComponentLocation();

        FVector MidPosition = (StartPosition + EndPosition) * 0.5f;
        MidPosition.Z -= PowerLineSettings->WireSettings.DeflectionWireValue;

        FVector TangentStart = (MidPosition - StartPosition) * 2.f;
        FVector TangentEnd = (EndPosition - MidPosition) * 2.f;

        USplineMeshComponent* SplineMesh = CreateWireSpline (
            StartPosition, 
            EndPosition, 
            TangentStart, 
            TangentEnd 
        );
        GeneratedWires.Add(SplineMesh);
    }
}

USplineMeshComponent* APowerLine::CreateWireSpline(const FVector& StartPosition, const FVector& EndPosition,
    const FVector& TangentStart, const FVector& TangentEnd)
{
    USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);

    SplineMesh->SetStaticMesh(PowerLineSettings->WireSettings.WireStaticMesh.LoadSynchronous());
    SplineMesh->SetMobility(EComponentMobility::Movable);
    SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;

    SplineMesh->RegisterComponentWithWorld(GetWorld());
    AddInstanceComponent(SplineMesh);
    SplineMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
    SplineMesh->SetStartAndEnd(StartPosition, TangentStart, EndPosition, TangentEnd, true);

    return SplineMesh;
}
