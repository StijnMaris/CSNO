// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOGameState.h"
#include "CSNOPlayerState.h"
#include "CSNOGameMode.h"
#include "CSNOPlayerController.h"
#include "Net/UnrealNetwork.h"

ACSNOGameState::ACSNOGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    NumTeams = 0;
    RemainingTime = 0;
    bTimerPaused = false;
}

void ACSNOGameState::GetRankedMap(int TeamIndex, RankedPlayerMap& OutRankedMap) const {
    OutRankedMap.Empty();

    //first, we need to go over all the PlayerStates, grab their score, and rank them
    TMultiMap<int, ACSNOPlayerState*> SortedMap;
    for (int i = 0; i < PlayerArray.Num(); ++i) {
        int Score = 0;
        ACSNOPlayerState* CurPlayerState = Cast<ACSNOPlayerState>(PlayerArray[i]);
        if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex)) {
            SortedMap.Add(FMath::TruncToInt(CurPlayerState->GetScore()), CurPlayerState);
        }
    }

    //sort by the keys
    SortedMap.KeySort(TGreater<int>());

    //now, add them back to the ranked map
    OutRankedMap.Empty();

    int Rank = 0;
    for (TMultiMap<int, ACSNOPlayerState*>::TIterator It(SortedMap); It; ++It) {
        OutRankedMap.Add(Rank++, It.Value());
    }
}

void ACSNOGameState::RequestFinishAndExitToMainMenu() {

    if (AuthorityGameMode) {
        // we are server, tell the gamemode
        ACSNOGameMode* const GameMode = Cast<ACSNOGameMode>(AuthorityGameMode);
        if (GameMode) {
            //Todo: Add exit to menu to gamemode
            // GameMode->RequestFinishAndExitToMainMenu();
        }
    }
    else {
        // we are client, handle our own business
        ACSNOPlayerController* const PrimaryPC = Cast<ACSNOPlayerController>(
            GetGameInstance()->GetFirstLocalPlayerController());
        if (PrimaryPC) {
            PrimaryPC->HandleReturnToMainMenu();
        }
    }
}

void ACSNOGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSNOGameState, NumTeams);
    DOREPLIFETIME(ACSNOGameState, RemainingTime);
    DOREPLIFETIME(ACSNOGameState, bTimerPaused);
    DOREPLIFETIME(ACSNOGameState, TeamScores);
}
