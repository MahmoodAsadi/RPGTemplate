// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameManagerGameMode.h"
#include "GameFramework/PlayerStart.h"

#include "ProfileManagerSubsystem.h"


AActor* AGameManagerGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    FActorSpawnParameters SpawnParam;
    SpawnParam.Owner = Player;
    SpawnParam.Instigator = Player->GetPawn();

    const FOpenLevelPlayerSpawnSetting& SpawnSetting = UProfileManagerSubsystem::Get(this)->GetPlayerSpawnSetting(this);
    switch (SpawnSetting.TransformType)
    {
        case EOpenLevelPlayerTransformType::Default:
        {
            if (AActor* AsDefault = Super::FindPlayerStart_Implementation(Player, "Default"))
                return AsDefault;

            return Super::FindPlayerStart_Implementation(Player, IncomingName);
        }

        case EOpenLevelPlayerTransformType::FromLoad:
        case EOpenLevelPlayerTransformType::CustomTransform:
            return GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), SpawnSetting.SpawnTransform, SpawnParam);
        
        case EOpenLevelPlayerTransformType::CustomPlayerStart:
            Super::FindPlayerStart_Implementation(Player, SpawnSetting.PlayerStartName);
    }

    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}