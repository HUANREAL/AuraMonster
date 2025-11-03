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

	/** Called when behavior state changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Monster")
	void OnBehaviorStateChanged(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);
	virtual void OnBehaviorStateChanged_Implementation(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState);
};
