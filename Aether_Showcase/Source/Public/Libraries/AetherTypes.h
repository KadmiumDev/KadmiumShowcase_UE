// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "NetworkPredictionStateTypes.h"
#include "NetworkPredictionReplicationProxy.h"

struct FAetherInputCmd
{
    float ForwardCmd = 0.f;
    float RightCmd = 0.f;
    float UpCmd = 0.f;

    float PitchCmd = 0.f;
    float YawCmd = 0.f;
    float RollCmd = 0.f;

    FVector AimDirection = FVector::ForwardVector;
    bool bIsCoupledMode = true;
    bool bWantsLandingGear = true;
    FVector GravityForce = FVector::ZeroVector;
    float EnvironmentDensity = 0.0f;

    void NetSerialize(const FNetSerializeParams& P)
    {
        P.Ar << ForwardCmd; P.Ar << RightCmd; P.Ar << UpCmd;
        P.Ar << PitchCmd; P.Ar << YawCmd; P.Ar << RollCmd;
        P.Ar << AimDirection;
        P.Ar << bIsCoupledMode;
        P.Ar << bWantsLandingGear;
        P.Ar << GravityForce;
        P.Ar << EnvironmentDensity;
    }

    void ToString(FAnsiStringBuilderBase& Builder) const
    {
        Builder.Appendf("Lin:(%.1f, %.1f, %.1f) Ang:(%.1f, %.1f, %.1f) AimDir:(%.2f, %.2f, %.2f) Grav:(%.2f, %.2f, %.2f) Dens: %.2f",
            ForwardCmd, RightCmd, UpCmd, PitchCmd, YawCmd, RollCmd,
            AimDirection.X, AimDirection.Y, AimDirection.Z,
            GravityForce.X, GravityForce.Y, GravityForce.Z,
            EnvironmentDensity);
    }
};


struct FAetherSyncState
{
    FVector Location = FVector::ZeroVector;
    FQuat Rotation = FQuat::Identity;
    FVector LinearVelocity = FVector::ZeroVector;
    bool bLandingGearDeployed = true;

    float CurrentGForce = 0.f;
    float CurrentAOA = 0.f;

    bool ShouldReconcile(const FAetherSyncState& AuthorityState) const
    {
        return FVector::DistSquared(Location, AuthorityState.Location) > 100.0f ||
               Rotation.AngularDistance(AuthorityState.Rotation) > 0.05f ||
               bLandingGearDeployed != AuthorityState.bLandingGearDeployed;
    }

    void Interpolate(const FAetherSyncState* From, const FAetherSyncState* To, float PCT)
    {
        Location = FMath::Lerp(From->Location, To->Location, PCT);
        Rotation = FQuat::FastLerp(From->Rotation, To->Rotation, PCT).GetNormalized();
        LinearVelocity = FMath::Lerp(From->LinearVelocity, To->LinearVelocity, PCT);
        bLandingGearDeployed = To->bLandingGearDeployed;
        CurrentGForce = FMath::Lerp(From->CurrentGForce, To->CurrentGForce, PCT);
        CurrentAOA = FMath::Lerp(From->CurrentAOA, To->CurrentAOA, PCT);

    }

    void NetSerialize(const FNetSerializeParams& P)
    {
        P.Ar << Location; P.Ar << Rotation; P.Ar << LinearVelocity;
        P.Ar << bLandingGearDeployed;
        P.Ar << CurrentGForce; P.Ar << CurrentAOA;
    }


    void ToString(FAnsiStringBuilderBase& Builder) const
    {
        Builder.Appendf("Loc: X=%.2f Y=%.2f Z=%.2f Vel: X=%.2f Y=%.2f Z=%.2f",
            Location.X, Location.Y, Location.Z, LinearVelocity.X, LinearVelocity.Y, LinearVelocity.Z);
    }
};


struct FAetherAuxState
{
    float Mass = 500.f;            


    float MaxForwardThrust = 8500.f; 
    float MaxReverseThrust = 5000.f;  
    float MaxStrafeThrust = 5000.f;   

    float MaxSpeed = 15000.f;        
    float LinearDampingBase = 1.f;
    float DiveSpeedMultiplier = 1.5f;

    float PitchRate = 2.25f;        
    float YawRate = 0.25f;
    float RollRate = 8.0f; 
    float RollInfluence = 1.0f;

    bool ShouldReconcile(const FAetherAuxState& AuthorityState) const
    {
        return Mass != AuthorityState.Mass ||
            MaxForwardThrust != AuthorityState.MaxForwardThrust ||
            MaxReverseThrust != AuthorityState.MaxReverseThrust ||
            MaxStrafeThrust != AuthorityState.MaxStrafeThrust ||
            MaxSpeed != AuthorityState.MaxSpeed ||
            LinearDampingBase != AuthorityState.LinearDampingBase ||
            PitchRate != AuthorityState.PitchRate ||
            YawRate != AuthorityState.YawRate ||
            RollRate != AuthorityState.RollRate ||
            RollInfluence != AuthorityState.RollInfluence ||
            DiveSpeedMultiplier != AuthorityState.DiveSpeedMultiplier;
    }

    void Interpolate(const FAetherAuxState* From, const FAetherAuxState* To, float PCT)
    {
        *this = *To;
    }

    void NetSerialize(const FNetSerializeParams& P)
    {
        P.Ar << Mass;
        P.Ar << MaxForwardThrust; P.Ar << MaxReverseThrust; P.Ar << MaxStrafeThrust;
        P.Ar << MaxSpeed; P.Ar << LinearDampingBase;
        P.Ar << PitchRate; P.Ar << YawRate; P.Ar << RollRate;
        P.Ar << RollInfluence;
        P.Ar << DiveSpeedMultiplier;
    }

    void ToString(FAnsiStringBuilderBase& Builder) const
    {
        Builder.Appendf("Mass: %.2f Fwd: %.2f Strf: %.2f", Mass, MaxForwardThrust, MaxStrafeThrust);
    }
};