// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/Slates/SQuestTreeGraphNode.h"

#include "IDocumentation.h"
#include "SCommentBubble.h"
#include "SGraphPin.h"
#include "TutorialMetaData.h"
#include "GraphEditorSettings.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Notifications/SErrorText.h"
#include "Editor.h"
#include "SGraphPanel.h"
#include "SCommentBubble.h"
#include "NodeFactory.h"

#include "Graph/QuestTreeEdGraphNode.h"
#include "Graph/QuestTreeNode.h"
#include "Graph/Slates/SQuestTreeGraphPin.h"

#define LOCTEXT_NAMESPACE "SQuestTreeGraphNode"

void SQuestTreeGraphNode::Construct(const FArguments& InArgs, UQuestTreeEdGraphNode* InNode)
{
	SetCursor(EMouseCursor::CardinalCross);

	GraphNode = InNode;
	UpdateGraphNode();
}

void SQuestTreeGraphNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	InputPinBox.Reset();
	OutputPinBox.Reset();

	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &SGraphNode::GetNodeTooltip), NULL, GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
		SetToolTip(DefaultToolTip);
	}

	// Setup a meta tag for this node
	FGraphNodeMetaData TagMeta(TEXT("QuestGraphNode"));
	PopulateMetaTag(&TagMeta);

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode).Text(this, &SQuestTreeGraphNode::GetNodeCompactTitle);
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<STextBlock> DetailTitleText;
	
	const FMargin NodePadding(8.0f);
	FSlateFontInfo DetailTitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 11.0f);
	FSlateFontInfo DetailDescriptionFont = FCoreStyle::GetDefaultFontStyle("Italic", 9.0f);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.MaxDesiredWidth(600.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(0.0f)
				.BorderBackgroundColor(this, &SQuestTreeGraphNode::GetBorderColor)
				[
					SNew(SOverlay)

					// Pins and node details
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SVerticalBox)

						// ~ Start input pins area
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							[
								SAssignNew(LeftNodeBox, SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Center)
								.Padding(20.0f, 0.0f)
								.FillHeight(1.0f)
								[
									SNew(SOverlay)

									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Top)
									[
										SNew(SBox)
										.HeightOverride(15.0f)
										.WidthOverride(GetInputPinBoxWidth())
										[
											SNew(SBorder)
											.BorderImage(this, &SQuestTreeGraphNode::GetPinBoxIcon)
											.BorderBackgroundColor(this, &SQuestTreeGraphNode::GetPinBoxColor)
											.Visibility(this, &SQuestTreeGraphNode::GetInputBoxVisibility)
										]
									]

									+SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Top)
									[
										SAssignNew(InputPinBox, SHorizontalBox)
										.RenderTransform(FTransform2D(FQuat2f(), FVector2f(0.0f, -9.0f)))
									]
								]
							]
						]
						// ~ End input pins area

						// ~ Start node info
						+SVerticalBox::Slot()
						.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
						[
							SAssignNew(NodeBody, SBorder)
							.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
							.BorderBackgroundColor(this, &SQuestTreeGraphNode::GetBackgroundColor)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							.Visibility(EVisibility::SelfHitTestInvisible)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Fill)
								[
									SNew(SVerticalBox)

									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)

										// ~ Start node icon
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Top)
										[
											SNew(SBox)
											.WidthOverride(22.0f)
											.HeightOverride(22.0f)
											[
												SNew(SImage)
												.Image(this, &SQuestTreeGraphNode::GetNodeIcon)
											]
										]
										// ~ End node icon

										+ SHorizontalBox::Slot()
										.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
										[
											SNew(SHorizontalBox)

											// ~ Start node title
											+ SHorizontalBox::Slot()
											.FillWidth(1.0f)
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SAssignNew(InlineEditableText, SInlineEditableTextBlock)
													.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
													.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
													//.OnVerifyTextChanged(this, &SQuestTreeGraphNode::OnVerifyNameTextChanged)
													//.OnTextCommitted(this, &SQuestTreeGraphNode::OnNameTextCommited)
													//.IsReadOnly(this, &SQuestTreeGraphNode::IsNameReadOnly)
													//.IsSelected(this, &SQuestTreeGraphNode::IsSelectedExclusively)
													.AutoWrapNonEditText(true)
													.AutoWrapMultilineEditText(true)
												]
												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													NodeTitle.ToSharedRef()
												]
											]
											// ~ End node title
										]
									]

									+ SVerticalBox::Slot()
									.Padding(5.0f)
									.AutoHeight()
									[
										// ~ Start node details
										SNew(SBorder)
										.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
										.BorderBackgroundColor(this, &SQuestTreeGraphNode::GetBackgroundColor)
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Visibility(this, &SQuestTreeGraphNode::GetDetailSlotVisibility)
										[
											SNew(SVerticalBox)

											+ SVerticalBox::Slot()
											.Padding(3.0f, 2.0f)
											.AutoHeight()
											[
												SAssignNew(DetailTitleText, STextBlock)
												.Text(FText::FromString("Details:"))
												.Font(DetailTitleFont)
												.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.9f))
											]

											+ SVerticalBox::Slot()
											.Padding(3.0f, 0.0f, 3.0f, 2.0f)
											.AutoHeight()
											[
												SNew(STextBlock)
												.Text(this, &SQuestTreeGraphNode::GetDetailSlotText)
												.Font(DetailDescriptionFont)
												.AutoWrapText(true)
											]
										]
										// ~ End node details
									]
								]

								+ SOverlay::Slot()
								.HAlign(HAlign_Right)
								.VAlign(VAlign_Fill)
								[
									// Debugger slot when node fails during debugging (PIE)
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
									.BorderBackgroundColor(FLinearColor::Red)
									.Padding(FMargin(4.0f, 0.0f))
									.Visibility(this, &SQuestTreeGraphNode::GetDebuggerSearchFailedMarkerVisibility)
								]
							]
						]
						// ~ End node info

						// ~ Start node compile status area
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(8.0f, 5.0f, 8.0f, 0.0f)
						[
							SNew(SErrorText)
							.Visibility(this, &SQuestTreeGraphNode::GetErrorTextVisibility)
							.ErrorText(GetErrorText())
							.BackgroundColor(GetErrorBackgroundColor())
							.ToolTipText(this, &SQuestTreeGraphNode::GetErrorMsgToolTip)
						]
						// ~ End node compile status area

						// ~ Start output pins area
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							[
								SAssignNew(RightNodeBox, SVerticalBox)
								+ SVerticalBox::Slot()
								.HAlign(HAlign_Fill)
								.VAlign(VAlign_Center)
								.Padding(20.0f, 0.0f)
								.FillHeight(1.0f)
								[
									SNew(SOverlay)

									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Bottom)
									[
										SNew(SBox)
										.HeightOverride(15.0f)
										.WidthOverride(GetOutputPinBoxWidth())
										[
											SNew(SBorder)
											.BorderImage(this, &SQuestTreeGraphNode::GetPinBoxIcon)
											.BorderBackgroundColor(this, &SQuestTreeGraphNode::GetPinBoxColor)
											.Visibility(this, &SQuestTreeGraphNode::GetOutputBoxVisibility)
										]
									]

									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Bottom)
									[
										SAssignNew(OutputPinBox, SHorizontalBox)
										.RenderTransform(FTransform2D(FQuat2f(), FVector2f(0.0f, 9.0f)))
									]
								]
							]
						]
						// ~ End output pins area
					]
				]
			]
		];
		
		// Create comment bubble
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

		GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

		CreatePinWidgets();
}

