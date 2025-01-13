// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameManagerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GAMEMANAGER_API AGameManagerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

};