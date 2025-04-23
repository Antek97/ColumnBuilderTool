#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "PowerLine.generated.h"

class AElectricPillar;
class USplineMeshComponent;
class UPowerLineDataAsset;

UCLASS(meta = (WorldPartitionActorDesc))
class APowerLine : public AActor
{
	GENERATED_BODY()
	
public:	
	APowerLine();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerLine")
    USplineComponent* SplineComponent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options")
    UPowerLineDataAsset* PowerLineSettings;

protected:
	virtual void BeginPlay() override;

public:
    UFUNCTION(CallInEditor, Category = "PowerLine")
    void GeneratePowerLine();
    UFUNCTION(CallInEditor, Category = "PowerLine")
    void RemovePowerLines();
    UFUNCTION(CallInEditor, Category = "PowerLine")
    void Remove();

private:
    void CreatePowerLineFromSpline();

    void CreatePillarsWithConnections();
    void ConnectTwoPillars(AElectricPillar* PrevPillar, AElectricPillar* CurrPillar);

    AElectricPillar* CreatePillarAtLocation(const FVector& Location, const FRotator& Rotation);
    FRotator GetAdjustedRotation(const FRotator& OriginalRotation) const;
    FVector GetGroundSnappedLocation(const FVector& OriginalLocation, UWorld* World) const;
    AElectricPillar* SpawnPillar(UWorld* World, const FVector& Location, const FRotator& Rotation);

    void SplitAndGeneratePowerLines();

    TArray<APowerLine*> SplitSplineIntoSegments();
    void InitializeSegment(APowerLine* Segment, bool bIsLast);
    void CopySplinePointsToSegment(APowerLine* Segment, float Start, float End);
    void LinkSegments(const TArray<APowerLine*>& Segments);
    void AttachAndRegisterPillar(AElectricPillar* Pillar);

    void ConnectPillarWithPrevious(int32 CurrentPoleIndex);
    USplineMeshComponent* CreateWireSpline(const FVector& StartPosition, const FVector& EndPosition,
        const FVector& TangentStart, const FVector& TangentEnd);

private:
    UPROPERTY()
    TArray<AElectricPillar*> GeneratedPillars;
    UPROPERTY()
    TArray<USplineMeshComponent*> GeneratedWires;

    TWeakObjectPtr<APowerLine> PreviousSegment;
    TWeakObjectPtr<APowerLine> NextSegment;

    bool bIsSegment = false;
    bool bIsLastSegment = false;

    const float LengthThreshold = 20000.f;
    const float MaxSegmentLength = 30000.f;
    const float SplineSegmentStep = 1000.f;
};
