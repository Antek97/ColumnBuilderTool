#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PowerLineDataAsset.generated.h"

class AElectricPillar;

USTRUCT(BlueprintType)
struct FPowerPillarSettings
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "PowerLine")
	TSubclassOf<AElectricPillar> PillarActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options")
	bool bSnapToGround = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options")
	bool bGenerateVertical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options")
	bool bSupportsWorldPartition = false;
};

USTRUCT(BlueprintType)
struct FPowerWireSettings
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine")
	TSoftObjectPtr<UStaticMesh> WireStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options", meta = (ClampMax = "1000.0"))
	float DeflectionWireValue = 200.f;
};

UCLASS()
class UPowerLineDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Pillars")
    FPowerPillarSettings PillarSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Wires")
    FPowerWireSettings WireSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerLine|Options", meta = (ClampMin = "500.0")) 
    float PillarDistance = 1000.0f;
};
