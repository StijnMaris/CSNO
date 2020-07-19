// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSNOBombPlantingZone.generated.h"

class UBoxComponent;
class ACSNODefusableBomb;

UCLASS()
class CSNO_API ACSNOBombPlantingZone : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACSNOBombPlantingZone();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* OverlapComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UDecalComponent* DecalComp;

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                          int32 OtherBodyIndex);

    UFUNCTION()
    void OnPlantedBomb(AActor* Planter, AController* PlanterController);

private:
    UPROPERTY()
    ACSNODefusableBomb* DefusableBomb;

};
