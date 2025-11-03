// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterBehaviorState.h"
#include "MonsterAIController.generated.h"

class AMonsterCharacter;
class UNavigationSystemV1;
class UPathFollowingComponent;

/**
 * AI Controller for managing monster behavior and state transitions
 */
UCLASS()
class AURAMONSTER_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMonsterAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/** Transition to a new behavior state */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	void TransitionToState(EMonsterBehaviorState NewState);

	/** Get the current behavior state */
	UFUNCTION(BlueprintCallable, Category = "Monster AI")
	EMonsterBehaviorState GetCurrentState() const { return CurrentState; }

protected:
	/** Execute behavior for the idle state */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster AI")
	void ExecuteIdleBehavior(float DeltaTime);
	virtual void ExecuteIdleBehavior_Implementation(float DeltaTime);

	/** Execute behavior for the patrol standing state */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster AI")
	void ExecutePatrolStandingBehavior(float DeltaTime);
	virtual void ExecutePatrolStandingBehavior_Implementation(float DeltaTime);

	/** Execute behavior for the patrol crawling state */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster AI")
	void ExecutePatrolCrawlingBehavior(float DeltaTime);
	virtual void ExecutePatrolCrawlingBehavior_Implementation(float DeltaTime);

	/** Called when entering a new state */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster AI")
	void OnEnterState(EMonsterBehaviorState NewState);
	virtual void OnEnterState_Implementation(EMonsterBehaviorState NewState);

	/** Called when exiting a state */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster AI")
	void OnExitState(EMonsterBehaviorState OldState);
	virtual void OnExitState_Implementation(EMonsterBehaviorState OldState);

protected:
	/** Minimum time in seconds to stay idle before potentially transitioning to patrol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle")
	float MinIdleDuration;

	/** Maximum time in seconds to stay idle before potentially transitioning to patrol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle")
	float MaxIdleDuration;

	/** Minimum time in seconds between subtle movements (neck twitches, finger shifts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle")
	float MinSubtleMovementInterval;

	/** Maximum time in seconds between subtle movements (neck twitches, finger shifts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle")
	float MaxSubtleMovementInterval;

	/** Breathing cycle duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle")
	float BreathingCycleDuration;

	/** Chance (0.0 to 1.0) that monster will transition to patrol after idle duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Idle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PatrolTransitionChance;

	/** Maximum distance from current position to select patrol destinations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol")
	float PatrolRange;

	/** Minimum time in seconds to wait at each patrol destination (to listen/look around) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol")
	float MinStopDuration;

	/** Maximum time in seconds to wait at each patrol destination (to listen/look around) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol")
	float MaxStopDuration;

	/** Acceptance radius in units - how close the monster needs to get to the destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol")
	float PatrolAcceptanceRadius;

	/** Chance (0.0 to 1.0) to transition to a different surface mid-patrol when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceTransitionChance;

	/** Minimum time in seconds between surface transition attempts when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling")
	float MinSurfaceTransitionInterval;

	/** Maximum time in seconds between surface transition attempts when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling")
	float MaxSurfaceTransitionInterval;

	/** Maximum angle in degrees for surface transitions (e.g., 90 for wall climbing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling")
	float MaxSurfaceAngle;

	/** Distance to search for adjacent surfaces when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling")
	float SurfaceSearchDistance;

	/** Maximum number of random attempts to find a valid crawling surface destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling", meta = (ClampMin = "1", ClampMax = "20"))
	int32 MaxSurfaceSearchAttempts;

	/** Ratio of SurfaceSearchDistance to use for surface transition searches (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceTransitionSearchRatio;

	/** Threshold for detecting different surfaces (0.0-1.0, lower = more sensitive to angle changes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceTransitionAngleThreshold;

private:
	/** Current behavior state */
	UPROPERTY(VisibleAnywhere, Category = "Monster AI")
	EMonsterBehaviorState CurrentState;

	/** Reference to the controlled monster character */
	UPROPERTY()
	AMonsterCharacter* ControlledMonster;

	/** Time accumulated in current idle period */
	float CurrentIdleTime;

	/** Target idle duration for current idle period */
	float TargetIdleDuration;

	/** Time accumulated since last subtle movement */
	float TimeSinceLastSubtleMovement;

	/** Target time until next subtle movement */
	float NextSubtleMovementTime;

	/** Current time in breathing cycle */
	float BreathingCycleTime;

	/** Time accumulated while stopped at current patrol destination */
	float CurrentStopTime;

	/** Target duration to stop at current patrol destination */
	float TargetStopDuration;

	/** Whether the monster is currently stopped and listening/looking around */
	bool bIsStoppedAtDestination;

	/** Cached reference to navigation system */
	UPROPERTY()
	UNavigationSystemV1* CachedNavSystem;

	/** Cached reference to path following component */
	UPROPERTY()
	UPathFollowingComponent* CachedPathFollowingComp;

	/** Time accumulated since last surface transition check when crawling */
	float TimeSinceSurfaceTransitionCheck = 0.0f;

	/** Target time until next surface transition check when crawling */
	float NextSurfaceTransitionCheckTime = 0.0f;

	/** Current crawling destination for surface-aware pathfinding */
	FVector CurrentCrawlingDestination;

	/** Whether a valid crawling destination has been set */
	bool bHasCrawlingDestination;

	/** Cached surface offset distance from the controlled monster */
	float CachedSurfaceOffsetDistance = 50.0f;

	/** Cached cosine of MaxSurfaceAngle for performance optimization */
	float CachedMaxSurfaceAngleCos = 0.0f;

	/** Helper function to get a random value within a validated range */
	float GetValidatedRandomRange(float MinValue, float MaxValue) const;

	/** Find a crawling destination on nearby surfaces (walls, ceilings, floors) */
	bool FindCrawlingSurfaceDestination(FVector& OutDestination);

	/** Attempt to transition to an adjacent surface for unpredictability */
	void AttemptSurfaceTransition();
};
