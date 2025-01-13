// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MultiShapeComponent.h"
#include "Core/InteractableHelper.h"
#include "InteractableComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ShowCategories = (Activation), ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent), Hidecategories = (Object, LOD, Lighting, TextureStreaming))
class MULTIPURPOSEINTERACTIONSYSTEM_API UInteractableComponent : public UMultiShapeComponent
{
	GENERATED_UCLASS_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

public:

	// Interaction Trigger Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionTriggerInfo InteractionTriggerInfo;

	// Visual presentation of Interaction when it is available
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionVisualInfo InteractionVisualPresentationInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
	TArray<FName> ExternalCollisionTags;

	// If true, check line of sight from Interaction Manager source location and this component
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
	bool bCheckLineOfSight;

	/*
	* Represent max angle the Interacation Manager can Interact from forward vector of this Interactable object
	* Zero mean Interaction Manager can interact with this Interactable from any angle
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties, UIMin = 0.0f, ClampMin = 0.0f, UIMax = 180.0f, ClampMax = 180.0f, Unit = "°"))
	float AcceptableInteractionHalfAngle = 0.0f;

	UPROPERTY(BlueprintAssignable)
	FOnInteractionActivate OnInteractionActivate;

	UPROPERTY(BlueprintAssignable)
	FOnInteractableInteracted OnInteractableInteracted;

public:

	UFUNCTION(BlueprintCallable, Category = Interactable)
	virtual void SetInteractionEnable(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = Interactable)
	virtual void Interact(UObject* Instigator);

protected:

	UPROPERTY()
	TArray<UPrimitiveComponent*> ExternalComponents;

	void GatherExternalCollisions();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};