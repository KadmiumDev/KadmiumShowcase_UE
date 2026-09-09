// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AetherGravityComponent.generated.h"

UENUM(BlueprintType)
enum class EAetherGravityType : uint8
{
    Spherical   UMETA(DisplayName = "Planet (Spherical Falloff)"),
    Directional UMETA(DisplayName = "Hangar (Directional)")
};

UCLASS(ClassGroup = (Aether), meta = (BlueprintSpawnableComponent))
class AETHER_API UAetherGravityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // -----------------------
    // Lifecycle
    // -----------------------
    UAetherGravityComponent();

    // -----------------------
    // Gravity Logic
    // -----------------------
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Aether|Gravity")
    FVector GetGravityForceAtLocation(FVector TargetLocation) const;

    // -----------------------
    // Configuration Properties
    // -----------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity|System")
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity|System")
    FGameplayTagContainer GravityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity")
    EAetherGravityType GravityType = EAetherGravityType::Spherical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity")
    float MaxGravityStrength = 980.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity")
    float AtmosphereDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity|Spherical")
    float InnerRadius = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity|Spherical")
    float OuterRadius = 15000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Gravity|Directional")
    FVector LocalGravityDirection = FVector(0.f, 0.f, -1.f);
};