void SQuestTreeGraphNode::CreatePinWidgets()
{
	for (int32 PinIdx = 0; PinIdx < GraphNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SQuestTreeGraphPin, MyPin)
				.ToolTipText(this, &SQuestTreeGraphNode::GetPinTooltip, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SQuestTreeGraphNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		InputPinBox->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(6.0f, 3.0f)
		[
			PinToAdd
		];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		OutputPinBox->AddSlot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(6.0f, 3.0f)
		[
			PinToAdd
		];

		OutputPins.Add(PinToAdd);
	}
}

FText SQuestTreeGraphNode::GetNodeCompactTitle() const
{
	return GraphNode->GetNodeTitle(ENodeTitleType::FullTitle);
}

FSlateColor SQuestTreeGraphNode::GetBackgroundColor() const
{
	UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode);
	return QuestTreeEdNode->GetBackgroundColor();
}

FSlateColor SQuestTreeGraphNode::GetBorderColor() const
{
	return FLinearColor(0.08f, 0.08f, 0.08f);
}

FSlateColor SQuestTreeGraphNode::GetPinBoxColor() const
{
	return FLinearColor(0.03f, 0.03f, 0.03f, 0.5f);
}

const FSlateBrush* SQuestTreeGraphNode::GetNodeIcon() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
	{
		return QuestTreeEdNode->GetNodeIcon();
	}

	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Root.Icon"));
}

