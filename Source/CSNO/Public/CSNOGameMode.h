// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CSNOGameMode.generated.h"

class ACSNOPlayerState;

UCLASS(minimalapi)
class ACSNOGameMode : public AGameMode {
    GENERATED_BODY()

public:
    ACSNOGameMode();

    virtual void PreInitializeComponents() override;

    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                          FString& ErrorMessage) override;

    virtual void DefaultTimer();

    virtual void HandleMatchIsWaitingToStart() override;

    virtual void HandleMatchHasStarted() override;

    virtual void RestartPlayer(AController* NewPlayer) override;

    UFUNCTION(exec)
    void FinishMatch();

protected:
    //var
    FTimerHandle TimerHandle_DefaultTimer;

    UPROPERTY(config)
    int WarmupTime;

    UPROPERTY(config)
    int RoundTime;

    UPROPERTY(config)
    int TimeBetweenMatches;

    UPROPERTY(config)
    int KillScore;

    UPROPERTY(config)
    int DeathScore;

    //func
    virtual void BeginPlay() override;

    virtual void DetermineMatchWinner();

    virtual bool IsWinner(ACSNOPlayerState* PlayerState) const;
};
