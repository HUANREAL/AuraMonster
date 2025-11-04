// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBehaviorState.h"
#include "MonsterCharacter.generated.h"

class USurfacePathfindingComponent;

UCLASS()
class AURAMONSTER_API AMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Get the current behavior state */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	EMonsterBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

	/** Set the current behavior state */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void SetBehaviorState(EMonsterBehaviorState NewState);

	/** Get the movement speed for the current behavior state */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	float GetMovementSpeedForState(EMonsterBehaviorState State) const;

	/** Get the surface pathfinding component */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	USurfacePathfindingComponent* GetSurfacePathfinding() const { return SurfacePathfinding; }

protected:
	/** 
	 * Internal method to set behavior state without triggering AI Controller synchronization.
	 * This should only be called by MonsterAIController to avoid circular state updates.
	 * Use SetBehaviorState() for external state changes that need full synchronization.
	 */
	void SetBehaviorStateInternal(EMonsterBehaviorState NewState);

	// Declare MonsterAIController as a friend to allow access to internal methods
	friend class AMonsterAIController;

	/** Called when a subtle neck twitch should be animated */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster|Idle")
	void OnNeckTwitch();
	virtual void OnNeckTwitch_Implementation();

	/** Called when a subtle finger shift should be animated */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster|Idle")
	void OnFingerShift();
	virtual void OnFingerShift_Implementation();

	/** Called to update breathing animation intensity (0.0 to 1.0) */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster|Idle")
	void OnBreathingUpdate(float BreathingIntensity);
	virtual void OnBreathingUpdate_Implementation(float BreathingIntensity);

protected:
	/** Current behavior state of the monster */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
	EMonsterBehaviorState CurrentBehaviorState;

	/** Movement speed when patrolling while standing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Movement")
	float PatrolStandingSpeed;

	/** Movement speed when patrolling while crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Movement")
	float PatrolCrawlingSpeed;

	/** Surface pathfinding component for crawling on walls and ceilings */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Components")
	USurfacePathfindingComponent* SurfacePathfinding;

	/** Called when behavior state changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster")
	void OnBehaviorStateChanged(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);
	virtual void OnBehaviorStateChanged_Implementation(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);
};
