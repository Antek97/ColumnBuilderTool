# Column Builder Unreal Engine 5.4.4

## Description:

This tool is a useful asset for creating power lines (and more) in open-world games. It is supported by World Partition and is designed for level designers to easily place poles and similar structures. 

> [!NOTE]
It's recommended to avoid spawning poles at runtime due to potential compatibility issues with World Partition.

## Creation:

1. To correctly use the tool for creating poles with cables:
    - Create a `BP_ElectricalPillar_Child` blueprint that inherits from `BP_ElectricalPillar`.
    - In the parent blueprint, add a `PoleStaticMesh` component and place a `BP_PillarPoint` component where the cables should connect.
    - Assign the necessary mesh.

2. Create a `DA_PowerLine_Child` data asset that inherits from `PowerLineDataAsset`.
    - In the `PillarActorClass` field, assign your `BP_ElectricalPillar_Child` and configure the remaining options.

3. Create a `BP_PillarPoint_Child` that inherits from `BP_PillarPoint`.
    - In its `PowerLineSettings` slot, set the previously created `DA_PowerLine_Child`.

## Usage:

  -> Place the `BP_PillarPoint_Child` into the level.
  -> Set up the spline and configure the tool's options.
  -> Select the root of the `BP_PillarPoint_Child` in the level, go to the Details panel, and click `GeneratePowerLine` to create the power lines.
  -> To remove them, use `RemovePowerLines`.
  -> If you update the `DA_PowerLine_Child`, you must regenerate the `BP_ElectricalPillar_Child`. You can do this simply by clicking `GeneratePowerLine` again to refresh the blueprint in the level.

## Settings in DataAsset

### PillarSettings: 

* PillarActorClass          –   The blueprint must inherit from BP_ElectricPillar.
* bSnapToGround             –   Determines if the pillar should align with the ground.
* bGenerateVertical         –   If enabled, poles will be generated vertically.
* bSupportsWorldPartition   –   When enabled, long splines will be split into smaller segments for better compatibility with World Partition.

### WireSettings:

WireStaticMesh            –   The static mesh used for the cable appearance.
      
DeflectionWireValue       –   Controls the sagging of the cable between poles. It's calculated only once and clamped (max 1000.f) to avoid excessive bending.

### Settings:

PillarDistance            –   Minimum allowed distance between poles (cannot be less than 500) to prevent issues like freezing the editor.

## Tool Actions

### Spline Actions:

      GeneratePowerLine         –   Creates poles and cables.
      RemovePowerLines          –   Removes the poles and cables, but keeps the spline.
      Remove                    –   Removes the entire spline along with its poles.

      
