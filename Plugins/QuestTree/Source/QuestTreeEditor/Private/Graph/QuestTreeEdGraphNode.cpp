// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeEdGraphNode.h"

#include "Misc/TransactionObjectEvent.h"

#include "Core/QuestTreeGraphHelper.h"
#include "Graph/QuestTreeEdGraph.h"
#include "Graph/QuestTreeEdGraphSchema.h"
#include "Graph/QuestTreeGraph.h"
#include "Graph/QuestTreeNode.h"
#include "Graph/QuestTreeNodePin.h"


void UQuestTreeEdGraphNode::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);
	
	TArray<FName> PropertiesChanged = TransactionEvent.GetChangedProperties();
	
	// Having "NodeGuid" means the node is just created (from duplication or direct create), the graph is already being notified, ignore
	if (PropertiesChanged.Contains("NodeGuid"))
		return;

	// NotifyGraphChanged for any of listed events below.
	TArray<FName> NotifyEvents = { "NodeComment", "bCommentBubbleVisible", "bCommentBubblePinned", "NodePosX", "NodePosY" };
	for (const FName& Event : PropertiesChanged)
	{
		if (NotifyEvents.Contains(Event))
		{
			// Make sure to make graph state dirty to save changes properly.
			GetGraph()->NotifyGraphChanged();
			return;
		}
	}
}

void UQuestTreeEdGraphNode::BeginDestroy()
{
	if (FCoreUObjectDelegates::OnObjectPropertyChanged.IsBoundToObject(this))
		FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);

	Super::BeginDestroy();
}

void UQuestTreeEdGraphNode::PostLoad()
{
	Super::PostLoad();
	
	if (!FCoreUObjectDelegates::OnObjectPropertyChanged.IsBoundToObject(this))
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UQuestTreeEdGraphNode::OnNodePropertiesChanged);
}

FText UQuestTreeEdGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return QuestTreeNode->GetNodeTitle();
}

void UQuestTreeEdGraphNode::AllocateDefaultPins()
{
	CreateInputPins();
	CreateOutputPins();
}

void UQuestTreeEdGraphNode::PrepareForCopying()
{
	// Temporarily take ownership of the node, so that it is not deleted when cutting
	QuestTreeNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
}

bool UQuestTreeEdGraphNode::CanDuplicateNode() const
{
	if (QuestTreeNode)
	{
		return !QuestTreeNode->GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable);
	}

	return true;
}

bool UQuestTreeEdGraphNode::CanUserDeleteNode() const
{
	if (QuestTreeNode)
	{
		return !QuestTreeNode->GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable);
	}

	return true;
}

FText UQuestTreeEdGraphNode::GetTooltipText() const
{
	return QuestTreeNode->GetClass()->GetToolTipText();
}

bool UQuestTreeEdGraphNode::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema->IsA(UQuestTreeEdGraphSchema::StaticClass());
}

void UQuestTreeEdGraphNode::DestroyNode()
{
	Super::DestroyNode();
}

FSlateColor UQuestTreeEdGraphNode::GetBackgroundColor() const
{
	return QuestTreeNode->GetBackgroundColor();
}

FSlateColor UQuestTreeEdGraphNode::GetBorderColor() const
{
	return QuestTreeNode->GetBorderColor();
}

const FSlateBrush* UQuestTreeEdGraphNode::GetNodeIcon() const
{
	if (QuestTreeNode)
	{
		return QuestTreeNode->GetNodeIcon();
	}

	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Root.Icon"));
}

void UQuestTreeEdGraphNode::Construct(UQuestTreeNode* InQuestTreeNode)
{
	QuestTreeNode = InQuestTreeNode;
	QuestTreeNode->SetGraphNode(this);
	NodePosX = InQuestTreeNode->GetPosition().X;
	NodePosY = InQuestTreeNode->GetPosition().Y;
	
	if (!FCoreUObjectDelegates::OnObjectPropertyChanged.IsBoundToObject(this))
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UQuestTreeEdGraphNode::OnNodePropertiesChanged);
}

void UQuestTreeEdGraphNode::ResetGraphNodeOwner()
{
	// Make sure QuestTree nodes owner change to the QuestTreeGraph after copying.
	check(QuestTreeNode);
	UQuestTreeGraph* QuestTreeGraph = Cast<UQuestTreeEdGraph>(GetGraph())->GetQuestTreeGraph();
	if (QuestTreeNode->GetOuter() != QuestTreeGraph)
	{
		QuestTreeNode->Rename(NULL, QuestTreeGraph, REN_DontCreateRedirectors);
		QuestTreeNode->SetGraph(QuestTreeGraph);
	}

	QuestTreeNode->SetGraphNode(this);

	if (!FCoreUObjectDelegates::OnObjectPropertyChanged.IsBoundToObject(this))
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UQuestTreeEdGraphNode::OnNodePropertiesChanged);
}

void UQuestTreeEdGraphNode::RestPinConnections()
{
	check(QuestTreeNode);
	QuestTreeNode->ResetPinConnections();
}

