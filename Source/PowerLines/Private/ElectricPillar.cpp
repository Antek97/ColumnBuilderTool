#include "ElectricPillar.h"
#include "Components/SplineMeshComponent.h"

AElectricPillar::AElectricPillar()
{
	PoleStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SplineComponent"));

	PrimaryActorTick.bCanEverTick = false;
	bAlwaysRelevant = false;

	SetIsSpatiallyLoaded(true);
}

bool AElectricPillar::GetPillarPoints()
{
	PillarPoints.Empty();
	TArray<USceneComponent*> SceneComponents;

	PoleStaticMesh->GetChildrenComponents(true, SceneComponents);

	for (USceneComponent* Comp : SceneComponents)
	{
		if (IsValid(Comp))
		{
			PillarPoints.Add(Comp);
		}
	}
	return true;
}

void AElectricPillar::OnConstruction(const FTransform& Transform)
{
	GetPillarPoints();
}

void AElectricPillar::BeginPlay()
{
	Super::BeginPlay();
	
}
