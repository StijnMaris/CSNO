// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CSNOHUD.generated.h"

UCLASS()
class ACSNOHUD : public AHUD {
    GENERATED_BODY()

public:
    ACSNOHUD();

    /** Primary draw call for the HUD */
    virtual void DrawHUD() override;

private:

};
