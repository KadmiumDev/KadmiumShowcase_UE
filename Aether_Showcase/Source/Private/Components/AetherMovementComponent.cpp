// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
// PUBLIC SHOWCASE VERSION - PROPRIETARY LOGIC REMOVED

#include "Components/AetherMovementComponent.h"
#include "Components/AetherGravityComponent.h"
#include "Components/AetherLandingGearComponent.h"
#include "NetworkPredictionProxyInit.h"
#include "NetworkPredictionProxyWrite.h"
#include "NetworkPredictionModelDefRegistry.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/HitResult.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
static TAutoConsoleVariable<int32> CVarAetherDebug(
    TEXT("Aether.ToggleDebug"),
    0,
    TEXT("Draws 3D floating text and lines for Aether Movement.\n0: Off, 1: On"),
    ECVF_Cheat);
#endif

NP_MODEL_REGISTER(FAetherModelDef);

// -----------------------
// Lifecycle
// -----------------------

UAetherMovementComponent::UAetherMovementComponent()
{
    bWantsInitializeComponent = true;
    bAutoActivate = true;
    SetIsReplicatedByDefault(true);

#if WITH_EDITOR
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
#else
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
#endif
}

void UAetherMovementComponent::InitializeComponent()
{
    UpdatedComponent = GetOwner()->GetRootComponent();
    Super::InitializeComponent();
}

void UAetherMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        FVector StartLoc = Owner->GetActorLocation();
        FQuat StartRot = Owner->GetActorQuat();

        auto SnapState = [StartLoc, StartRot](FAetherSyncState& Sync)
            {
                Sync.Location = StartLoc;
                Sync.Rotation = StartRot;
                Sync.LinearVelocity = FVector::ZeroVector;
            };

        NetworkPredictionProxy.WriteSyncState<FAetherSyncState>(SnapState, "BeginPlay_Snap");
        NetworkPredictionProxy.WritePresentationSyncState<FAetherSyncState>(SnapState, "BeginPlay_Pres");
        NetworkPredictionProxy.WritePrevPresentationSyncState<FAetherSyncState>(SnapState, "BeginPlay_PrevPres");

        TArray<UAetherLandingGearComponent*> FoundGears;
        Owner->GetComponents<UAetherLandingGearComponent>(FoundGears);
        for (UAetherLandingGearComponent* Gear : FoundGears)
        {
            RegisterLandingGear(Gear);
        }
    }

    if (VisualComponent)
    {
        VisualComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
}

#if WITH_EDITOR
void UAetherMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CVarAetherDebug.GetValueOnGameThread() > 0 && GetOwner())
    {
        const FAetherSyncState* Sync = NetworkPredictionProxy.ReadSyncState<FAetherSyncState>();
        const FAetherAuxState* Aux = NetworkPredictionProxy.ReadAuxState<FAetherAuxState>();
        const FAetherInputCmd* Cmd = NetworkPredictionProxy.ReadInputCmd<FAetherInputCmd>();

        if (!Sync || !Aux) return;

        FVector ActorLoc = GetOwner()->GetActorLocation();
        FVector NPLoc = Sync->Location;
        FString PosDebug = FString::Printf(TEXT("ACTOR LOCATION: %s\nNPP LOCATION:   %s"), *ActorLoc.ToString(), *NPLoc.ToString());

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1337, 0.f, FColor::Yellow, PosDebug, true, FVector2D(1.5f, 1.5f));
        }

        FVector StartLoc = GetOwner()->GetActorLocation();
        FString RoleStr = GetOwner()->HasAuthority() ?
            TEXT("SERVER") :
            (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy ? TEXT("CLIENT (Controlling)") : TEXT("CLIENT (Ghost)"));

        FString DebugText = FString::Printf(TEXT("=== AETHER MOVEMENT [%s] ===\n"), *RoleStr);
        DebugText += FString::Printf(TEXT("Speed: %.0f / %.0f\n"), Sync->LinearVelocity.Size(), Aux->MaxSpeed);

        if (Cmd)
        {
            DebugText += FString::Printf(TEXT("Input Lin: X=%.2f Y=%.2f Z=%.2f\n"), Cmd->ForwardCmd, Cmd->RightCmd, Cmd->UpCmd);
            DebugText += FString::Printf(TEXT("Input Ang: P=%.2f Y=%.2f R=%.2f\n"), Cmd->PitchCmd, Cmd->YawCmd, Cmd->RollCmd);
            if (CurrentControlScheme == EAetherControlScheme::MouseAim && !Cmd->AimDirection.IsNearlyZero())
            {
                FVector TraceEnd = StartLoc + (Cmd->AimDirection * 5000.f);
                DrawDebugLine(GetWorld(), StartLoc, TraceEnd, FColor::Red, false, -1.f, 0, 2.f);
                DrawDebugSphere(GetWorld(), TraceEnd, 50.f, 8, FColor::Orange, false, -1.f, 0, 2.f);
            }
        }

        DrawDebugString(GetWorld(), StartLoc + FVector(0, 0, 200.f), DebugText, nullptr, FColor::Cyan, 0.f, true, 1.2f);
        if (!Sync->LinearVelocity.IsNearlyZero())
        {
            DrawDebugDirectionalArrow(GetWorld(), StartLoc, StartLoc + (Sync->LinearVelocity * 0.5f), 100.f, FColor::Green, false, -1.f, 0, 5.f);
        }
    }
}
#endif

