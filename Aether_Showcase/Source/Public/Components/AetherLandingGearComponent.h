// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AetherLandingGearComponent.generated.h"

UENUM(BlueprintType)
enum class EAetherLandingGearMode : uint8
{
    Sticky  UMETA(DisplayName = "Magnetic / Sticky "),
    Rolling UMETA(DisplayName = "Rolling ")
};

UCLASS(ClassGroup = (Aether), meta = (BlueprintSpawnableComponent))
class AETHER_API UAetherLandingGearComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    // -----------------------
    // Lifecycle
    // -----------------------
    UAetherLandingGearComponent();

    // -----------------------
    // Gear State Management
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Landing Gear")
    void SetGearDeployed(bool bDeploy);

    UFUNCTION(BlueprintCallable, Category = "Aether|Landing Gear")
    void ToggleGearDeployed();

    UFUNCTION(BlueprintPure, Category = "Aether|Landing Gear")
    bool IsGearDeployed() const { return bIsDeployed; }

    // -----------------------
    // Configuration Properties
    // -----------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    bool bIsDeployed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    EAetherLandingGearMode GearMode = EAetherLandingGearMode::Rolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    float SuspensionLength = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    float SpringFrequency = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    float DampingRatio = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    float MaxSuspensionGs = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Landing Gear")
    float GroundFriction = 10.f;
};