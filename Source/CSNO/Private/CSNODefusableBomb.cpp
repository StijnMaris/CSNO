// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNODefusableBomb.h"

#include "CSNOCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineBaseTypes.h"

// Sets default values
ACSNODefusableBomb::ACSNODefusableBomb() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
    OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapComp->SetBoxExtent(FVector(20.f));
    OverlapComp->SetHiddenInGame(false);
    RootComponent = OverlapComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    OverlapComp->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACSNODefusableBomb::HandleBeginOverlap);
    OverlapComp->OnComponentEndOverlap.AddUniqueDynamic(this, &ACSNODefusableBomb::HandleEndOverlap);

    SetReplicates(true);
    // SetReplicateMovement(true);

    BombDefuseTime = 5.f;
    BombExplodeTime = 10.f;
    ExplosionDamage = 1000.f;
    ExplosionRadius = 500.f;
}

// Called when the game starts or when spawned
void ACSNODefusableBomb::BeginPlay() {
    Super::BeginPlay();

    UAudioComponent* Beepsound = UGameplayStatics::SpawnSoundAttached(BombBeepSound, MeshComp);
    //UGameplayStatics::PlaySoundAtLocation(GetWorld(), BombBeepSound, GetActorLocation());
    if (HasAuthority()) {
        GetWorldTimerManager().SetTimer(TimerHandle_Explode, this, &ACSNODefusableBomb::Explode, BombExplodeTime);
    }

}

// Called every frame
void ACSNODefusableBomb::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

}

void ACSNODefusableBomb::StartDefuse() {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    
    if (Player) {
        UCSNOInventoryComponent* InventoryComp = Player->GetInventoryComponent();
        if (InventoryComp) {
            float ActualDefuseTime = BombDefuseTime / (1 + InventoryComp->HasItemInInventory("Defuse_Kit"));
            GetWorldTimerManager().SetTimer(TimerHandle_BombDefuse, this, &ACSNODefusableBomb::BombDefused,
                                           ActualDefuseTime );
            UE_LOG(LogTemp, Log, TEXT("%f: Sec Defuse started"), ActualDefuseTime);
        }
    }
}

void ACSNODefusableBomb::StopDefuse() {
    GetWorldTimerManager().ClearTimer(TimerHandle_BombDefuse);
}

void ACSNODefusableBomb::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                            const FHitResult& SweepResult) {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(OtherActor);
    if (Player) {
        Player->DefusableBomb = this;
        SetOwner(Player);
    }
}

void ACSNODefusableBomb::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(OtherActor);
    if (Player) {
        Player->DefusableBomb = nullptr;
        SetOwner(nullptr);
    }
}

void ACSNODefusableBomb::BombDefused() {
    if (!HasAuthority()) {
        ServerBombDefused();
        return;
    }
    bIsDefused = true;
    GetWorldTimerManager().ClearTimer(TimerHandle_Explode);

    UE_LOG(LogTemp, Log, TEXT("%s %d: Bomb Defused"), *UEnum::GetValueAsString(GetLocalRole()), GPlayInEditorID);
}

void ACSNODefusableBomb::ServerBombDefused_Implementation() {
    BombDefused();
}

bool ACSNODefusableBomb::ServerBombDefused_Validate() {
    return true;
}

void ACSNODefusableBomb::Explode() {

    if (HasAuthority()) {
        UE_LOG(LogTemp, Log, TEXT("%s %d: Bomb Exploded"), *UEnum::GetValueAsString(GetLocalRole()), GPlayInEditorID);

        TArray<AActor*> IgnoredActors;
        IgnoredActors.Add(this);

        bool ApliedDamage = UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage,
                                                                MeshComp->GetComponentLocation() +
                                                                FVector(0.f, 0.f, 10.f),
                                                                ExplosionRadius, nullptr, IgnoredActors,
                                                                this, GetInstigatorController(), true);

        if (ApliedDamage) {
            PlayExplodeEffects();
            DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 24, FColor::Red,
                            false, 5.f, 0, 2.f);
        }
    }

}

void ACSNODefusableBomb::PlayExplodeEffects_Implementation() {
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
    UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
}
