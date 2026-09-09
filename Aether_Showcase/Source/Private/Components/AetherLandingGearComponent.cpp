// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#include "Components/AetherLandingGearComponent.h"

// -----------------------
// Lifecycle
// -----------------------

UAetherLandingGearComponent::UAetherLandingGearComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// -----------------------
// Gear State Management
// -----------------------

void UAetherLandingGearComponent::SetGearDeployed(bool bDeploy)
{
    bIsDeployed = bDeploy;
}

void UAetherLandingGearComponent::ToggleGearDeployed()
{
    bIsDeployed = !bIsDeployed;
}