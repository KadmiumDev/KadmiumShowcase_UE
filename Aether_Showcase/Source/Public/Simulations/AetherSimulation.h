// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "Libraries/AetherTypes.h"
#include "NetworkPredictionModelDef.h"
#include "NetworkPredictionSimulation.h"
#include "NetworkPredictionTickState.h"

struct FAetherModelDef : FNetworkPredictionModelDef
{
    NP_MODEL_BODY();

    using StateTypes = TNetworkPredictionStateTypes<FAetherInputCmd, FAetherSyncState, FAetherAuxState>;
    using Simulation = class FAetherSimulation;
    using Driver = class UAetherMovementComponent;

    static const TCHAR* GetName() { return TEXT("AetherMovement"); }
    static constexpr int32 GetSortPriority() { return (int32)ENetworkPredictionSortPriority::KinematicMovers; }
};

class FAetherSimulation
{
public:
    // -----------------------
    // Core Simulation
    // -----------------------
    class UAetherMovementComponent* Driver = nullptr;

    void SimulationTick(const FNetSimTimeStep& TimeStep, const TNetSimInput<FAetherModelDef::StateTypes>& Input, const TNetSimOutput<FAetherModelDef::StateTypes>& Output);
};