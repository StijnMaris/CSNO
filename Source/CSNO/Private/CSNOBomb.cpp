// Fill out your copyright notice in the Description page of Project Settings.

#include "CSNOBomb.h"
#include "CSNOCharacter.h"
#include "CSNOPlayerController.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "CSNODefusableBomb.h"
#include "Net/UnrealNetwork.h"

ACSNOBomb::ACSNOBomb(): Super() {
    BaseDamage = 100.f;
    MaxClipAmmo = 1;
    ArmourPiercing = 0.5;
    BombPlantTime = 4.f;
    
    TraceOffset = -500.f;
}

#pragma optimize("", off)
void ACSNOBomb::StartFire() {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    if (Player) {
        if (Player->IsInBombPlantingSite()) {
            if (CurrentClipAmmo > 0) {
                StartFireTime = GetWorld()->TimeSeconds;

                if (StartFireTime - LastFireTime >= 1.f || LastFireTime == 0.f) {
                    ACSNOPlayerController* PC = Cast<ACSNOPlayerController>(Player->GetController());
                    if (PC) {
                        PC->SetIgnoreMoveInput(true);
                        Fire();
                        //UE_LOG(LogTemp, Log, TEXT("Starting To plant bomb."));
                    }
                    LastFireTime = StartFireTime;
                }
            }
        }
    }
}

void ACSNOBomb::StopFire() {
    if (!HasAuthority()) {
        ServerStopFire();
    }

    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    if (Player) {
        ACSNOPlayerController* PC = Cast<ACSNOPlayerController>(Player->GetController());
        if (PC) {
            PC->SetIgnoreMoveInput(false);
            GetWorldTimerManager().ClearTimer(TimerHandle_BombPlant);
            //UE_LOG(LogTemp, Log, TEXT("Stopped To plant bomb"));
        }
    }
}
#pragma optimize("", on)

void ACSNOBomb::Fire() {
    if (!HasAuthority()) {
        ServerFire();
    }
    GetWorldTimerManager().SetTimer(TimerHandle_BombPlant, this, &ACSNOBomb::BombPlanted, BombPlantTime);
}

void ACSNOBomb::ServerStopFire_Implementation() {
    StopFire();
}

bool ACSNOBomb::ServerStopFire_Validate() {
    return true;
}

void ACSNOBomb::AddRecoil(ACSNOCharacter* Player) {
}

void ACSNOBomb::BombPlanted() {
    ACSNOCharacter* Player = Cast<ACSNOCharacter>(GetOwner());
    if (Player) {
        ACSNOPlayerController* PC = Cast<ACSNOPlayerController>(Player->GetController());
        if (PC) {
            UE_LOG(LogTemp, Log, TEXT("Bomb planted."));
            Player->OnPlantedBomb.Broadcast(Player, PC);
            
            SpawnDefusableBomb(Player);
            
            CurrentClipAmmo--;
            StopFire();

            UCSNOInventoryComponent* InvComp = Player->GetInventoryComponent();
            if (InvComp) {
                InvComp->RemoveItemFromInventory("Bomb_C4");
                InvComp->ChangeToPreviousInventorySlot();
            }

            Destroy();
            UGameplayStatics::PlaySound2D(GetWorld(), BombPlantedSound);
        }
    }
}

void ACSNOBomb::SpawnDefusableBomb(AActor* Planter) {

    if (!HasAuthority()) {
        ServerSpawnDefusableBomb(Planter);
        return;
    }
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FHitResult OutHit;
        FVector StartPoint = Planter->GetActorLocation();
        FVector EndPoint = StartPoint + FVector{0.f, 0.f, TraceOffset};
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(Planter);

        DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Green, false, 1, 0, 1);

        ENetMode netMode = GetNetMode();

        if (GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECC_WorldStatic, CollisionParams)) {
            if (OutHit.bBlockingHit) {
                /*DefusableBomb = */
                GetWorld()->SpawnActor<ACSNODefusableBomb>(DefusableBombClass, OutHit.ImpactPoint,
                                                           FRotator::ZeroRotator, SpawnParams);
                UE_LOG(LogTemp, Log, TEXT("Defusable bomb spawned"));
            }
        }
        else {
            /*DefusableBomb = */
            GetWorld()->SpawnActor<ACSNODefusableBomb>(DefusableBombClass, Planter->GetActorLocation(),
                                                       FRotator::ZeroRotator, SpawnParams);
        }
    
}

void ACSNOBomb::ServerSpawnDefusableBomb_Implementation(AActor* Planter) {
    SpawnDefusableBomb(Planter);
}

bool ACSNOBomb::ServerSpawnDefusableBomb_Validate(AActor* Planter) {
    return true;
}