void UQuestTreeEdGraphNode::PopulateNodePinConnections(TArray<UEdGraphPin*>& VisitedPins)
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != EGPD_Output)
			continue;

		if (VisitedPins.Contains(Pin))
			continue;
		
		TArray<UQuestTreeEdGraphNode*> LinkedQuestTreeEdNodes;
		UQuestTreeNodePin* CurrentOutputPin = QuestTreeNode->GetPinByName(Pin->PinName, EGPD_Output);
		VisitedPins.Add(Pin);

		for (UEdGraphPin* LinkedToPin : Pin->LinkedTo)
		{
			if (UQuestTreeEdGraphNode* LinkedEdGraphNode = Cast<UQuestTreeEdGraphNode>(LinkedToPin->GetOwningNode()))
			{
				UQuestTreeNode* LinkedQuestTreeNode = LinkedEdGraphNode->GetQuestTreeNode();
				UQuestTreeNodePin* LinkedNodePin = LinkedQuestTreeNode->GetPinByName(LinkedToPin->PinName, EGPD_Input);
				CurrentOutputPin->ConnectedPins.AddUnique(LinkedNodePin);
				LinkedQuestTreeEdNodes.AddUnique(LinkedEdGraphNode);
			}
		}

		for (UQuestTreeEdGraphNode* LinkedNode : LinkedQuestTreeEdNodes)
		{
			LinkedNode->PopulateNodePinConnections(VisitedPins);
		}
	}
}

bool UQuestTreeEdGraphNode::IsConnectedToGraphRoot() const
{
	TArray<const UEdGraphNode*> VisitedNodes;
	return IsConnectedToGraphRoot_Internal(VisitedNodes);
}

bool UQuestTreeEdGraphNode::IsConnectedToGraphRoot_Internal(TArray<const UEdGraphNode*>& VisitedNodes) const
{
	if (Cast<UQuestTreeNode_Root>(QuestTreeNode))
		return true;

	if (VisitedNodes.Contains(this))
		return false;

	VisitedNodes.AddUnique(this);
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != EGPD_Input)
			continue;

		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			if (UQuestTreeEdGraphNode* AsQuestTreeEdNode = Cast<UQuestTreeEdGraphNode>(LinkedPin->GetOwningNode()))
			{
				if (AsQuestTreeEdNode->IsConnectedToGraphRoot_Internal(VisitedNodes))
					return true;
			}
		}
	}

	return false;
}

void UQuestTreeEdGraphNode::CheckGraphNodeCompileStatusRecursively(TArray<const UEdGraphNode*>& VisitedNodes, TArray<FQuestTreeCompileErrorInfo>& FoundIssues) const
{
	if (VisitedNodes.Contains(this))
		return;

	VisitedNodes.AddUnique(this);
	QuestTreeNode->CheckNodeCompileStatus(FoundIssues);

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->Direction != EGPD_Output)
			continue;

		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			if (const UQuestTreeEdGraphNode* QuestTreeGraphNode = Cast<UQuestTreeEdGraphNode>(LinkedPin->GetOwningNode()))
			{
				QuestTreeGraphNode->CheckGraphNodeCompileStatusRecursively(VisitedNodes, FoundIssues);
			}
		}
	}
}

TArray<FQuestTreeCompileErrorInfo> UQuestTreeEdGraphNode::GetGraphNodeCompileStatus() const
{
	return QuestTreeNode->GetCachedCompileStatus();
}

void UQuestTreeEdGraphNode::CreateInputPins()
{
	check(QuestTreeNode);
	
	for (const FQuestTreePinMaker& PinMaker : QuestTreeNode->GetInputPinsMakers())
	{
		UEdGraphPin* GeneratedPin = CreatePin(EGPD_Input, FQuestTreeEditorCommon::GraphPinCategory, NAME_None, nullptr, PinMaker.Name);
		GeneratedPin->PinToolTip = PinMaker.ToolTip;

		// Generate and register QuestTreeNodePin
		UQuestTreeNodePin* GeneratedNodePin = NewObject<UQuestTreeNodePin>(QuestTreeNode, UQuestTreeNodePin::StaticClass(), NAME_None, RF_Transactional);
		GeneratedNodePin->EdGraphPinName = PinMaker.Name;
		GeneratedNodePin->Direction = EGPD_Input;
		GeneratedNodePin->OwningNode = QuestTreeNode;
		QuestTreeNode->AddInputPin(GeneratedNodePin);
	}
}

void UQuestTreeEdGraphNode::CreateOutputPins()
{
	check(QuestTreeNode);

	for (const FQuestTreePinMaker& PinMaker : QuestTreeNode->GetOutputPinsMakers())
	{
		UEdGraphPin* CreatedPin = CreatePin(EGPD_Output, FQuestTreeEditorCommon::GraphPinCategory, NAME_None, nullptr, PinMaker.Name);
		CreatedPin->PinToolTip = PinMaker.ToolTip;

		// Generate and register QuestTreeNodePin
		UQuestTreeNodePin* GeneratedNodePin = NewObject<UQuestTreeNodePin>(QuestTreeNode, UQuestTreeNodePin::StaticClass(), NAME_None, RF_Transactional);
		GeneratedNodePin->EdGraphPinName = PinMaker.Name;
		GeneratedNodePin->Direction = EGPD_Output;
		GeneratedNodePin->OwningNode = QuestTreeNode;
		QuestTreeNode->AddOutputPin(GeneratedNodePin);
	}
}

void UQuestTreeEdGraphNode::OnNodePropertiesChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!QuestTreeNode)
		return;
	
	if (Object == QuestTreeNode)
	{
		if (UQuestTreeEdGraph* QuestTreeEdGraph = Cast<UQuestTreeEdGraph>(GetGraph()))
		{
			QuestTreeEdGraph->MarkCompileStateDirty();
			QuestTreeEdGraph->NotifyGraphChanged();
		}
	}
}