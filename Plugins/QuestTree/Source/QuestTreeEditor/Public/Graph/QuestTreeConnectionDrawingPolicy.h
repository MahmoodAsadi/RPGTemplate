// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

class UEdGraph;
class UEdGraphPin;

struct FSplineShape
{
	float DeltaX;
	float DeltaY;
	float ScaleX;
	float ScaleY;
};

struct FVerticalSplineSettings
{
	float FwdDeltaRangeX;
	float BwdDeltaRangeX;
	float FwdDeltaRangeY;
	float BwdDeltaRangeY;
	double FwdTanScaleX;
	double FwdTanScaleY;
	double BwdTanScaleX;
	double BwdTanScaleY;
};

/**
 * 
 */
class FQuestTreeConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	
	// ~ Start FConnectionDrawingPolicy interface
	FQuestTreeConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraph);
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params) override;
	virtual FVector2D ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const override;
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint, const FVector2D& EndPoint, UEdGraphPin* Pin) override;
	virtual void DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint, const FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
	// ~ End FConnectionDrawingPolicy interface

protected:

	FVector2D GetPinConnectionPoint(const FGeometry& InPinGeom, const EEdGraphPinDirection InDirection) const;
	FSplineShape GetSplineShape(FVector2D DeltaPos) const;

	UEdGraph* EdGraph;
	float PinRadius = 14.0f;

	/** Cached settings for the vertically flowing spines */
	FVerticalSplineSettings SplineSettings;

};