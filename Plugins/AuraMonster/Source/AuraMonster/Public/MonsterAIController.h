// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterBehaviorState.h"
#include "MonsterAIController.generated.h"

class AMonsterCharacter;

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

	/** Helper function to get a random value within a validated range */
	float GetValidatedRandomRange(float MinValue, float MaxValue) const;
};
