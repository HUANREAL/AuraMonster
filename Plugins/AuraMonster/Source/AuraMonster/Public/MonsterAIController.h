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

private:
	/** Current behavior state */
	UPROPERTY(VisibleAnywhere, Category = "Monster AI")
	EMonsterBehaviorState CurrentState;

	/** Reference to the controlled monster character */
	UPROPERTY()
	AMonsterCharacter* ControlledMonster;
};
