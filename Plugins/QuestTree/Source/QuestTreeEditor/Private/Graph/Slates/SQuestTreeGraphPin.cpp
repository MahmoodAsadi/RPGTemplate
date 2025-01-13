// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/Slates/SQuestTreeGraphPin.h"
#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/QuestTreeNode.h"
#include "QuestTreeStyle.h"

#define LOCTEXT_NAMESPACE "SQuestTreeGraphPin"

void SQuestTreeGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SQuestTreeGraphPin::GetPinImage)
		.BorderBackgroundColor(this, &SQuestTreeGraphPin::GetPinColor)
		.OnMouseButtonDown(this, &SQuestTreeGraphPin::OnPinMouseDown)
		.Cursor(this, &SQuestTreeGraphPin::GetPinCursor)
		.Padding(FMargin(7.0f))
	);
}

FSlateColor SQuestTreeGraphPin::GetPinColor() const
{
	UQuestTreeNode* GraphNode = Cast<UQuestTreeEdGraphNode>(GetPinObj()->GetOwningNode())->GetQuestTreeNode();
	TArray<FQuestTreePinMaker> Pins = GetPinObj()->Direction == EGPD_Input ? GraphNode->GetInputPinsMakers() : GraphNode->GetOutputPinsMakers();
	
	FLinearColor FoundColor(FLinearColor::Gray);
	for (const FQuestTreePinMaker& PinMaker : Pins)
	{
		if (PinMaker.Name == GetPinObj()->PinName)
		{
			FoundColor = PinMaker.DefaultColor;
			break;
		}
	}
	
	return FSlateColor(IsHovered() ? FLinearColor::Yellow : FoundColor);
}

TSharedRef<SWidget> SQuestTreeGraphPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SQuestTreeGraphPin::GetPinImage() const
{
	FString BrushNameString = GetPinObj()->Direction == EGPD_Input ? FString("QuestTree.InputPin") : FString("QuestTree.OutputPin");
	if (IsConnected())
		BrushNameString.Append(".Connected");
	
	return FQuestTreeStyle::Get().GetBrush((TEXT("%s"), *BrushNameString));
}

#undef LOCTEXT_NAMESPACE