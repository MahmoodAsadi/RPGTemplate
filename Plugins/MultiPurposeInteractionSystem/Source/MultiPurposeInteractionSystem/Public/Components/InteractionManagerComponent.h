// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/InteractableHelper.h"
#include "InteractionManagerComponent.generated.h"

class UInteractionUserWidget;
class UInteractionWidgetComponent;
class UInteractableComponent;

UCLASS(Blueprintable, ShowCategories = (Activation), ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent), Hidecategories = (Object, LOD, Lighting, TextureStreaming))
class MULTIPURPOSEINTERACTIONSYSTEM_API UInteractionManagerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/*
	* Represent max angle between look at direction and direction to the Interactable object to let Interaction enable
	* Zero means, no matter which direction Interaction Direction Source is facing, can Interact even if Interactable is behind of source
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (UIMin = 0.0f, ClampMin = 0.0f, UIMax = 180.0f, ClampMax = 180.0f, Unit = "°"))
	float InteractionHalfAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionSourceDirectionInfo InteractionSourceDirectionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionSourceLocationInfo InteractionSourceLocationInfo;

	// Source location of Interaction to Interactable object if needed to check line of sight between them.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionSourceLocationInfo LineOfSightSourceLocationInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (ShowOnlyInnerProperties))
	FInteractionTraceInfo LineOfSightTraceInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction)
	TSoftClassPtr<UInteractionUserWidget> WidgetClass;

	// Other Primitives Component Tag if needed for Overlap events
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape)
	TArray<FName> ExternalCollisionComponentTag;


public:

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void RegisterInteractable(UInteractableComponent* Component);

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void UnregisterInteractable(UInteractableComponent* Component);

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void InteractionStart();

	UFUNCTION(BlueprintCallable, Category = Interaction)
	void InteractionStop();

	/*
	* Calls when new interactable is available or current one become unavailable
	* Calls on owining client only
	*/
	UPROPERTY(BlueprintAssignable)
	FOnInteractableUpdated OnInteractableUpdated;

	/*
	* Call when interacted with an interactable, before Interact() calls on interactable
	* Call on server and replicated to all clients
	*/
	UPROPERTY(BlueprintAssignable)
	FOnInteractedWithInteractable PreInteractedWithInteractable;

	/*
	* Call when interacted with an interactable, after Interact() calls on interactable
	* Call on server and replicated to all clients
	*/
	UPROPERTY(BlueprintAssignable)
	FOnInteractedWithInteractable PostInteractedWithInteractable;

protected:

	bool IsLocalPlayer() const;
	void CreateInteractionWidget();
	void InteractWithInteractableComponent(UInteractableComponent* Component);
	void EnableInteractionWidget(UInteractableComponent* Component);
	void DisableInteractionWidget();

	void ExecuteInteraction();
	void ResetInteractionCounter();

	void FindBestInteractableCandidate();
	FVector GetInteractionSourceLocation();
	FVector GetLineOfSightSourceLocation();
	FVector GetLookAtDirection();
	bool PerformTrace(const FVector& Start, const FVector& End, AActor* Against);

	UFUNCTION(Server, WithValidation, Reliable)
	void Server_InteractWithInteractableComponent(UInteractableComponent* Component);

	UFUNCTION(NetMultiCast, Reliable)
	void Multi_InteractWithInteractableComponent(UInteractableComponent* Component);

private:

	UPROPERTY()
	UInteractionWidgetComponent* InteractionWidgetComponent;

	UPROPERTY()
	UInteractionUserWidget* InteractionWidget;

	UPROPERTY()
	bool bIsLocalPlayer = false;

	UPROPERTY()
	UInteractableComponent* BestInteractableCandidate;

	UPROPERTY()
	TArray<UInteractableComponent*> Interactables;

	UPROPERTY()
	FTimerHandle DetachWidgetTimerHandle;

	UPROPERTY()
	FTimerHandle HoldInteractionTimerHandle;

	UPROPERTY()
	FTimerHandle ResetInteractionCounterTimerHandle;

	UPROPERTY()
	UInteractableComponent* UsingInteractable;
};