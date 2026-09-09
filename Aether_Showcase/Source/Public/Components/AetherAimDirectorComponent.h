// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherAimDirectorComponent.generated.h"

class UAetherMovementComponent;

UCLASS(ClassGroup = (Aether), meta = (BlueprintSpawnableComponent))
class AETHER_API UAetherAimDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // -----------------------
    // Lifecycle
    // -----------------------
    UAetherAimDirectorComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // -----------------------
    // Aim Management
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Aim")
    void InitializeAimDirector(UAetherMovementComponent* InMovementComponent, USceneComponent* InCameraComponent, USceneComponent* InVisualMesh);

    UFUNCTION(BlueprintCallable, Category = "Aether|Aim")
    void AddAimInput(FVector2D MouseDelta);

private:
    // -----------------------
    // Internal State
    // -----------------------
    UPROPERTY()
    FQuat CurrentAimRotation;

    TWeakObjectPtr<UAetherMovementComponent> MovementComponent;
    TWeakObjectPtr<USceneComponent> CameraComponent;
};