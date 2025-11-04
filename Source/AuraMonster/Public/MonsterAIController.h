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

	/** Maximum distance to check for crawlable surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float CrawlSurfaceDetectionDistance;

	/** Chance (0.0 to 1.0) to transition to a different surface type during crawling patrol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceTransitionChance;

	/** Speed at which the monster rotates to align with new surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float SurfaceAlignmentSpeed;

	/** Distance offset from surface to place the monster when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float CrawlSurfaceOffset;

	/** Distance above current position for fallback floor detection trace start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float FallbackTraceUpDistance;

	/** Distance below current position for fallback floor detection trace end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float FallbackTraceDownDistance;

	/** Minimum pitch angle for random direction generation during normal crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float MinCrawlPitch;

	/** Maximum pitch angle for random direction generation during normal crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float MaxCrawlPitch;

	/** Minimum pitch angle when transitioning between surfaces to favor vertical surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float MinTransitionPitch;

	/** Maximum pitch angle when transitioning between surfaces to favor vertical surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling")
	float MaxTransitionPitch;

	/** Minimum distance multiplier (relative to PatrolRange) for crawl destinations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster AI|Patrol Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinCrawlDistanceMultiplier;

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

	/** Current surface normal the monster is crawling on */
	FVector CurrentSurfaceNormal;

	/** Target surface normal for smooth transitions */
	FVector TargetSurfaceNormal;

	/** Current crawling destination */
	FVector CrawlingDestination;

	/** Whether currently moving to a crawling destination */
	bool bIsMovingToCrawlDestination;

	/** Whether currently transitioning between surfaces */
	bool bIsTransitioningBetweenSurfaces;

	/** Time accumulated while stopped at current crawl destination */
	float CurrentCrawlStopTime;

	/** Target duration to stop at current crawl destination */
	float TargetCrawlStopDuration;

	/** Whether the monster is currently stopped at a crawl destination */
	bool bIsStoppedAtCrawlDestination;

	/** Helper function to get a random value within a validated range */
	float GetValidatedRandomRange(float MinValue, float MaxValue) const;

	/** Find a crawlable surface point near the current location */
	bool FindCrawlableDestination(FVector& OutLocation, FVector& OutSurfaceNormal);

	/** Update monster orientation to align with surface */
	void UpdateSurfaceAlignment(float DeltaTime);

	/** Check if monster has reached the crawl destination */
	bool HasReachedCrawlDestination() const;
};
