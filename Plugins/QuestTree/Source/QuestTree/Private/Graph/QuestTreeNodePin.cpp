// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/QuestTreeNodePin.h"
#include "Components/QuestTreeManagerComponent.h"


void UQuestTreeNodePin::ExecutePin(UQuestTreeManagerComponent* InQuestManager)
{
	for (UQuestTreeNodePin* LinkedPin : ConnectedPins)
	{
		if (LinkedPin->OnExecutePin.IsBound())
		{
			LinkedPin->OnExecutePin.Broadcast(LinkedPin, InQuestManager);
		}
	}
}