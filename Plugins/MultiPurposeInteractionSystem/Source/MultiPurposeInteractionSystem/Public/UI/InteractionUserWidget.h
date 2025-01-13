// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionUserWidget.generated.h"

class URadialSlider;
class UTextBlock;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class MULTIPURPOSEINTERACTIONSYSTEM_API UInteractionUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void StartInteraction(const float Duration);
	void StopInteraction();
	void UpdateInteractionCounter(int Current, int Max);

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URadialSlider* InteractionSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ButtonText;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* HoldAnimation;

	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* PressAnimation;

	UPROPERTY()
	float StartTime = 0.0f;

	UPROPERTY()
	float InteractionDuration = 0.0f;
};