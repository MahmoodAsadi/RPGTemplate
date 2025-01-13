// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "QuestTreeEdGraphSchema.generated.h"

class UEdGraph;
class UEdGraphPin;
class UEdGraphNode;
class UToolMenu;
class UGraphNodeContextMenuContext;
class FQuestTreeEditor;
class UQuestTreeEdGraphNode;

/**
 * 
 */
UCLASS()
class QUESTTREEEDITOR_API UQuestTreeEdGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
	
public:

	//~ Begin EdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;
	virtual bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraph) const override;
	//~ End EdGraphSchema Interface

	static TSharedPtr<FQuestTreeEditor> GetQuestTreeGraphEditor(const UEdGraph* InGraph);
	TArray<UQuestTreeEdGraphNode*> GetSelectedNodes(const UEdGraph* InGraph) const;

private:

	static void InitNodeClasses();
	void GetQuestTreeNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = nullptr) const;

	static TArray<UClass*> QuestTreeNodeClasses;
	static bool bQuestTreeNodeClassesInitialized;
};