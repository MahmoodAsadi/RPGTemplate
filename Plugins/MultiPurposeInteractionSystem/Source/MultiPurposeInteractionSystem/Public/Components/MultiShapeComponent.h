// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ShapeComponent.h"
#include "MultiShapeComponent.generated.h"

UENUM(BlueprintType)
enum class EBodyShapeType : uint8
{
	Sphere,
	Box,
	Capsule
};

/**
 * A shape component generally can change to Sphere, Box or Capsule as simple collision. Bounds are rendered as lines in the editor.
 */
UCLASS(Blueprintable, ClassGroup = "Collision", Editinlinenew, Hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "Multi Shape Collision", BlueprintSpawnableComponent))
class MULTIPURPOSEINTERACTIONSYSTEM_API UMultiShapeComponent : public UShapeComponent
{
	GENERATED_UCLASS_BODY()

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape)
	EBodyShapeType CollisionShape;
	
	/** The radius of the sphere **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape, meta = (EditCondition = "CollisionShape == EBodyShapeType::Sphere", EditConditionHides))
	float SphereRadius;

	/** The extents (radii dimensions) of the box **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape, meta = (EditCondition = "CollisionShape == EBodyShapeType::Box", EditConditionHides))
	FVector BoxExtent;

	/** Used to control the line thickness when rendering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape, meta = (EditCondition = "CollisionShape == EBodyShapeType::Box", EditConditionHides))
	float LineThickness;

	/**
	 *	Half-height, from center of capsule to the end of top or bottom hemisphere.
	 *	This cannot be less than CapsuleRadius.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape, meta = (EditCondition = "CollisionShape == EBodyShapeType::Capsule", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float CapsuleHalfHeight;

	/**
	 *	Radius of cap hemispheres and center cylinder.
	 *	This cannot be more than CapsuleHalfHeight.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Shape, meta = (EditCondition = "CollisionShape == EBodyShapeType::Capsule", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float CapsuleRadius;

public:

	// Sets the sphere radius without triggering a render or physics update.
	FORCEINLINE void InitSphereRadius(float InSphereRadius) { SphereRadius = InSphereRadius; }

	/**
	 * Change the sphere radius. This is the unscaled radius, before component scale is applied.
	 * @param	InSphereRadius: the new sphere radius
	 * @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Multi Purpose Interactable")
	void SetSphereRadius(float InSphereRadius, bool bUpdateOverlaps = true);

	// @return the radius of the sphere, with component scale applied.
	UFUNCTION(BlueprintCallable, Category = "Components|Multi Purpose Interactable")
	float GetScaledSphereRadius() const;

	// @return the radius of the sphere, ignoring component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Multi Purpose Interactable")
	float GetUnscaledSphereRadius() const;

	// Get the scale used by this shape. This is a uniform scale that is the minimum of any non-uniform scaling.
	// @return the scale used by this shape.
	UFUNCTION(BlueprintCallable, Category = "Components|Multi Purpose Interactable")
	float GetShapeScale() const;

	/**
	 * Change the box extent size. This is the unscaled size, before component scale is applied.
	 * @param	InBoxExtent: new extent (radius) for the box.
	 * @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Box")
	void SetBoxExtent(FVector InBoxExtent, bool bUpdateOverlaps = true);

	// Set the LineThickness
	UFUNCTION(BlueprintCallable, Category = "Components|Box")
	void SetLineThickness(float Thickness);

	// @return the box extent, scaled by the component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Box")
	FVector GetScaledBoxExtent() const;

	// @return the box extent, ignoring component scale.
	UFUNCTION(BlueprintCallable, Category = "Components|Box")
	FVector GetUnscaledBoxExtent() const;

	// Sets the box extents without triggering a render or physics update.
	FORCEINLINE void InitBoxExtent(const FVector& InBoxExtent) { BoxExtent = InBoxExtent; }

	/**
	 * Change the capsule size. This is the unscaled size, before component scale is applied.
	 * @param	InRadius : radius of end-cap hemispheres and center cylinder.
	 * @param	InHalfHeight : half-height, from capsule center to end of top or bottom hemisphere.
	 * @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void SetCapsuleSize(float InRadius, float InHalfHeight, bool bUpdateOverlaps = true);

	/**
	 * Set the capsule radius. This is the unscaled radius, before component scale is applied.
	 * If this capsule collides, updates touching array for owner actor.
	 * @param	Radius : radius of end-cap hemispheres and center cylinder.
	 * @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void SetCapsuleRadius(float Radius, bool bUpdateOverlaps = true);

	/**
	 * Set the capsule half-height. This is the unscaled half-height, before component scale is applied.
	 * If this capsule collides, updates touching array for owner actor.
	 * @param	HalfHeight : half-height, from capsule center to end of top or bottom hemisphere.
	 * @param	bUpdateOverlaps: if true and this shape is registered and collides, updates touching array for owner actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void SetCapsuleHalfHeight(float HalfHeight, bool bUpdateOverlaps = true);

	/**
	 * Returns the capsule radius scaled by the component scale.
	 * @return The capsule radius scaled by the component scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetScaledCapsuleRadius() const;

	/**
	 * Returns the capsule half-height scaled by the component scale. This includes both the cylinder and hemisphere cap.
	 * @return The capsule half-height scaled by the component scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetScaledCapsuleHalfHeight() const;

	/**
	* Returns the capsule half-height minus radius (to exclude the hemisphere), scaled by the component scale.
	* From the center of the capsule this is the vertical distance along the straight cylindrical portion to the point just before the curve of top hemisphere begins.
	* @return The capsule half-height minus radius, scaled by the component scale.
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetScaledCapsuleHalfHeight_WithoutHemisphere() const;

	/**
	 * Returns the capsule radius and half-height scaled by the component scale. Half-height includes the hemisphere end cap.
	 * @param OutRadius Radius of the capsule, scaled by the component scale.
	 * @param OutHalfHeight Half-height of the capsule, scaled by the component scale. Includes the hemisphere end cap.
	 * @return The capsule radius and half-height scaled by the component scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void GetScaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const;

	/**
	 * Returns the capsule radius and half-height scaled by the component scale. Half-height excludes the hemisphere end cap.
	 * @param OutRadius Radius of the capsule, ignoring component scaling.
	 * @param OutHalfHeightWithoutHemisphere Half-height of the capsule, scaled by the component scale. Excludes the hemisphere end cap.
	 * @return The capsule radius and half-height scaled by the component scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void GetScaledCapsuleSize_WithoutHemisphere(float& OutRadius, float& OutHalfHeightWithoutHemisphere) const;

	/**
	 * Returns the capsule radius, ignoring component scaling.
	 * @return the capsule radius, ignoring component scaling.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetUnscaledCapsuleRadius() const { return CapsuleRadius; }

	/**
	 * Returns the capsule half-height, ignoring component scaling. This includes the hemisphere end cap.
	 * @return The capsule radius, ignoring component scaling.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetUnscaledCapsuleHalfHeight() const { return CapsuleHalfHeight; }

	/**
	* Returns the capsule half-height minus radius (to exclude the hemisphere), ignoring component scaling. This excludes the hemisphere end cap.
	* From the center of the capsule this is the vertical distance along the straight cylindrical portion to the point just before the curve of top hemisphere begins.
	* @return The capsule half-height minus radius, ignoring component scaling.
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	float GetUnscaledCapsuleHalfHeight_WithoutHemisphere() const { return CapsuleHalfHeight - CapsuleRadius; }

	/**
	 * Returns the capsule radius and half-height scaled by the component scale. Half-height includes the hemisphere end cap.
	 * @param OutRadius Radius of the capsule, scaled by the component scale.
	 * @param OutHalfHeight Half-height of the capsule, scaled by the component scale. Includes the hemisphere end cap.
	 * @return The capsule radius and half-height scaled by the component scale.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void GetUnscaledCapsuleSize(float& OutRadius, float& OutHalfHeight) const;

	/**
	 * Returns the capsule radius and half-height, ignoring component scaling. Half-height excludes the hemisphere end cap.
	 * @param OutRadius Radius of the capsule, ignoring component scaling.
	 * @param OutHalfHeightWithoutHemisphere Half-height of the capsule, scaled by the component scale. Excludes the hemisphere end cap.
	 * @return The capsule radius and half-height (excluding hemisphere end cap), ignoring component scaling.
	 */
	UFUNCTION(BlueprintCallable, Category = "Components|Capsule")
	void GetUnscaledCapsuleSize_WithoutHemisphere(float& OutRadius, float& OutHalfHeightWithoutHemisphere) const;

	// Sets the capsule size without triggering a render or physics update. This is the preferred method when initializing a component in a class constructor.
	FORCEINLINE void InitCapsuleSize(float InRadius, float InHalfHeight)
	{
		CapsuleRadius = FMath::Max(0.f, InRadius);
		CapsuleHalfHeight = FMath::Max3(0.f, InHalfHeight, InRadius);
	}

	//~ Begin UPrimitiveComponent Interface.
	virtual bool IsZeroExtent() const override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual struct FCollisionShape GetCollisionShape(float Inflation = 0.0f) const override;
	virtual bool AreSymmetricRotations(const FQuat& A, const FQuat& B, const FVector& Scale3D) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void CalcBoundingCylinder(float& CylinderRadius, float& CylinderHalfHeight) const override;
	//~ End USceneComponent Interface

	//~ Begin UShapeComponent Interface
	virtual void UpdateBodySetup() override;
	//~ End UShapeComponent Interface

	//~ Begin UObject Interface
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
#endif // WITH_EDITOR
	//~ End UObject Interface

};