const FSlateBrush* SQuestTreeGraphNode::GetPinBoxIcon() const
{
	return FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body");
}

float SQuestTreeGraphNode::GetInputPinBoxWidth() const
{
	int32 PinCount = 0;
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Input)
			PinCount++;
	}

	return (PinCount * 26.0f) + 40.0f;
}

float SQuestTreeGraphNode::GetOutputPinBoxWidth() const
{
	int32 PinCount = 0;
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Output)
			PinCount++;
	}
	
	return (PinCount * 26.0f) + 40.0f;
}

EVisibility SQuestTreeGraphNode::GetInputBoxVisibility() const
{
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Input)
		{
			return EVisibility::Visible;
		}
	}
	
	return EVisibility::Hidden;
}

EVisibility SQuestTreeGraphNode::GetOutputBoxVisibility() const
{
	for (UEdGraphPin* Pin : GraphNode->Pins)
	{
		if (Pin->Direction == EGPD_Output)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Hidden;
}

EVisibility SQuestTreeGraphNode::GetQuestNameTextVisibility() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
	{
		if (UQuestTreeNode_Quest* AsQuestNode = Cast<UQuestTreeNode_Quest>(QuestTreeEdNode->GetQuestTreeNode()))
		{
			return AsQuestNode->QuestData.Title.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
		}
	}

	return EVisibility::Collapsed;
}

FText SQuestTreeGraphNode::GetQuestNameText() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
	{
		if (UQuestTreeNode_Quest* AsQuestNode = Cast<UQuestTreeNode_Quest>(QuestTreeEdNode->GetQuestTreeNode()))
		{
			if (!AsQuestNode->QuestData.Title.IsEmpty())
			{
				return FText::Format(LOCTEXT("GetQuestNameTextLabel", ": {0}"), AsQuestNode->QuestData.Title);
			}
		}
	}

	return FText();
}

EVisibility SQuestTreeGraphNode::GetDetailSlotVisibility() const
{
	return GetDetailSlotText().IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

FText SQuestTreeGraphNode::GetDetailSlotText() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
		return QuestTreeEdNode->GetQuestTreeNode()->GetNodeDetailText();

	return FText();
}

EVisibility SQuestTreeGraphNode::GetErrorTextVisibility() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
		return QuestTreeEdNode->GetGraphNodeCompileStatus().Num() > 0 ? EVisibility::HitTestInvisible : EVisibility::Collapsed;

	return EVisibility::Collapsed;
}

FText SQuestTreeGraphNode::GetErrorText() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
	{
		for (const FQuestTreeCompileErrorInfo& CompileInfo : QuestTreeEdNode->GetGraphNodeCompileStatus())
		{
			if (CompileInfo.Status == EQuestTreeCompileStatus::Failed)
				return FText::FromString("Error");
		}

		return FText::FromString("Warning");
	}

	return FText();
}

FLinearColor SQuestTreeGraphNode::GetErrorBackgroundColor() const
{
	if (UQuestTreeEdGraphNode* QuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(GraphNode))
	{
		for (const FQuestTreeCompileErrorInfo& CompileInfo : QuestTreeEdNode->GetGraphNodeCompileStatus())
		{
			if (CompileInfo.Status == EQuestTreeCompileStatus::Failed)
				return FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
		}

		return FLinearColor(1.0f, 1.0f, 0.0f, 0.5f);
	}

	return FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
}

EVisibility SQuestTreeGraphNode::GetDebuggerSearchFailedMarkerVisibility() const
{
	return EVisibility::Collapsed;
}

FText SQuestTreeGraphNode::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	FText HoverText = FText::GetEmpty();

	check(GraphPinObj != nullptr);
	UEdGraphNode* OwningGraphNode = GraphPinObj->GetOwningNode();
	if (OwningGraphNode != nullptr)
	{
		FString HoverStr;
		OwningGraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
		if (!HoverStr.IsEmpty())
		{
			HoverText = FText::FromString(HoverStr);
		}
	}

	return HoverText;
}

#undef LOCTEXT_NAMESPACE