// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSNODefusableBomb.generated.h"

class UBoxComponent;

UCLASS()
class CSNO_API ACSNODefusableBomb : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACSNODefusableBomb();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void StartDefuse();

    UFUNCTION()
    void StopDefuse();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UBoxComponent* OverlapComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, Category = "Sounds")
    USoundBase* BombBeepSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sounds")
    USoundBase* ExplosionSound;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    UParticleSystem* ExplosionEffect;

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                          int32 OtherBodyIndex);

    UFUNCTION()
    void BombDefused();
    
    UFUNCTION(Server,Reliable,WithValidation)
    void ServerBombDefused();

    UFUNCTION()
    void Explode();
    
    UFUNCTION(NetMulticast, Unreliable)
    void PlayExplodeEffects();
    
private:
    FTimerHandle TimerHandle_BombDefuse;

    FTimerHandle TimerHandle_Explode;

    UPROPERTY(EditDefaultsOnly)
    float BombDefuseTime;

    UPROPERTY(EditDefaultsOnly)
    float BombExplodeTime;

    UPROPERTY()
    bool bIsDefused;

    UPROPERTY(EditDefaultsOnly, Category = "Explosion")
    float ExplosionDamage;

    UPROPERTY(EditDefaultsOnly, Category = "Explosion")
    float ExplosionRadius;
};
