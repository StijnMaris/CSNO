// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNO/Public/CSNOArmourDamageType.h"

UCSNOArmourDamageType::UCSNOArmourDamageType(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Multiplier = 0.5;
    bCanBeArmoured = true;
    bWasHeadshot = false;
}
