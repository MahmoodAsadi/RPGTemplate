// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "Core/QuestTreeHelper.h"
#include "QuestTreeEdGraph.generated.h"

class UQuestTreeGraph;
class UQuestTreeNode;
class UQuestTreeEdGraphNode;

/**
 * 
 */
UCLASS()
class QUESTTREEEDITOR_API UQuestTreeEdGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()
	
public:

	/** UObject Implementation */
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	virtual void PostEditUndo() override;
	virtual void PostInitProperties() override;
	virtual void NotifyGraphChanged() override;
	/** End UObject */

	void MarkCompileStateDirty();
	void ConstructEdGraphNode(UQuestTreeNode* InGraphNode);
	void SetQuestTreeGraph(UQuestTreeGraph* InGraph);
	UQuestTreeGraph* GetQuestTreeGraph() const;
	void SetEdGraphRootNode(UQuestTreeEdGraphNode* InRoot) { GraphRootNode = InRoot; }
	UQuestTreeEdGraphNode* GetEdGraphRootNode() { return GraphRootNode; }
	EQuestTreeCompileStatus CompileGraph(TArray<FQuestTreeCompileErrorInfo>& OutIssues);
	EQuestTreeCompileStatus FindGraphCompileStatusFromFoundIssues(const TArray<FQuestTreeCompileErrorInfo>& FoundIssues);

private:

	UPROPERTY()
	UQuestTreeGraph* QuestGraph;

	UPROPERTY()
	UQuestTreeEdGraphNode* GraphRootNode;
};
