// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurfacePathfindingComponent.generated.h"

/**
 * Component that enables monsters to crawl across any surface (floors, walls, ceilings)
 * with smooth transitions between surfaces.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURAMONSTER_API USurfacePathfindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurfacePathfindingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Find a random valid surface location within the specified range
	 * @param OriginLocation Starting point for the search
	 * @param Range Maximum distance to search
	 * @param OutLocation The found surface location
	 * @param OutNormal The surface normal at that location
	 * @return True if a valid surface location was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Surface Pathfinding")
	bool GetRandomSurfaceLocation(const FVector& OriginLocation, float Range, FVector& OutLocation, FVector& OutNormal);

	/**
	 * Move the owner actor toward a target location while maintaining surface attachment
	 * @param TargetLocation The destination to move toward
	 * @param DeltaTime Time step for this movement update
	 * @param Speed Movement speed in units per second
	 * @return True if still moving toward target, false if reached
	 */
	UFUNCTION(BlueprintCallable, Category = "Surface Pathfinding")
	bool MoveTowardsSurfaceLocation(const FVector& TargetLocation, float DeltaTime, float Speed);

	/**
	 * Check if the owner is currently attached to a valid surface
	 */
	UFUNCTION(BlueprintCallable, Category = "Surface Pathfinding")
	bool IsOnValidSurface() const;

	/**
	 * Get the current surface normal the actor is attached to
	 */
	UFUNCTION(BlueprintCallable, Category = "Surface Pathfinding")
	FVector GetCurrentSurfaceNormal() const { return CurrentSurfaceNormal; }

	/**
	 * Chance (0.0 to 1.0) that the monster will attempt to transition to a different surface type mid-patrol
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Pathfinding", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceTransitionChance;

	/**
	 * How far to trace when detecting surfaces
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Pathfinding")
	float SurfaceDetectionRange;

	/**
	 * How quickly to rotate to align with new surfaces (higher = faster)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Pathfinding")
	float SurfaceAlignmentSpeed;

	/**
	 * Minimum angle difference (degrees) required to trigger a surface transition
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Pathfinding")
	float MinTransitionAngle;

	/**
	 * Distance threshold to consider target location reached
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Pathfinding")
	float AcceptanceRadius;

protected:
	/**
	 * Detect the nearest surface below/around the given location
	 * @param Location Point to check from
	 * @param OutHitLocation Where the surface was hit
	 * @param OutHitNormal The normal of the hit surface
	 * @return True if a surface was found
	 */
	bool DetectSurface(const FVector& Location, FVector& OutHitLocation, FVector& OutHitNormal);

	/**
	 * Smoothly align the actor's rotation to match the surface normal
	 * @param TargetNormal The surface normal to align with
	 * @param DeltaTime Time step
	 */
	void AlignToSurface(const FVector& TargetNormal, float DeltaTime);

	/**
	 * Check if should attempt a surface transition based on probability
	 */
	bool ShouldAttemptSurfaceTransition() const;

private:
	/** Currently tracked surface normal */
	FVector CurrentSurfaceNormal;

	/** Whether the actor is currently on a valid surface */
	bool bIsOnSurface;

	/** Cached reference to the owner actor */
	UPROPERTY()
	AActor* CachedOwner;
};
