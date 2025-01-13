// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InteractionUserWidget.h"
#include "Components/RadialSlider.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"


void UInteractionUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (InteractionDuration > 0.0f)
	{
		const float SliderValue = FMath::Clamp((GetWorld()->GetTimeSeconds() - StartTime) / InteractionDuration, 0.0f, 1.0f);
		InteractionSlider->SetValue(SliderValue);
	}
}

void UInteractionUserWidget::StartInteraction(const float Duration)
{
	if (Duration <= 0.0f)
		return;

	InteractionDuration = Duration;
	InteractionSlider->SetValue(0.0f);
	StartTime = GetWorld()->GetTimeSeconds();

	if (IsValid(HoldAnimation))
	{
		PlayAnimation(HoldAnimation, 0.0f, 1, EUMGSequencePlayMode::PingPong, 1.0f, true);
	}
}

void UInteractionUserWidget::StopInteraction()
{
	InteractionDuration = 0.0f;
	InteractionSlider->SetValue(0.0f);

	if (IsValid(HoldAnimation))
	{
		StopAnimation(HoldAnimation);
	}
}

void UInteractionUserWidget::UpdateInteractionCounter(int Current, int Max)
{
	Max = Max == 0 ? 1 : Max;
	InteractionSlider->SetValue(float(Current) / float(Max));

	if (IsValid(PressAnimation))
	{
		PlayAnimation(PressAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, true);
	}
}