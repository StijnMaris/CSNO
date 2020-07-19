// Copyright Epic Games, Inc. All Rights Reserved.

#include "CSNOGameMode.h"
#include "CSNOHUD.h"
#include "CSNOCharacter.h"
#include "CSNOGameState.h"
#include "CSNOPlayerController.h"
#include "CSNOPlayerState.h"
#include "EngineUtils.h"
#include "GameFramework/SpectatorPawn.h"

ACSNOGameMode::ACSNOGameMode()
    : Super() {
    // set default pawn class to our Blueprinted character
    DefaultPawnClass = ACSNOCharacter::StaticClass();

    // use our custom HUD class
    HUDClass = ACSNOHUD::StaticClass();

    PlayerControllerClass = ACSNOPlayerController::StaticClass();
    PlayerStateClass = ACSNOPlayerState::StaticClass();
    GameStateClass = ACSNOGameState::StaticClass();
    //Todo: Make spectator class ?
    SpectatorClass = ASpectatorPawn::StaticClass();
    ReplaySpectatorPlayerControllerClass = ACSNOPlayerController::StaticClass();

    MinRespawnDelay = 2.0f;

    WarmupTime = 5.f;
    RoundTime = 300.f;
    TimeBetweenMatches = 5.f;
    KillScore = 1;
    DeathScore = 0;
}

void ACSNOGameMode::PreInitializeComponents() {
    Super::PreInitializeComponents();

    GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ACSNOGameMode::DefaultTimer,
                                    GetWorldSettings()->GetEffectiveTimeDilation(), true);

}

void ACSNOGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                             FString& ErrorMessage) {

    ACSNOGameState* const MyGameState = Cast<ACSNOGameState>(GameState);
    const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();

    if (bMatchIsOver) {
        ErrorMessage = TEXT("Match is over!");
    }
    else {
        Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    }
}

void ACSNOGameMode::DefaultTimer() {
    ACSNOGameState* const MyGameState = Cast<ACSNOGameState>(GameState);
    if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused) {
        MyGameState->RemainingTime--;

        if (MyGameState->RemainingTime <= 0) {
            if (GetMatchState() == MatchState::WaitingPostMatch) {
                RestartGame();
            }
            else if (GetMatchState() == MatchState::InProgress) {
                FinishMatch();
            }
            else if (GetMatchState() == MatchState::WaitingToStart) {
                StartMatch();
            }
        }
    }
}

void ACSNOGameMode::HandleMatchIsWaitingToStart() {
    Super::HandleMatchIsWaitingToStart();

    if (bDelayedStart) {
        // start warmup if needed
        ACSNOGameState* const MyGameState = Cast<ACSNOGameState>(GameState);
        if (MyGameState && MyGameState->RemainingTime == 0) {
            if (WarmupTime > 0) {
                MyGameState->RemainingTime = WarmupTime;
            }
            else {
                MyGameState->RemainingTime = 0.0f;
            }
        }
    }
}

void ACSNOGameMode::HandleMatchHasStarted() {
    Super::HandleMatchHasStarted();

    ACSNOGameState* const MyGameState = Cast<ACSNOGameState>(GameState);
    MyGameState->RemainingTime = RoundTime;
}

void ACSNOGameMode::RestartPlayer(AController* NewPlayer) {
    Super::RestartPlayer(NewPlayer);
}

void ACSNOGameMode::FinishMatch() {
    ACSNOGameState* const MyGameState = Cast<ACSNOGameState>(GameState);
    if (IsMatchInProgress()) {
        EndMatch();
        DetermineMatchWinner();

        // notify players
        for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It) {
            ACSNOPlayerState* PlayerState = Cast<ACSNOPlayerState>((*It)->PlayerState);
            const bool bIsWinner = IsWinner(PlayerState);

            (*It)->GameHasEnded(nullptr, bIsWinner);
        }

        for (APawn* Pawn : TActorRange<APawn>(GetWorld())) {
            Pawn->TurnOff();
        }

        // set up to restart the match
        MyGameState->RemainingTime = TimeBetweenMatches;
    }
}

void ACSNOGameMode::BeginPlay() {
    Super::BeginPlay();

    //OnActorKilled.AddDynamic(this, &ACSNOGameMode::OnKilled);
}

void ACSNOGameMode::DetermineMatchWinner() {
}

bool ACSNOGameMode::IsWinner(ACSNOPlayerState* PlayerState) const {
    return false;
}
