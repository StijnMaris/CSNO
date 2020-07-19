// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOBombPlantingZone.h"

#include "CSNOCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "CSNODefusableBomb.h"
#include "DrawDebugHelpers.h"

// Sets default values
ACSNOBombPlantingZone::ACSNOBombPlantingZone() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    OverlapComp = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComp"));
    OverlapComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    OverlapComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    OverlapComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    OverlapComp->SetBoxExtent(FVector(200.f));
    OverlapComp->SetHiddenInGame(false);
    RootComponent = OverlapComp;

    OverlapComp->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACSNOBombPlantingZone::HandleBeginOverlap);
    OverlapComp->OnComponentEndOverlap.AddUniqueDynamic(this, &ACSNOBombPlantingZone::HandleEndOverlap);

    DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
    DecalComp->DecalSize = FVector(1.f, 200.f, 200.f);
    DecalComp->SetupAttachment(RootComponent);

    
}

// Called when the game starts or when spawned
void ACSNOBombPlantingZone::BeginPlay() {
    Super::BeginPlay();

}

void ACSNOBombPlantingZone::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult) {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(OtherActor);
    if (Player) {
        Player->SetIsInBombPlantingSite(true);
        Player->OnPlantedBomb.AddUniqueDynamic(this, &ACSNOBombPlantingZone::OnPlantedBomb);
    }
}

void ACSNOBombPlantingZone::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(OtherActor);
    if (Player) {
        Player->SetIsInBombPlantingSite(false);
        Player->OnPlantedBomb.RemoveDynamic(this, &ACSNOBombPlantingZone::OnPlantedBomb);
    }
}
#pragma optimize("", off)
void ACSNOBombPlantingZone::OnPlantedBomb(AActor* Planter, AController* PlanterController) {
    ENetMode netMode = GetNetMode();
}
#pragma optimize("", on)


// Called every frame
void ACSNOBombPlantingZone::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