// -----------------------
// Component Configuration
// -----------------------

void UAetherMovementComponent::SetVisualComponent(USceneComponent* InVisualComponent)
{
    VisualComponent = InVisualComponent;
    if (VisualComponent && UpdatedComponent)
    {
        VisualComponent->SetWorldLocation(UpdatedComponent->GetComponentLocation());
        VisualComponent->SetWorldRotation(UpdatedComponent->GetComponentRotation());
    }
}

// -----------------------
// Input Handling
// -----------------------

void UAetherMovementComponent::SetForwardInput(float Value) { RawLinearInput.X = Value; }
void UAetherMovementComponent::SetRightInput(float Value) { RawLinearInput.Y = Value; }
void UAetherMovementComponent::SetUpInput(float Value) { RawLinearInput.Z = Value; }
void UAetherMovementComponent::SetRollInput(float Value) { RawAngularInput.X = Value; }
void UAetherMovementComponent::SetPitchInput(float Value) { RawAngularInput.Y = Value; }
void UAetherMovementComponent::SetYawInput(float Value) { RawAngularInput.Z = Value; }

void UAetherMovementComponent::ToggleCoupledMode()
{
    bIsCoupledMode = !bIsCoupledMode;
}

void UAetherMovementComponent::ToggleLandingGear()
{
    bInputLandingGear = !bInputLandingGear;
}

void UAetherMovementComponent::SetAimDirection(FVector InAimDirection)
{
    CurrentAimDirection = InAimDirection;
}

// -----------------------
// Gravity Management
// -----------------------

void UAetherMovementComponent::AddGravitySource(UAetherGravityComponent* GravitySource)
{
    if (GravitySource && !ActiveGravitySources.Contains(GravitySource))
    {
        ActiveGravitySources.Add(GravitySource);
    }
}

void UAetherMovementComponent::RemoveGravitySource(UAetherGravityComponent* GravitySource)
{
    if (GravitySource)
    {
        ActiveGravitySources.RemoveSingleSwap(GravitySource);
    }
}

// -----------------------
// Landing Gear System
// -----------------------

void UAetherMovementComponent::RegisterLandingGear(UAetherLandingGearComponent* Gear)
{
    if (Gear && !ActiveLandingGears.Contains(Gear))
    {
        ActiveLandingGears.Add(Gear);
    }
}

void UAetherMovementComponent::CalculateLandingGearForces(const FAetherSyncState* Sync, const FAetherAuxState* Aux, float DeltaTime, FVector& OutLinearForce, FVector& OutAngularAcceleration) const
{
    // [PROPRIETARY LOGIC REMOVED]
    // The commercial version performs line traces per active gear, calculating:
    // 1. Compression ratio & non-linear spring stiffness (cubic progressive multiplier).
    // 2. Axis velocity damping curves & G-Force clamping limits.
    // 3. Lateral and cross ground friction based on landing gear mode (Rolling vs Sticky).
    // 4. Lever arm cross products to generate angular torque acceleration.

    OutLinearForce = FVector::ZeroVector;
    OutAngularAcceleration = FVector::ZeroVector;
}

// -----------------------
// Mechanics & Simulation
// -----------------------

FVector UAetherMovementComponent::SweepSimulation(FVector Start, FVector DesiredEnd, FQuat Rotation, FHitResult& OutHit) const
{
    // [PROPRIETARY LOGIC REMOVED]
    // The commercial version executes multi-sweep component tracing, evaluating
    // blocking hits, initial penetration depth extraction, and impact normal offsets.

    return DesiredEnd;
}

