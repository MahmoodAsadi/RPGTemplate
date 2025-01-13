// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EdGraph/EdGraphNode.h"
#include "QuestTreeNodePin.generated.h"

class UEdGraphPin;
class UQuestTreeNode;
class UQuestTreeManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecutePin, UQuestTreeNodePin*, Pin, UQuestTreeManagerComponent*, InQuestManager);

/**
 * 
 */
UCLASS()
class QUESTTREE_API UQuestTreeNodePin : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	TEnumAsByte<enum EEdGraphPinDirection> Direction;

	UPROPERTY()
	FName EdGraphPinName;

	UPROPERTY()
	TArray<UQuestTreeNodePin*> ConnectedPins;

	UPROPERTY()
	FOnExecutePin OnExecutePin;

	UFUNCTION()
	void ExecutePin(UQuestTreeManagerComponent* InQuestManager);

	UPROPERTY()
	UQuestTreeNode* OwningNode;

};