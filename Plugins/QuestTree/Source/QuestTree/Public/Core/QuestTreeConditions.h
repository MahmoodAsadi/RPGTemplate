// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "QuestTreeConditions.generated.h"

class UQuestTreeManagerComponent;

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class QUESTTREE_API UQuestTreeConditionBase : public UObject
{
	GENERATED_BODY()
	
public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const { return true; }

#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR
};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "AnyConditionsMatch"))
class QUESTTREE_API UQuestTreeCondition_AnyConditionsMatch : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Quest)
	TArray<UQuestTreeConditionBase*> Conditions;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "AllConditionsMatch"))
class QUESTTREE_API UQuestTreeCondition_AllConditionsMatch : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Quest)
	TArray<UQuestTreeConditionBase*> Conditions;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "NoConditionsMatch"))
class QUESTTREE_API UQuestTreeCondition_NoConditionsMatch : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = Quest)
	TArray<UQuestTreeConditionBase*> Conditions;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "ActiveQuest"))
class QUESTTREE_API UQuestTreeCondition_ActiveQuest : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTagQuery ActiveQuestQuery;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "CompletedQuest"))
class QUESTTREE_API UQuestTreeCondition_CompletedQuest : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTagQuery CompletedQuestQuery;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "ExpiredQuest"))
class QUESTTREE_API UQuestTreeCondition_ExpiredQuest : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTagQuery ExpiredQuestQuery;

};

UCLASS(EditInlineNew, NotBlueprintable, meta = (DisplayName = "ExecutedEvent"))
class QUESTTREE_API UQuestTreeCondition_ExecutedEvent : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override;
#if WITH_EDITOR
	virtual FString GetDescription() const;
#endif // WITH_EDITOR

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Quest)
	FGameplayTagQuery ExecutedEventQuery;

};

UCLASS(Abstract, EditInlineNew, Blueprintable)
class QUESTTREE_API UQuestTreeCondition_Custom : public UQuestTreeConditionBase
{
	GENERATED_BODY()

public:

	virtual bool PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const override { return K2_PerformQuestConditionCheck(InQuestManager); }

	UFUNCTION(BlueprintImplementableEvent, Category = Quest, meta = (DisplayName = "PerformQuestConditionCheck"))
	bool K2_PerformQuestConditionCheck(const UQuestTreeManagerComponent* InQuestManager) const;

};