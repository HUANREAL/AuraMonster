// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBehaviorState.h"
#include "MonsterCharacter.generated.h"

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

	/** Get the current surface normal the monster is attached to */
	UFUNCTION(BlueprintCallable, Category = "Monster|Crawling")
	FVector GetCurrentSurfaceNormal() const { return CurrentSurfaceNormal; }

	/** Check if monster is currently attached to a surface */
	UFUNCTION(BlueprintCallable, Category = "Monster|Crawling")
	bool IsAttachedToSurface() const { return bIsAttachedToSurface; }

	/** Update surface attachment and orientation for crawling */
	UFUNCTION(BlueprintCallable, Category = "Monster|Crawling")
	void UpdateSurfaceAttachment(float DeltaTime);

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

	/** Maximum distance to trace for surface detection when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Crawling")
	float SurfaceTraceDistance;

	/** Speed at which the monster rotates to align with surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Crawling")
	float SurfaceAlignmentSpeed;

	/** Threshold for detecting surface normal changes (0.0-1.0, higher = more sensitive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Crawling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceChangeThreshold;

	/** Offset distance from surface to position the monster when crawling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Crawling")
	float SurfaceOffsetDistance;

	/** Current surface normal the monster is attached to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Crawling")
	FVector CurrentSurfaceNormal;

	/** Whether the monster is currently attached to a surface */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Crawling")
	bool bIsAttachedToSurface;

	/** Called when behavior state changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster")
	void OnBehaviorStateChanged(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);
	virtual void OnBehaviorStateChanged_Implementation(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);

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

	/** Called when the monster transitions to a new surface */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster|Crawling")
	void OnSurfaceTransition(const FVector& NewSurfaceNormal);
	virtual void OnSurfaceTransition_Implementation(const FVector& NewSurfaceNormal);
};
