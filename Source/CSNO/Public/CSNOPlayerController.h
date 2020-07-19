// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CSNOPlayerController.generated.h"

class ACSNOHUD;

/**
 * 
 */
UCLASS()
class CSNO_API ACSNOPlayerController : public APlayerController {
    GENERATED_BODY()

public:

    virtual void HandleReturnToMainMenu();

protected:
    //func
    ACSNOHUD* GetCSNOHUD() const;

    //var
    bool bAllowGameActions;
};
