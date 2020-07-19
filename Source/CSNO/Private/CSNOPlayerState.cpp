// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOPlayerState.h"
#include "CSNOCharacter.h"
#include "CSNOGameState.h"
#include "CSNO/CSNO.h"
#include "Kismet/GameplayStatics.h"
#include "Net/OnlineEngineInterface.h"
#include "Net/UnrealNetwork.h"

ACSNOPlayerState::ACSNOPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
    TeamNumber = 0;
    NumKills = 0;
    NumDeaths = 0;
    NumBulletsFired = 0;
    NumBulletsHit = 0;
    Accuracy = 0;
    bQuitter = false;
}

void ACSNOPlayerState::Reset() {
    Super::Reset();

    //PlayerStates persist across seamless travel.  Keep the same teams as previous match.
    //SetTeamNum(0);
    NumKills = 0;
    NumDeaths = 0;
    NumBulletsFired = 0;
    NumBulletsHit = 0;
    Accuracy = 0;
    bQuitter = false;
}

void ACSNOPlayerState::RegisterPlayerWithSession(bool bWasFromInvite) {
    if (UOnlineEngineInterface::Get()->DoesSessionExist(GetWorld(), NAME_GameSession)) {
        Super::RegisterPlayerWithSession(bWasFromInvite);
    }
}

void ACSNOPlayerState::UnregisterPlayerWithSession() {
    if (!IsFromPreviousLevel() && UOnlineEngineInterface::Get()->DoesSessionExist(GetWorld(), NAME_GameSession)) {
        Super::UnregisterPlayerWithSession();
    }
}

void ACSNOPlayerState::ClientInitialize(AController* InController) {
    Super::ClientInitialize(InController);

    UpdateTeamColors();
}

void ACSNOPlayerState::SetTeamNum(int NewTeamNumber) {
    TeamNumber = NewTeamNumber;

    UpdateTeamColors();
}

void ACSNOPlayerState::OnRep_TeamColor() {
    UpdateTeamColors();
}

void ACSNOPlayerState::AddBulletsFired(int NumBullets) {
    NumBulletsFired += NumBullets;
}

void ACSNOPlayerState::AddBulletsHit(int NumHits) {
    NumBulletsHit += NumHits;
}

void ACSNOPlayerState::SetQuitter(bool bInQuitter) {
    bQuitter = bInQuitter;
}

void ACSNOPlayerState::CopyProperties(APlayerState* PlayerState) {
    Super::CopyProperties(PlayerState);

    ACSNOPlayerState* ShooterPlayer = Cast<ACSNOPlayerState>(PlayerState);
    if (ShooterPlayer) {
        ShooterPlayer->TeamNumber = TeamNumber;
    }
}

//#pragma optimize("", off)
void ACSNOPlayerState::BeginPlay() {
    Super::BeginPlay();

    APawn* Player = GetPawn();
    if (Player) {
        UCSNOHealthComponent* HC = Cast<UCSNOHealthComponent>(
            Player->GetComponentByClass(UCSNOHealthComponent::StaticClass()));

        if (HC) {
            HC->OnDied.AddUniqueDynamic(this, &ACSNOPlayerState::OnDied);
        }
    }

}

void ACSNOPlayerState::UpdateTeamColors() {
    AController* OwnerController = Cast<AController>(GetOwner());
    if (OwnerController) {
        ACSNOCharacter* ShooterCharacter = Cast<ACSNOCharacter>(OwnerController->GetCharacter());
        if (ShooterCharacter) {
            //Todo: TeamColors 
            //ShooterCharacter->UpdateTeamColorsAllMIDs();
        }
    }
}

int ACSNOPlayerState::GetTeamNum() const {
    return TeamNumber;
}

int ACSNOPlayerState::GetKills() const {
    return NumKills;
}

int ACSNOPlayerState::GetDeaths() const {
    return NumDeaths;
}

int ACSNOPlayerState::GetNumBulletsFired() const {
    return NumBulletsFired;
}

int ACSNOPlayerState::GetNumBulletsHit() const {
    return NumBulletsHit;
}

int ACSNOPlayerState::GetAccuracy() const {
    return Accuracy;
}

bool ACSNOPlayerState::IsQuitter() const {
    return bQuitter;
}

void ACSNOPlayerState::ScoreKill(ACSNOPlayerState* Victim, int Points) {
    NumKills++;
    ScorePoints(Points);
}

void ACSNOPlayerState::ScoreDeath(ACSNOPlayerState* KilledBy, int Points) {
    NumDeaths++;
    ScorePoints(Points);
}

void ACSNOPlayerState::ScorePoints(int Points) {
    ACSNOGameState* const MyGameState = GetWorld()->GetGameState<ACSNOGameState>();
    if (MyGameState && TeamNumber >= 0) {
        if (TeamNumber >= MyGameState->TeamScores.Num()) {
            MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
        }

        MyGameState->TeamScores[TeamNumber] += Points;
    }

    SetScore(GetScore() + Points);
}

void ACSNOPlayerState::OnDied(bool bIsDead, AActor* VictimActor, AActor* KillerActor, AController* KillerController) {
    UE_LOG(LogTemp, Log, TEXT("Playerstate: Died delegate"));
    if (KillerController) {
        ACSNOPlayerState* PS = KillerController->GetPlayerState<ACSNOPlayerState>();
        if (PS) {
            PS->ScoreKill(this, 1);
            this->ScoreDeath(PS, 0);
        }
    }
}

void ACSNOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACSNOPlayerState, TeamNumber);
    DOREPLIFETIME(ACSNOPlayerState, NumKills);
    DOREPLIFETIME(ACSNOPlayerState, NumDeaths);
    DOREPLIFETIME(ACSNOPlayerState, Accuracy);
}

FString ACSNOPlayerState::GetShortPlayerName() const {
    if (GetPlayerName().Len() > MAX_PLAYER_NAME_LENGTH) {
        return GetPlayerName().Left(MAX_PLAYER_NAME_LENGTH) + "...";
    }
    return GetPlayerName();
}