void UAetherMovementComponent::TeleportTo(FVector NewLocation, FRotator NewRotation)
{
    if (UpdatedComponent)
    {
        UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    if (VisualComponent)
    {
        VisualComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    ActiveGravitySources.Empty();
    if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(UpdatedComponent))
    {
        PrimComp->UpdateOverlaps();
        TArray<AActor*> OverlappingActors;
        PrimComp->GetOverlappingActors(OverlappingActors);

        for (AActor* OverlapActor : OverlappingActors)
        {
            if (UAetherGravityComponent* GravSource = OverlapActor->FindComponentByClass<UAetherGravityComponent>())
            {
                ActiveGravitySources.AddUnique(GravSource);
            }
        }
    }

    auto SnapState = [&](FAetherSyncState& Sync)
        {
            Sync.Location = NewLocation;
            Sync.Rotation = NewRotation.Quaternion();
            Sync.LinearVelocity = FVector::ZeroVector;
        };

    NetworkPredictionProxy.WriteSyncState<FAetherSyncState>(SnapState, "Teleport");
    NetworkPredictionProxy.WritePresentationSyncState<FAetherSyncState>(SnapState, "Teleport_Pres");
    NetworkPredictionProxy.WritePrevPresentationSyncState<FAetherSyncState>(SnapState, "Teleport_PrevPres");
}

// -----------------------
// Network Prediction Proxy (NPP) Hooks
// -----------------------

void UAetherMovementComponent::InitializeNetworkPredictionProxy()
{
    ActiveSimulation = MakeUnique<FAetherSimulation>();
    ActiveSimulation->Driver = this;
    NetworkPredictionProxy.Init<FAetherModelDef>(GetWorld(), GetReplicationProxies(), ActiveSimulation.Get(), this);
}

void UAetherMovementComponent::InitializeSimulationState(FAetherSyncState* Sync, FAetherAuxState* Aux)
{
    if (AActor* Owner = GetOwner())
    {
        Sync->Location = Owner->GetActorLocation();
        Sync->Rotation = Owner->GetActorQuat();
        Sync->LinearVelocity = FVector::ZeroVector;
    }

    if (Aux)
    {
        Aux->Mass = Mass;
        Aux->MaxForwardThrust = MaxForwardThrust;
        Aux->MaxReverseThrust = MaxReverseThrust;
        Aux->MaxStrafeThrust = MaxStrafeThrust;
        Aux->MaxSpeed = MaxSpeed;
        Aux->LinearDampingBase = LinearDampingBase;
        Aux->PitchRate = PitchRate;
        Aux->YawRate = YawRate;
        Aux->RollRate = RollRate;
        Aux->RollInfluence = RollInfluence;
    }
}

void UAetherMovementComponent::ProduceInput(const int32 DeltaTimeMS, FAetherInputCmd* Cmd)
{
    FVector ClampedLinear = RawLinearInput.GetClampedToMaxSize(1.0f);
    Cmd->ForwardCmd = ClampedLinear.X;
    Cmd->RightCmd = ClampedLinear.Y;
    Cmd->UpCmd = ClampedLinear.Z;
    Cmd->bIsCoupledMode = this->bIsCoupledMode;
    Cmd->bWantsLandingGear = this->bInputLandingGear;

    if (CurrentControlScheme == EAetherControlScheme::DirectAxis)
    {
        Cmd->PitchCmd = FMath::Clamp(RawAngularInput.Y, -1.f, 1.f);
        Cmd->YawCmd = FMath::Clamp(RawAngularInput.Z, -1.f, 1.f);
        Cmd->RollCmd = FMath::Clamp(RawAngularInput.X, -1.f, 1.f);
        Cmd->AimDirection = FVector::ZeroVector;
    }
    else if (CurrentControlScheme == EAetherControlScheme::MouseAim)
    {
        Cmd->AimDirection = CurrentAimDirection;
        Cmd->PitchCmd = FMath::Clamp(RawAngularInput.Y, -1.f, 1.f);
        Cmd->YawCmd = FMath::Clamp(RawAngularInput.Z, -1.f, 1.f);
        Cmd->RollCmd = FMath::Clamp(RawAngularInput.X, -1.f, 1.f);
    }

    Cmd->GravityForce = FVector::ZeroVector;
    Cmd->EnvironmentDensity = 1.0f;
}

void UAetherMovementComponent::FinalizeFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux)
{
    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorLocationAndRotation(Sync->Location, Sync->Rotation);
    }

    for (UAetherLandingGearComponent* Gear : ActiveLandingGears)
    {
        if (Gear) Gear->SetGearDeployed(Sync->bLandingGearDeployed);
    }
}

