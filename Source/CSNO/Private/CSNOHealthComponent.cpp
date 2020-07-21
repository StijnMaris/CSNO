// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOHealthComponent.h"

#include "CSNOArmourComponent.h"
#include "CSNOPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "CSNOWeaponBase.h"
#include "CSNO/Public/CSNODefaultDamageType.h"

// Sets default values for this component's properties
UCSNOHealthComponent::UCSNOHealthComponent() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    MaxHealth = 100;

    bIsDead = false;

    SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UCSNOHealthComponent::BeginPlay() {
    Super::BeginPlay();

    if (GetOwner()->HasAuthority()) {
        AActor* MyOwner = GetOwner();
        if (MyOwner) {
            MyOwner->OnTakeAnyDamage.AddUniqueDynamic(this, &UCSNOHealthComponent::HandleTakeAnyDamage);
        }
    }
    Health = MaxHealth;
}

// Called every frame
void UCSNOHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

float UCSNOHealthComponent::GetHealth() const {
    return Health;
}

float UCSNOHealthComponent::GetMaxHealth() const {
    return MaxHealth;
}

bool UCSNOHealthComponent::GetIsDead() const {
    return bIsDead;
}

void UCSNOHealthComponent::Heal(float HealAmount) {
    if (HealAmount <= 0.f || Health <= 0.f) {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health),
           *FString::SanitizeFloat(HealAmount));

    OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);

}

void UCSNOHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                               AController* InstigatedBy, AActor* DamageCauser) {
    if (Damage <= 0.f || bIsDead) {
        return;
    }

    float ActualDamage = Damage;
    UCSNOArmourComponent* ArmourComponent = Cast<UCSNOArmourComponent>(
        DamagedActor->GetComponentByClass(UCSNOArmourComponent::StaticClass()));

    ACSNOWeaponBase* Weapon = Cast<ACSNOWeaponBase>(DamageCauser);

    if (ArmourComponent && Weapon) {
        if (ArmourComponent->GetHasArmour()) {
            // if (ArmourComponent->GetHasHeadArmour()) {
            //     ArmourComponent->SetHasHeadArmour(false);
            // }
            // else {
            ActualDamage = Damage * Weapon->GetArmourPiercing();
            ArmourComponent->Damage(Damage - ActualDamage);
            //}
        }
    }

    Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));

    bIsDead = Health <= 0.f;
    if (bIsDead) {
        Killer = DamageCauser;
        KillerPlayerState = InstigatedBy->GetPlayerState<ACSNOPlayerState>();
        PlayerDied(bIsDead, GetOwner(), Killer, KillerPlayerState);
    }

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UCSNOHealthComponent::OnRep_Health(float OldHealth) {
    float Damage = Health - OldHealth;
    OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void UCSNOHealthComponent::PlayerDied_Implementation(bool bIsPlayerDead, AActor* VictimActor, AActor* KillerActor,
                                                     APlayerState* KillerPS) {
    OnDied.Broadcast(bIsPlayerDead, VictimActor, KillerActor, KillerPS);
}

void UCSNOHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCSNOHealthComponent, Health);
    DOREPLIFETIME(UCSNOHealthComponent, bIsDead);
    DOREPLIFETIME(UCSNOHealthComponent, Killer);
    DOREPLIFETIME(UCSNOHealthComponent, KillerPlayerState);
}
