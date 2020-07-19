// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNO/Public/CSNOHeadDamageType.h"

UCSNOHeadDamageType::UCSNOHeadDamageType(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Multiplier = 4;
    bCanBeArmoured = true;
    bWasHeadshot = true;
}