void UAetherMovementComponent::RestoreFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux)
{
    if (AActor* Owner = GetOwner())
        Owner->SetActorLocationAndRotation(Sync->Location, Sync->Rotation);
}

void UAetherMovementComponent::FinalizeSmoothingFrame(const FAetherSyncState* Sync, const FAetherAuxState* Aux)
{
    if (VisualComponent)
    {
        VisualComponent->SetWorldLocationAndRotation(Sync->Location, Sync->Rotation);
    }

    for (UAetherLandingGearComponent* Gear : ActiveLandingGears)
    {
        if (Gear) Gear->SetGearDeployed(Sync->bLandingGearDeployed);
    }
}

// -----------------------
// Statistics & State Getters
// -----------------------


// ---------------------------------------------------------------------------------
// Shameless plug - Flux Framework 
// (BindStatToProperty) > TargetComponent->GetClass()->FindPropertyByName(PropertyName)
// (NotifyStatChanged) > PropertyPtr->ContainerPtrToValuePtr<float>(TargetComp)
// 
// Synchronizes movement parameters with external systems (e.g., FluxStatComponent).
// When Flux modifies stats via C++ Reflection, this pushes the updated values 
// into Network Prediction's FAetherAuxState for client/server reconciliation.
// 
// ---------------------------------------------------------------------------------

void UAetherMovementComponent::RefreshNetworkedStats()
{
    if (!GetOwner()->HasAuthority())
    {
        return;
    }

    const FAetherAuxState* CurrentAux = NetworkPredictionProxy.ReadAuxState<FAetherAuxState>();
    if (!CurrentAux) return;

    if (CurrentAux->Mass != Mass ||
        CurrentAux->MaxForwardThrust != MaxForwardThrust ||
        CurrentAux->MaxReverseThrust != MaxReverseThrust ||
        CurrentAux->MaxStrafeThrust != MaxStrafeThrust ||
        CurrentAux->MaxSpeed != MaxSpeed ||
        CurrentAux->LinearDampingBase != LinearDampingBase ||
        CurrentAux->PitchRate != PitchRate ||
        CurrentAux->YawRate != YawRate ||
        CurrentAux->RollRate != RollRate ||
        CurrentAux->RollInfluence != RollInfluence ||
        CurrentAux->DiveSpeedMultiplier != DiveSpeedMultiplier)
    {
        NetworkPredictionProxy.WriteAuxState<FAetherAuxState>([this](FAetherAuxState& Aux)
            {
                Aux.Mass = this->Mass;
                Aux.MaxForwardThrust = this->MaxForwardThrust;
                Aux.MaxReverseThrust = this->MaxReverseThrust;
                Aux.MaxStrafeThrust = this->MaxStrafeThrust;
                Aux.MaxSpeed = this->MaxSpeed;
                Aux.LinearDampingBase = this->LinearDampingBase;
                Aux.PitchRate = this->PitchRate;
                Aux.YawRate = this->YawRate;
                Aux.RollRate = this->RollRate;
                Aux.RollInfluence = this->RollInfluence;
                Aux.DiveSpeedMultiplier = this->DiveSpeedMultiplier;
            }, "Flux_Stat_Update");  // Context tag tracing updates originating from the Flux Stat system.
    
    }
}

FRotator UAetherMovementComponent::GetMouseTargetRelativeRotation() const
{
    const FAetherSyncState* Sync = NetworkPredictionProxy.ReadSyncState<FAetherSyncState>();
    if (!Sync || CurrentAimDirection.IsNearlyZero()) return FRotator::ZeroRotator;

    FVector LocalDirToTarget = Sync->Rotation.Inverse().RotateVector(CurrentAimDirection);
    return LocalDirToTarget.Rotation();
}

const FAetherSyncState* UAetherMovementComponent::GetSyncState() const
{
    return NetworkPredictionProxy.ReadSyncState<FAetherSyncState>();
}

const FAetherAuxState* UAetherMovementComponent::GetAuxState() const
{
    return NetworkPredictionProxy.ReadAuxState<FAetherAuxState>();
}