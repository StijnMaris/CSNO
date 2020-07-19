// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CSNOGameState.generated.h"

class ACSNOPlayerState;

typedef TMap<int, TWeakObjectPtr<ACSNOPlayerState>> RankedPlayerMap;

UCLASS()
class CSNO_API ACSNOGameState : public AGameState {
    GENERATED_BODY()

public:
    ACSNOGameState(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(Transient, Replicated)
    int NumTeams;

    /** accumulated score per team */
    UPROPERTY(Transient, Replicated)
    TArray<int> TeamScores;

    /** time left for warmup / match */
    UPROPERTY(Transient, Replicated, BlueprintReadOnly)
    int RemainingTime;

    /** is timer paused? */
    UPROPERTY(Transient, Replicated)
    bool bTimerPaused;

    /** gets ranked PlayerState map for specific team */
    void GetRankedMap(int TeamIndex, RankedPlayerMap& OutRankedMap) const;

    void RequestFinishAndExitToMainMenu();
};
