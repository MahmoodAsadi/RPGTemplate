// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeConnectionDrawingPolicy.h"


FQuestTreeConnectionDrawingPolicy::FQuestTreeConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraph)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements), EdGraph(InGraph)
{
	ArrowImage = nullptr;
	ArrowRadius = FVector2D::ZeroVector;

	//Flip direction of relevant spline settings for vertical flow 
	SplineSettings.FwdDeltaRangeX =
		Settings->ForwardSplineVerticalDeltaRange;

	SplineSettings.BwdDeltaRangeX =
		Settings->BackwardSplineVerticalDeltaRange;

	SplineSettings.FwdDeltaRangeY =
		Settings->ForwardSplineHorizontalDeltaRange;

	SplineSettings.BwdDeltaRangeY =
		Settings->BackwardSplineHorizontalDeltaRange;

	SplineSettings.FwdTanScaleX =
		Settings->ForwardSplineTangentFromVerticalDelta.Y;

	SplineSettings.FwdTanScaleY =
		Settings->ForwardSplineTangentFromHorizontalDelta.X;

	SplineSettings.BwdTanScaleX =
		Settings->BackwardSplineTangentFromVerticalDelta.Y;

	SplineSettings.BwdTanScaleY =
		Settings->BackwardSplineTangentFromHorizontalDelta.X;

	/*if (GetDefault<UBlueprintEditorSettings>()->bDrawMidpointArrowsInBlueprints)
	{
		MidpointImage = FAppStyle::GetBrush(TEXT("Graph.Arrow"));
		MidpointRadius = MidpointImage->ImageSize * ZoomFactor * 0.5f;
	}*/
}

void FQuestTreeConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, FConnectionParams& Params)
{
	FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);

	// Emphasize wire thickness on hovered pins
	if (HoveredPins.Contains(InputPin) && HoveredPins.Contains(OutputPin))
	{
		Params.WireThickness = Params.WireThickness * 3;
	}
}

FVector2D FQuestTreeConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const
{
	//Calculate delta 
	const FVector2D DeltaPos = End - Start;

	//Determine directionality
	const FSplineShape Shape = GetSplineShape(DeltaPos);

	//Calculate base clamping 
	const float ClampedTensionX = FMath::Min<float>(FMath::Abs<float>(DeltaPos.X), Shape.DeltaX);
	const float ClampedTensionY = FMath::Min<float>(FMath::Abs<float>(DeltaPos.Y), Shape.DeltaY);

	//Return final tangent 
	return FVector2D(ClampedTensionX * Shape.ScaleX, ClampedTensionY * Shape.ScaleY);
}

void FQuestTreeConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint, const FVector2D& EndPoint, UEdGraphPin* Pin)
{
	FConnectionParams ConnectionParams;
	DetermineWiringStyle(Pin, nullptr, ConnectionParams);

	if (Pin->Direction == EGPD_Input)
	{
		DrawSplineWithArrow(StartPoint, GetPinConnectionPoint(PinGeometry, EGPD_Input), ConnectionParams);
	}
	else //Output pin
	{
		DrawSplineWithArrow(GetPinConnectionPoint(PinGeometry, EGPD_Output), EndPoint, ConnectionParams);
	}
}

void FQuestTreeConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint, const FConnectionParams& Params)
{
	FConnectionDrawingPolicy::DrawSplineWithArrow(StartPoint, EndPoint, Params);
}

void FQuestTreeConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	const FVector2D StartPoint = GetPinConnectionPoint(StartGeom, EGPD_Output);
	const FVector2D EndPoint = GetPinConnectionPoint(EndGeom, EGPD_Input);
	DrawSplineWithArrow(StartPoint, EndPoint, Params);
}

FVector2D FQuestTreeConnectionDrawingPolicy::GetPinConnectionPoint(const FGeometry& InPinGeom, const EEdGraphPinDirection InDirection) const
{
	FVector2D PinCenter = FGeometryHelper::CenterOf(InPinGeom);
	
	if (InDirection == EGPD_Input)
	{
		FVector2D InputPinOffset = FVector2D(0.f, PinRadius * ZoomFactor);
		return PinCenter - InputPinOffset;
	}
	else //Output pin 
	{
		FVector2D OutputPinOffset = FVector2D(0.f, PinRadius * ZoomFactor);
		return PinCenter + OutputPinOffset;
	}
}

FSplineShape FQuestTreeConnectionDrawingPolicy::GetSplineShape(FVector2D DeltaPos) const
{
	//Determine directionality
	const bool bGoingForward = DeltaPos.Y <= 0.0f;
	FSplineShape Shape = FSplineShape();

	if (bGoingForward)
	{
		Shape.DeltaX = SplineSettings.FwdDeltaRangeX;
		Shape.DeltaY = SplineSettings.FwdDeltaRangeY;
		Shape.ScaleX = SplineSettings.FwdTanScaleX;
		Shape.ScaleY = SplineSettings.FwdTanScaleY;
	}
	else //Going backward
	{
		Shape.DeltaX = SplineSettings.BwdDeltaRangeX;
		Shape.DeltaY = SplineSettings.BwdDeltaRangeY;
		Shape.ScaleX = SplineSettings.BwdTanScaleX;
		Shape.ScaleY = SplineSettings.BwdTanScaleY;
	}

	return Shape;
}