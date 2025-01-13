// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InteractionWidgetComponent.h"


UInteractionWidgetComponent::UInteractionWidgetComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(64, 64);
	CanCharacterStepUpOn = ECB_No;
	SetCollisionProfileName("NoCollision");
}