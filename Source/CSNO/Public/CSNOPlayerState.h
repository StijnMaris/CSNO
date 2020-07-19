// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CSNOPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CSNO_API ACSNOPlayerState : public APlayerState {
    GENERATED_BODY()

public:
    ACSNOPlayerState(const FObjectInitializer& ObjectInitializer);

    virtual void Reset() override;

    virtual void ClientInitialize(class AController* InController) override;

    virtual void RegisterPlayerWithSession(bool bWasFromInvite) override;
    virtual void UnregisterPlayerWithSession() override;

    void SetTeamNum(int NewTeamNumber);

    void ScoreKill(ACSNOPlayerState* Victim, int Points);

    void ScoreDeath(ACSNOPlayerState* KilledBy, int Points);

    int GetTeamNum() const;

    int GetKills() const;

    int GetDeaths() const;

    int GetNumBulletsFired() const;

    int GetNumBulletsHit() const;

    int GetAccuracy() const;

    bool IsQuitter() const;

    FString GetShortPlayerName() const;

    UFUNCTION()
    void OnRep_TeamColor();

    //We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
    void AddBulletsFired(int NumBullets);
    void AddBulletsHit(int NumHits);

    void SetQuitter(bool bInQuitter);

    virtual void CopyProperties(class APlayerState* PlayerState) override;

protected:
    virtual void BeginPlay() override;

    /** Set the mesh colors based on the current teamnum variable */
    void UpdateTeamColors();

    UPROPERTY(Transient, ReplicatedUsing=OnRep_TeamColor)
    int TeamNumber;

    UPROPERTY(Transient, Replicated)
    int NumKills;

    UPROPERTY(Transient, Replicated)
    int NumDeaths;

    UPROPERTY()
    int NumBulletsFired;

    UPROPERTY()
    int NumBulletsHit;

    UPROPERTY(Transient, Replicated)
    int Accuracy;

    UPROPERTY()
    bool bQuitter;

    /** helper for scoring points */
    void ScorePoints(int Points);

    UFUNCTION()
    void OnDied(bool bIsDead, AActor* VictimActor, AActor* KillerActor, AController* KillerController);
};
