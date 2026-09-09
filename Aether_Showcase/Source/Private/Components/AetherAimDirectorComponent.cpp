// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#include "Components/AetherAimDirectorComponent.h"
#include "Components/AetherMovementComponent.h"
#include "Components/SceneComponent.h"

// -----------------------
// Lifecycle
// -----------------------

UAetherAimDirectorComponent::UAetherAimDirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UAetherAimDirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FVector CamForward = CurrentAimRotation.GetForwardVector();
    float VerticalDot = FMath::Abs(FVector::DotProduct(CamForward, FVector::UpVector));

    if (VerticalDot < 0.98f)
    {
        FVector FlatRight = FVector::CrossProduct(FVector::UpVector, CamForward).GetSafeNormal();
        if (!FlatRight.IsNearlyZero())
        {
            FQuat LeveledRotation = FRotationMatrix::MakeFromXY(CamForward, FlatRight).ToQuat();
            float PitchAlpha = 1.0f - VerticalDot;
            float SmoothSpeed = 4.0f * PitchAlpha;
            float InterpAlpha = 1.0f - FMath::Exp(-SmoothSpeed * DeltaTime);
            CurrentAimRotation = FQuat::Slerp(CurrentAimRotation, LeveledRotation, InterpAlpha);
        }
    }

    if (CameraComponent.IsValid())
    {
        CameraComponent->SetWorldRotation(CurrentAimRotation);
    }

    if (MovementComponent.IsValid())
    {
        MovementComponent->SetAimDirection(CurrentAimRotation.GetForwardVector());
    }
}

// -----------------------
// Aim Management
// -----------------------

void UAetherAimDirectorComponent::InitializeAimDirector(UAetherMovementComponent* InMovementComponent, USceneComponent* InCameraComponent, USceneComponent* InVisualMesh)
{
    MovementComponent = InMovementComponent;
    CameraComponent = InCameraComponent;

    if (InVisualMesh)
    {
        FVector MeshForward = InVisualMesh->GetForwardVector();
        CurrentAimRotation = FRotationMatrix::MakeFromXZ(MeshForward, FVector::UpVector).ToQuat();
    }
    else if (CameraComponent.IsValid())
    {
        CurrentAimRotation = CameraComponent->GetComponentQuat();
    }
}

void UAetherAimDirectorComponent::AddAimInput(FVector2D MouseDelta)
{
    if (FMath::Abs(MouseDelta.X) > 50.f || FMath::Abs(MouseDelta.Y) > 50.f)
    {
        return;
    }

    if (MouseDelta.Y != 0.f)
    {
        FQuat PitchQuat(FVector::RightVector, FMath::DegreesToRadians(MouseDelta.Y));
        CurrentAimRotation = (CurrentAimRotation * PitchQuat).GetNormalized();
    }

    if (MouseDelta.X != 0.f)
    {
        FQuat YawQuat(FVector::UpVector, FMath::DegreesToRadians(MouseDelta.X));
        CurrentAimRotation = (YawQuat * CurrentAimRotation).GetNormalized();
    }
}