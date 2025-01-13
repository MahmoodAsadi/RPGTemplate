// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UQuestTreeEdGraphNode;

/**
 * 
 */
class SQuestTreeGraphNode : public SGraphNode
{
public:
	
	SLATE_BEGIN_ARGS(SQuestTreeGraphNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestTreeEdGraphNode* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End of SGraphNode interface

private:

	FText GetNodeCompactTitle() const;
	FSlateColor GetBackgroundColor() const;
	FSlateColor GetBorderColor() const;
	FSlateColor GetPinBoxColor() const;
	const FSlateBrush* GetNodeIcon() const;
	const FSlateBrush* GetPinBoxIcon() const;
	float GetInputPinBoxWidth() const;
	float GetOutputPinBoxWidth() const;
	EVisibility GetInputBoxVisibility() const;
	EVisibility GetOutputBoxVisibility() const;
	EVisibility GetQuestNameTextVisibility() const;
	FText GetQuestNameText() const;
	EVisibility GetDetailSlotVisibility() const;
	FText GetDetailSlotText() const;
	EVisibility GetErrorTextVisibility() const;
	FText GetErrorText() const;
	FLinearColor GetErrorBackgroundColor() const;
	EVisibility GetDebuggerSearchFailedMarkerVisibility() const;
	FText GetPinTooltip(UEdGraphPin* GraphPinObj) const;

	TSharedPtr<SHorizontalBox> InputPinBox;
	TSharedPtr<SHorizontalBox> OutputPinBox;
	TSharedPtr<SBorder> NodeBody;
};