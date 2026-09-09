// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#include "Components/AetherGravityComponent.h"
#include "GameFramework/Actor.h"

// -----------------------
// Lifecycle
// -----------------------

UAetherGravityComponent::UAetherGravityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// -----------------------
// Gravity Logic
// -----------------------

FVector UAetherGravityComponent::GetGravityForceAtLocation(FVector TargetLocation) const
{
    AActor* Owner = GetOwner();
    if (!Owner) return FVector::ZeroVector;

    if (GravityType == EAetherGravityType::Spherical)
    {
        FVector CenterLocation = Owner->GetActorLocation();
        FVector DirectionToCenter = (CenterLocation - TargetLocation).GetSafeNormal();
        float Distance = FVector::Distance(CenterLocation, TargetLocation);
        float Alpha = 1.0f - FMath::Clamp((Distance - InnerRadius) / FMath::Max(1.0f, OuterRadius - InnerRadius), 0.0f, 1.0f);
        return DirectionToCenter * (MaxGravityStrength * Alpha);
    }
    else if (GravityType == EAetherGravityType::Directional)
    {
        FVector WorldGravityDir = Owner->GetActorQuat().RotateVector(LocalGravityDirection).GetSafeNormal();
        return WorldGravityDir * MaxGravityStrength;
    }

    return FVector::ZeroVector;
}