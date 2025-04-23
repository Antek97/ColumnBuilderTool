#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElectricPillar.generated.h"

UCLASS(meta = (WorldPartitionActorDesc))
class AElectricPillar : public AActor
{
	GENERATED_BODY()
	
public:	
	AElectricPillar();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Setings")
	UStaticMeshComponent* PoleStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USceneComponent*> PillarPoints;

	virtual void OnConstruction(const FTransform& Transform) override;
protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	bool GetPillarPoints();
};
