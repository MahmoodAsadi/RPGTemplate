// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/QuestTreeGraphHelper.h"
#include "Core/QuestTreeHelper.h"
#include "QuestTreeNode.generated.h"

class UEdGraphNode;
class UQuestTreeGraph;
class UQuestTreeNodePin;
class UQuestTreeConditionBase;
class UQuestTreeExecutableEvent;
class UQuestTreeManagerComponent;

/**
 * 
 */
UCLASS(Abstract, HideCategories = Object)
class QUESTTREE_API UQuestTreeNode : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin UQuestTreeNode Interface.
#if WITH_EDITOR
	virtual FText GetNodeTitle() const;
	virtual FSlateColor GetBackgroundColor() const { return FLinearColor(0.1f, 0.1f, 0.1f); }
	virtual FSlateColor GetBorderColor() const { return FLinearColor(0.08f, 0.08f, 0.08f); }
	virtual const FSlateBrush* GetNodeIcon() const;
	virtual FText GetNodeDetailText() const { return FText(); }
	virtual void PreDeleteNode() {}
	virtual void CheckNodeCompileStatus(TArray<FQuestTreeCompileErrorInfo>& FoundIssues) { CachedNodeCompileStatus.Empty(); }
#endif // WITH_EDITOR
	virtual void SetupNode() {}
	//~ End UQuestTreeNode Interface.

#if WITH_EDITOR
	void MakeInputPinsMaker(const TArray<FQuestTreePinMaker>& InPinsInfo);
	void MakeOutputPinsMaker(const TArray<FQuestTreePinMaker>& InPinsInfo);
	void AddInputPinMaker(const FQuestTreePinMaker& InPinInfo);
	void AddOutputPinMaker(const FQuestTreePinMaker& InPinInfo);
	TArray<FQuestTreePinMaker> GetInputPinsMakers() const { return InputPinsMakers; }
	TArray<FQuestTreePinMaker> GetOutputPinsMakers() const { return OutputPinsMakers; }
	void SetGraphNode(UEdGraphNode* InGraphNode) { GraphNode = InGraphNode; }
	UEdGraphNode* GetGraphNode() const { return GraphNode; }
	void SetGraph(UQuestTreeGraph* InGraph) { Graph = InGraph; }
	void SetPosition(const FVector2D& InPoisition) { PositionInGraph = InPoisition; }
	FVector2D GetPosition() { return PositionInGraph; }

	void AddInputPin(UQuestTreeNodePin* InPin) { InputPins.Add(InPin); }
	void AddOutputPin(UQuestTreeNodePin* InPin) { OutputPins.Add(InPin); }
	void ResetPinConnections();
	TArray<FQuestTreeCompileErrorInfo> GetCachedCompileStatus() { return CachedNodeCompileStatus; }
#endif // WITH_EDITOR

	UQuestTreeGraph* GetGraph() const { return Graph; }
	TArray<UQuestTreeNodePin*> GetInputPins() { return InputPins; }
	TArray<UQuestTreeNodePin*> GetOutputPins() { return OutputPins; }
	UQuestTreeNodePin* GetPinByName(const FName& InName) const;
	UQuestTreeNodePin* GetPinByName(const FName& InName, TEnumAsByte<enum EEdGraphPinDirection> InDirection) const;
	void ExecutePin(const FName& InName, UQuestTreeManagerComponent* InQuestManager);

	UPROPERTY()
	bool bUseCustomTitle = false;

	UPROPERTY(EditAnywhere, Category = Description, meta = (EditCondition = "bUseCustomTitle"))
	FString NodeTitle;

	UPROPERTY(EditAnywhere, Category = Description)
	bool bShowDescription = true;

protected:

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	TArray<FQuestTreeCompileErrorInfo> CachedNodeCompileStatus;

private:

	UPROPERTY()
	UEdGraphNode* GraphNode;

	UPROPERTY()
	TArray<FQuestTreePinMaker> InputPinsMakers;

	UPROPERTY()
	TArray<FQuestTreePinMaker> OutputPinsMakers;

	UPROPERTY()
	FVector2D PositionInGraph;
#endif // WITH_EDITORONLY_DATA

	UPROPERTY()
	UQuestTreeGraph* Graph;

	UPROPERTY()
	TArray<UQuestTreeNodePin*> InputPins;

	UPROPERTY()
	TArray<UQuestTreeNodePin*> OutputPins;
};

/**
 * Root of the graph
 */
UCLASS(NotPlaceable, DisplayName = "ROOT")
class QUESTTREE_API UQuestTreeNode_Root : public UQuestTreeNode
{
	GENERATED_UCLASS_BODY()

public:

};

/**
 * Make quest available on trigger
 */
UCLASS(DisplayName = "Quest", Category = "Quest")
class QUESTTREE_API UQuestTreeNode_Quest : public UQuestTreeNode
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest, meta = (ShowOnlyInnerProperties))
	FQuestTreeData QuestData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Condition)
	UQuestTreeConditionBase* Condition;

	virtual void SetupNode() override;

	UFUNCTION()
	void OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager);

	UFUNCTION()
	void OnExecuteInExpirePin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager);

#if WITH_EDITOR
	//~ Begin UQuestTreeNode Interface.
	virtual FText GetNodeTitle() const override;
	virtual const FSlateBrush* GetNodeIcon() const override;
	virtual FText GetNodeDetailText() const override;
	virtual void CheckNodeCompileStatus(TArray<FQuestTreeCompileErrorInfo>& FoundIssues) override;
	//~ End UQuestTreeNode Interface.

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

};

/**
 * Checks the condition(s)
 */
UCLASS(DisplayName = "Branch", Category = "Quest")
class QUESTTREE_API UQuestTreeNode_Branch : public UQuestTreeNode
{
	GENERATED_UCLASS_BODY()

public:

	virtual void SetupNode() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Condition)
	UQuestTreeConditionBase* Condition;

	UFUNCTION()
	void OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager);

#if WITH_EDITOR
	//~ Begin UQuestTreeNode Interface.
	virtual const FSlateBrush* GetNodeIcon() const override;
	virtual FText GetNodeDetailText() const override;
	//~ End UQuestTreeNode Interface.
#endif // WITH_EDITOR
};

/**
 * Execute event
 */
UCLASS(DisplayName = "Event", Category = "Quest")
class QUESTTREE_API UQuestTreeNode_Event : public UQuestTreeNode
{
	GENERATED_UCLASS_BODY()

public:

	virtual void SetupNode() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Event)
	TSubclassOf<UQuestTreeExecutableEvent> EventClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Event)
	FGameplayTag EventTag;

	UFUNCTION()
	void OnExecuteInExecutionPin(UQuestTreeNodePin* Pin, UQuestTreeManagerComponent* InQuestManager);

#if WITH_EDITOR
	//~ Begin UQuestTreeNode Interface.
	/*virtual const FSlateBrush* GetNodeIcon() const override; */
	virtual FText GetNodeDetailText() const override;
	//~ End UQuestTreeNode Interface.
#endif // WITH_EDITOR
};