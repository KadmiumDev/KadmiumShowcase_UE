// Copyright (c) 2026 Kadmium (Emil Fredrik Sjöstedt). All rights reserved.
// Licensed under the Kadmium Software & Source Code License Agreement.
// See LICENSE.md in the project root for full license terms.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AetherGravitySource.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAetherGravitySource : public UInterface
{
    GENERATED_BODY()
};

class AETHER_API IAetherGravitySource
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aether|Gravity")
    FVector GetGravityForceAtLocation(FVector TargetLocation);
};