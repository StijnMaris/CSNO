// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOPlayerController.h"

#include "CSNOHUD.h"

void ACSNOPlayerController::HandleReturnToMainMenu() {

}

ACSNOHUD* ACSNOPlayerController::GetCSNOHUD() const {
    return Cast<ACSNOHUD>(GetHUD());
}
