// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestTreeEdGraphNode.generated.h"

class UQuestTreeNode;

/**
 * 
 */
UCLASS()
class QUESTTREEEDITOR_API UQuestTreeEdGraphNode : public UEdGraphNode
{
	GENERATED_BODY()
	
public:

	// ~Begin UObject interface
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override;
	virtual void BeginDestroy() override;
	virtual void PostLoad() override;
	// ~End UObject interface

	//~ Begin UEdGraphNode Interface.
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void AllocateDefaultPins() final override;
	virtual void PrepareForCopying() override;
	virtual bool CanDuplicateNode() const override;
	virtual bool CanUserDeleteNode() const override;
	virtual FText GetTooltipText() const override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	virtual void DestroyNode() override;
	//~ End UEdGraphNode Interface.

	//~ Begin UQuestTreeEdGraphNode Interface.
	virtual FSlateColor GetBackgroundColor() const;
	virtual FSlateColor GetBorderColor() const;
	const FSlateBrush* GetNodeIcon() const;
	//~ End UQuestTreeEdGraphNode Interface.

	void Construct(UQuestTreeNode* InQuestTreeNode);
	UQuestTreeNode* GetQuestTreeNode() const { return QuestTreeNode; }
	void ResetGraphNodeOwner();

	void RestPinConnections();
	void PopulateNodePinConnections(TArray<UEdGraphPin*>& VisitedPins);
	bool IsConnectedToGraphRoot() const;
	bool IsConnectedToGraphRoot_Internal(TArray<const UEdGraphNode*>& VisitedNodes) const;
	void CheckGraphNodeCompileStatusRecursively(TArray<const UEdGraphNode*>& VisitedNodes, TArray<FQuestTreeCompileErrorInfo>& FoundIssues) const;
	TArray<FQuestTreeCompileErrorInfo> GetGraphNodeCompileStatus() const;

protected:

	void CreateInputPins();
	void CreateOutputPins();

	void OnNodePropertiesChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

	UPROPERTY()
	UQuestTreeNode* QuestTreeNode;
};