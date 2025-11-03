// Copyright Epic Games, Inc. All Rights Reserved.

#include "MonsterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
 	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize default state
	CurrentBehaviorState = EMonsterBehaviorState::Idle;

	// Set default movement speeds
	PatrolStandingSpeed = 300.0f;
	PatrolCrawlingSpeed = 150.0f;

	// Initialize crawling properties
	SurfaceTraceDistance = 200.0f;
	SurfaceAlignmentSpeed = 5.0f;
	SurfaceChangeThreshold = 0.99f;
	SurfaceOffsetDistance = 50.0f;
	CurrentSurfaceNormal = FVector::UpVector;
	bIsAttachedToSurface = false;
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize movement speed based on initial state
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = GetMovementSpeedForState(CurrentBehaviorState);
	}
}

// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update surface attachment when in crawling mode
	if (CurrentBehaviorState == EMonsterBehaviorState::PatrolCrawling)
	{
		UpdateSurfaceAttachment(DeltaTime);
	}
}

// Called to bind functionality to input
void AMonsterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMonsterCharacter::SetBehaviorState(EMonsterBehaviorState NewState)
{
	if (CurrentBehaviorState != NewState)
	{
		EMonsterBehaviorState OldState = CurrentBehaviorState;
		CurrentBehaviorState = NewState;

		// Update movement speed based on new state
		if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = GetMovementSpeedForState(NewState);
		}

		// Notify about state change
		OnBehaviorStateChanged(OldState, NewState);
	}
}

float AMonsterCharacter::GetMovementSpeedForState(EMonsterBehaviorState State) const
{
	switch (State)
	{
		case EMonsterBehaviorState::Idle:
			return 0.0f;
		
		case EMonsterBehaviorState::PatrolStanding:
			return PatrolStandingSpeed;
		
		case EMonsterBehaviorState::PatrolCrawling:
			return PatrolCrawlingSpeed;
		
		default:
			return 0.0f;
	}
}

void AMonsterCharacter::OnBehaviorStateChanged_Implementation(EMonsterBehaviorState OldState, EMonsterBehaviorState NewState)
{
	// Default implementation - can be overridden in Blueprint or subclasses
}

void AMonsterCharacter::OnNeckTwitch_Implementation()
{
	// Default implementation - can be overridden in Blueprint or subclasses to trigger neck twitch animation
}

void AMonsterCharacter::OnFingerShift_Implementation()
{
	// Default implementation - can be overridden in Blueprint or subclasses to trigger finger shift animation
}

void AMonsterCharacter::OnBreathingUpdate_Implementation(float BreathingIntensity)
{
	// Default implementation - can be overridden in Blueprint or subclasses to update breathing animation
}

void AMonsterCharacter::UpdateSurfaceAttachment(float DeltaTime)
{
	if (!GetWorld())
	{
		return;
	}

	// Trace downward from the character to detect the surface
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation - GetActorUpVector() * SurfaceTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		bIsAttachedToSurface = true;
		FVector NewSurfaceNormal = HitResult.ImpactNormal;

		// Check if surface normal has changed significantly
		float DotProduct = FVector::DotProduct(CurrentSurfaceNormal, NewSurfaceNormal);
		if (DotProduct < SurfaceChangeThreshold)
		{
			// Surface has changed, trigger transition event
			OnSurfaceTransition(NewSurfaceNormal);
			CurrentSurfaceNormal = NewSurfaceNormal;
		}

		// Smoothly rotate to align with surface normal
		FRotator CurrentRotation = GetActorRotation();
		FRotator TargetRotation = NewSurfaceNormal.Rotation();
		
		// Adjust pitch and roll to align with surface, preserve yaw for movement direction
		TargetRotation.Yaw = CurrentRotation.Yaw;
		
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SurfaceAlignmentSpeed);
		SetActorRotation(NewRotation);
	}
	else
	{
		// No surface detected, gradually return to default orientation
		bIsAttachedToSurface = false;
		FVector DefaultNormal = FVector::UpVector;
		CurrentSurfaceNormal = FMath::VInterpTo(CurrentSurfaceNormal, DefaultNormal, DeltaTime, SurfaceAlignmentSpeed * 0.5f);
		
		FRotator CurrentRotation = GetActorRotation();
		FRotator DefaultRotation = FRotator::ZeroRotator;
		DefaultRotation.Yaw = CurrentRotation.Yaw;
		
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DefaultRotation, DeltaTime, SurfaceAlignmentSpeed * 0.5f);
		SetActorRotation(NewRotation);
	}
}

void AMonsterCharacter::OnSurfaceTransition_Implementation(const FVector& NewSurfaceNormal)
{
	// Default implementation - can be overridden in Blueprint or subclasses
	// to trigger animations or effects when transitioning to a new surface
}
