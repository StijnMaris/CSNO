// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOArmourComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCSNOArmourComponent::UCSNOArmourComponent() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    DefaultArmour = 100;
    bHasArmour = false;
    bHasHeadArmour = false;
    bStartsWithArmour = false;

    SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UCSNOArmourComponent::BeginPlay() {
    Super::BeginPlay();
    if (bStartsWithArmour) {
        Armour = DefaultArmour;
        bHasArmour = true;
    }
}

// Called every frame
void UCSNOArmourComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

float UCSNOArmourComponent::GetArmour() const {
    return Armour;
}

bool UCSNOArmourComponent::GetHasArmour() const {
    return bHasArmour;
}

bool UCSNOArmourComponent::GetHasHeadArmour() const {
    return bHasHeadArmour;
}

void UCSNOArmourComponent::Damage(float DamageAmount) {
    Armour -= DamageAmount;
    bHasArmour = Armour >= 0.f;
}

void UCSNOArmourComponent::SetHasHeadArmour(bool HasHeadArmour) {
    bHasHeadArmour = HasHeadArmour;
}

void UCSNOArmourComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCSNOArmourComponent, Armour)
    DOREPLIFETIME(UCSNOArmourComponent, bHasArmour)
    DOREPLIFETIME(UCSNOArmourComponent, bHasHeadArmour)
}
