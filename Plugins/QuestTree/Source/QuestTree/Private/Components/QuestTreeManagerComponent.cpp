// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/QuestTreeManagerComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "Components/QuestTreeGiverComponent.h"
#include "Core/QuestTreeSubsystem.h"
#include "Graph/QuestTreeGraph.h"


// Sets default values for this component's properties
UQuestTreeManagerComponent::UQuestTreeManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(false);

	// ...
}

// Called when the game starts
void UQuestTreeManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsOwningClient())
		return;

	if (UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this))
		QuestManagerSubsystem->RegisterQuestManager(this);
}

void UQuestTreeManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this))
		QuestManagerSubsystem->UnregisterQuestManager();

	Super::EndPlay(EndPlayReason);
}

void UQuestTreeManagerComponent::AddAvailableQuest(const FQuestTreeData& InQuestData, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->AddAvailableQuest(InQuestData, bNotifyGraph);
}

void UQuestTreeManagerComponent::ActivateQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->ActivateQuest(InQuestTag, bNotifyGraph);
}

void UQuestTreeManagerComponent::CompleteQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->CompleteQuest(InQuestTag, bNotifyGraph);
}

void UQuestTreeManagerComponent::FailQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->FailQuest(InQuestTag, bNotifyGraph);
}

void UQuestTreeManagerComponent::ExpireQuest(const FGameplayTag& InQuestTag, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->ExpireQuest(InQuestTag, bNotifyGraph);
}

EQuestTreeStatus UQuestTreeManagerComponent::GetQuestStatus(const FGameplayTag& InQuestTag) const
{
	if (!IsOwningClient())
		return EQuestTreeStatus::None;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->GetQuestStatus(InQuestTag);
}

void UQuestTreeManagerComponent::ExecuteQuestEvent(const FGameplayTag& InEventTag, bool bNotifyGraph)
{
	if (!IsOwningClient())
		return;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	QuestManagerSubsystem->ExecuteQuestEvent(InEventTag, bNotifyGraph);
}

bool UQuestTreeManagerComponent::DoesActiveQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	if (!IsOwningClient())
		return false;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->DoesActiveQuestsMatchesQuery(InQuestQuery);
}

bool UQuestTreeManagerComponent::DoesCompletedQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	if (!IsOwningClient())
		return false;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->DoesCompletedQuestsMatchesQuery(InQuestQuery);
}

bool UQuestTreeManagerComponent::DoesExpiredQuestsMatchesQuery(const FGameplayTagQuery& InQuestQuery) const
{
	if (!IsOwningClient())
		return false;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->DoesExpiredQuestsMatchesQuery(InQuestQuery);
}

bool UQuestTreeManagerComponent::DoesExecutedEventsMatchesQuery(const FGameplayTagQuery& InEventQuery) const
{
	if (!IsOwningClient())
		return false;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->DoesExecutedEventsMatchesQuery(InEventQuery);
}

bool UQuestTreeManagerComponent::IsEventExecuted(const FGameplayTag& InEventTag) const
{
	if (!IsOwningClient())
		return false;

	UQuestTreeSubsystem* QuestManagerSubsystem = UQuestTreeSubsystem::Get(this);
	return QuestManagerSubsystem->IsEventExecuted(InEventTag);
}

bool UQuestTreeManagerComponent::IsOwningClient() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		return OwnerPawn->GetController()->IsLocalController();

	if (AController* OwnerController = Cast<AController>(GetOwner()))
		return OwnerController->IsLocalController();

	if (APlayerState* OwnerPlayerState = Cast<APlayerState>(GetOwner()))
	{
		if (AController* OwnerController = OwnerPlayerState->GetOwningController())
		{
			return OwnerController->IsLocalController();
		}

		return false;
	}

	ensureMsgf(nullptr, TEXT("[%s]: Owner [%s] is not a PlayerPawn, PlayerController or PlayerState, make sure to attach the component to one of those"), *GetName(), *GetOwner()->GetName());
	return false;
}