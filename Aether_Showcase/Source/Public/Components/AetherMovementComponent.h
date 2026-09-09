// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "NetworkPredictionComponent.h"
#include "Simulations/AetherSimulation.h"
#include "Engine/HitResult.h"
#include "AetherMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EAetherControlScheme : uint8
{
    DirectAxis  UMETA(DisplayName = "Direct Input"),
    MouseAim    UMETA(DisplayName = "Follow target")
};

class UAetherLandingGearComponent;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class AETHER_API UAetherMovementComponent : public UNetworkPredictionComponent
{
    GENERATED_BODY()

public:
    // -----------------------
    // Lifecycle
    // -----------------------
    UAetherMovementComponent();
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
#if WITH_EDITOR
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#endif

    // -----------------------
    // Component Configuration
    // -----------------------
    UPROPERTY(Transient)
    USceneComponent* UpdatedComponent;

    UFUNCTION(BlueprintCallable, Category = "Aether|Components")
    void SetVisualComponent(USceneComponent* InVisualComponent);

    // -----------------------
    // Input Handling
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetForwardInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetRightInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetUpInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetPitchInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetYawInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetRollInput(float Value);

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void ToggleCoupledMode();

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void ToggleLandingGear();

    UFUNCTION(BlueprintCallable, Category = "Aether|Input")
    void SetAimDirection(FVector InAimDirection);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Control")
    EAetherControlScheme CurrentControlScheme = EAetherControlScheme::MouseAim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Control")
    bool bIsCoupledMode = true;

    // -----------------------
    // Gravity Management
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Gravity")
    void AddGravitySource(UAetherGravityComponent* GravitySource);

    UFUNCTION(BlueprintCallable, Category = "Aether|Gravity")
    void RemoveGravitySource(UAetherGravityComponent* GravitySource);

    // -----------------------
    // Landing Gear System
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Landing Gear")
    void RegisterLandingGear(UAetherLandingGearComponent* Gear);

    void CalculateLandingGearForces(const FAetherSyncState* Sync, const FAetherAuxState* Aux, float DeltaTime, FVector& OutLinearForce, FVector& OutAngularAcceleration) const;

    // -----------------------
    // Mechanics & Simulation
    // -----------------------
    FVector SweepSimulation(FVector Start, FVector DesiredEnd, FQuat Rotation, FHitResult& OutHit) const;

    UFUNCTION(BlueprintCallable, Category = "Aether|Movement")
    void TeleportTo(FVector NewLocation, FRotator NewRotation);

    // -----------------------
    // Network Prediction Proxy (NPP) Hooks
    // -----------------------
    void InitializeSimulationState(FAetherSyncState* Sync, FAetherAuxState* Aux);
    void ProduceInput(const int32 DeltaTimeMS, FAetherInputCmd* Cmd);
    void FinalizeFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux);
    void FinalizeSmoothingFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux);
    void RestoreFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux);

    // -----------------------
    // Statistics & State Getters
    // -----------------------
    UFUNCTION(BlueprintCallable, Category = "Aether|Stats")
    void RefreshNetworkedStats();

    const FAetherSyncState* GetSyncState() const;
    const FAetherAuxState* GetAuxState() const;

    UFUNCTION(BlueprintPure, Category = "Aether|UI")
    FRotator GetMouseTargetRelativeRotation() const;

    // -----------------------
    // Core Movement Stats
    // -----------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float Mass = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float MaxForwardThrust = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float MaxReverseThrust = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float MaxStrafeThrust = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float MaxSpeed = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float LinearDampingBase = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float DiveSpeedMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float PitchRate = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float YawRate = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float RollRate = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aether|Stats")
    float RollInfluence = 1.0f;

protected:
    virtual void InitializeNetworkPredictionProxy() override;

    UPROPERTY(BlueprintReadWrite, Category = "Aether|Input")
    bool bInputLandingGear = true;

private:
    // -----------------------
    // Internal Variables
    // -----------------------
    FVector RawLinearInput;
    FVector RawAngularInput;
    FVector CurrentAimDirection = FVector::ForwardVector;
    TUniquePtr<FAetherSimulation> ActiveSimulation;

    UPROPERTY()
    USceneComponent* VisualComponent;

    UPROPERTY()
    TArray<UAetherGravityComponent*> ActiveGravitySources;

    UPROPERTY()
    TArray<UAetherLandingGearComponent*> ActiveLandingGears;
};