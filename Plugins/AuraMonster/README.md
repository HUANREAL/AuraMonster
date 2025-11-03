# Aura Monster Plugin

A comprehensive Unreal Engine 4 plugin for creating AI-controlled monsters with multiple behavior states.

## Features

### Behavior States
The plugin implements three distinct behavior states for monsters:

1. **Idle** - Monster remains stationary with lifelike animations
   - Subtle breathing cycles with smooth intensity variations
   - Random subtle movements (neck twitches, finger shifts)
   - Configurable idle duration before potentially transitioning to patrol
   - Blueprint-implementable animation events
   
2. **Patrol (Standing)** - Monster patrols an area while standing/walking upright
   - Walks with deliberate, heavy pace (configurable via PatrolStandingSpeed)
   - Follows the floor using Unreal Engine's navigation system
   - Selects random reachable destinations within a defined range
   - Walks toward destinations with occasional stops to listen or look around
   - Configurable patrol range, stop duration, and acceptance radius

3. **Patrol (Crawling)** - Monster patrols while crawling on surfaces (floors, walls, ceilings)
   - **Custom pathfinding system** allowing full freedom of movement across all surfaces
   - **Surface detection and attachment** - Monster automatically detects and attaches to nearby surfaces
   - **Smooth surface transitions** - Seamlessly moves between floors, walls, and ceilings
   - **Mid-patrol surface changes** - Unpredictably transitions to different surfaces for added realism
   - Slower movement speed compared to standing patrol (configurable via PatrolCrawlingSpeed)
   - Surface-aware orientation - Monster rotates to align with the surface it's crawling on
   - Configurable surface transition frequency and search distance

### Core Components

#### EMonsterBehaviorState (Enum)
Defines the available behavior states:
- `Idle` - No movement
- `PatrolStanding` - Upright patrol movement
- `PatrolCrawling` - Crawling patrol movement

#### AMonsterCharacter (Character Class)
Base character class for monsters with:
- Behavior state management
- Configurable movement speeds per state
- Blueprint-callable functions for state queries and transitions
- Event callbacks for state changes

**Key Properties:**
- `PatrolStandingSpeed` (default: 300.0) - Movement speed when patrolling while standing
- `PatrolCrawlingSpeed` (default: 150.0) - Movement speed when patrolling while crawling
- `SurfaceTraceDistance` (default: 200.0) - Maximum distance to trace for surface detection when crawling
- `SurfaceAlignmentSpeed` (default: 5.0) - Speed at which the monster rotates to align with surface normal
- `SurfaceChangeThreshold` (default: 0.99) - Dot product threshold for detecting surface normal changes; lower values make the monster more sensitive to surface changes (0.0-1.0)
- `SurfaceOffsetDistance` (default: 50.0) - Offset distance from surface to position the monster when crawling
- `CurrentSurfaceNormal` - Current surface normal the monster is attached to (read-only)
- `bIsAttachedToSurface` - Whether the monster is currently attached to a surface (read-only)

**Key Functions:**
- `GetBehaviorState()` - Returns current behavior state
- `SetBehaviorState(EMonsterBehaviorState)` - Changes behavior state
- `GetMovementSpeedForState(EMonsterBehaviorState)` - Returns movement speed for a given state
- `OnBehaviorStateChanged(OldState, NewState)` - Event called when state changes (Blueprint implementable)
- `OnNeckTwitch()` - Event called to trigger neck twitch animation (Blueprint implementable)
- `OnFingerShift()` - Event called to trigger finger shift animation (Blueprint implementable)
- `OnBreathingUpdate(BreathingIntensity)` - Event called each frame with breathing intensity 0.0-1.0 (Blueprint implementable)
- `GetCurrentSurfaceNormal()` - Returns the current surface normal the monster is attached to
- `IsAttachedToSurface()` - Returns whether the monster is currently attached to a surface
- `GetSurfaceOffsetDistance()` - Returns the surface offset distance used when crawling
- `UpdateSurfaceAttachment(DeltaTime)` - Updates surface attachment and orientation (automatically called when crawling)
- `OnSurfaceTransition(NewSurfaceNormal)` - Event called when transitioning to a new surface (Blueprint implementable)

#### AMonsterAIController (AI Controller)
AI controller that manages monster behavior:
- State machine implementation
- Per-state behavior execution
- State transition management
- Blueprint-extensible behavior functions

**Key Functions:**
- `TransitionToState(EMonsterBehaviorState)` - Transition to a new state
- `GetCurrentState()` - Returns current state
- `ExecuteIdleBehavior(DeltaTime)` - Override to implement idle behavior
- `ExecutePatrolStandingBehavior(DeltaTime)` - Override to implement standing patrol behavior
- `ExecutePatrolCrawlingBehavior(DeltaTime)` - Override to implement crawling patrol behavior
- `OnEnterState(NewState)` - Event called when entering a state
- `OnExitState(OldState)` - Event called when exiting a state

**Idle Behavior Properties:**
- `MinIdleDuration` (default: 5.0) - Minimum seconds to stay idle
- `MaxIdleDuration` (default: 15.0) - Maximum seconds to stay idle
- `MinSubtleMovementInterval` (default: 2.0) - Minimum seconds between subtle movements
- `MaxSubtleMovementInterval` (default: 6.0) - Maximum seconds between subtle movements
- `BreathingCycleDuration` (default: 4.0) - Duration of one breathing cycle in seconds
- `PatrolTransitionChance` (default: 0.3) - Probability (0.0-1.0) of transitioning to patrol after idle duration

**Patrol Behavior Properties:**
- `PatrolRange` (default: 1000.0) - Maximum distance from current position to select patrol destinations
- `MinStopDuration` (default: 2.0) - Minimum seconds to wait at each patrol destination (to listen/look around)
- `MaxStopDuration` (default: 5.0) - Maximum seconds to wait at each patrol destination (to listen/look around)
- `PatrolAcceptanceRadius` (default: 100.0) - How close the monster needs to get to the destination before considering it reached

**Crawling Behavior Properties:**
- `SurfaceTransitionChance` (default: 0.3) - Probability (0.0-1.0) of transitioning to a different surface mid-patrol
- `MinSurfaceTransitionInterval` (default: 3.0) - Minimum seconds between surface transition attempts
- `MaxSurfaceTransitionInterval` (default: 8.0) - Maximum seconds between surface transition attempts
- `MaxSurfaceAngle` (default: 90.0) - Maximum angle in degrees for surface transitions (e.g., 90 for wall climbing)
- `SurfaceSearchDistance` (default: 500.0) - Distance to search for adjacent surfaces when crawling
- `MaxSurfaceSearchAttempts` (default: 8) - Maximum number of random attempts to find a valid crawling surface destination
- `SurfaceTransitionSearchRatio` (default: 0.5) - Ratio of SurfaceSearchDistance to use for surface transition searches (0.0-1.0)
- `SurfaceTransitionAngleThreshold` (default: 0.9) - Dot product threshold for detecting different surfaces; lower values make the monster more sensitive to surface angle changes (0.0-1.0)

## Installation

1. Copy the `Plugins/AuraMonster` folder to your Unreal Engine 4 project's `Plugins` directory
2. If `Plugins` directory doesn't exist, create it in your project root
3. Regenerate project files (right-click .uproject → Generate Visual Studio project files)
4. Open your project in Unreal Engine 4
5. Enable the plugin: Edit → Plugins → Search for "Aura Monster" → Check the box → Restart Editor

## Usage

### In C++

1. **Create a Monster Character:**
```cpp
#include "MonsterCharacter.h"

// In your code
AMonsterCharacter* Monster = GetWorld()->SpawnActor<AMonsterCharacter>(
    AMonsterCharacter::StaticClass(),
    SpawnLocation,
    SpawnRotation
);
```

2. **Set Behavior State:**
```cpp
Monster->SetBehaviorState(EMonsterBehaviorState::PatrolStanding);
```

3. **Create Custom AI Controller:**
```cpp
#include "MonsterAIController.h"

class AMyMonsterAI : public AMonsterAIController
{
    virtual void ExecutePatrolStandingBehavior_Implementation(float DeltaTime) override
    {
        // Implement your patrol logic here
        // e.g., move to waypoints, detect enemies, etc.
    }
};
```

### In Blueprint

1. **Create Monster Blueprint:**
   - Create a new Blueprint class based on `MonsterCharacter`
   - Customize appearance, animations, and properties
   - Set `PatrolStandingSpeed` and `PatrolCrawlingSpeed` as desired

2. **Create AI Controller Blueprint:**
   - Create a new Blueprint class based on `MonsterAIController`
   - Configure idle behavior properties in the Details panel:
     - `Min/Max Idle Duration` - How long to stay idle
     - `Min/Max Subtle Movement Interval` - Frequency of twitches/shifts
     - `Breathing Cycle Duration` - Speed of breathing animation
     - `Patrol Transition Chance` - Likelihood of starting patrol
   - Override behavior functions:
     - `Execute Idle Behavior`
     - `Execute Patrol Standing Behavior`
     - `Execute Patrol Crawling Behavior`
   - Implement your AI logic (waypoint navigation, detection, etc.)

3. **Implement Idle Animations in Monster Blueprint:**
   - Override these events in your Monster Blueprint:
     - `On Neck Twitch` - Play neck twitch animation montage
     - `On Finger Shift` - Play finger/hand shift animation montage
     - `On Breathing Update` - Use breathing intensity to drive skeletal mesh blend or morph target
   - Example: In `On Breathing Update`, multiply BreathingIntensity by a scale factor and apply to chest bone scale

4. **Assign AI Controller:**
   - In your Monster Blueprint, set "AI Controller Class" to your custom AI controller
   - Or set "Auto Possess AI" to "PlacedInWorldOrSpawned"

5. **Change States in Blueprint:**
   ```
   Call "Transition To State" on AI Controller
   Or "Set Behavior State" on Monster Character
   ```

### Example Behavior Implementation

The patrol standing behavior is already implemented by default with the following features:
- Selects random reachable destinations within `PatrolRange`
- Uses Unreal Engine's navigation system to follow the floor
- Walks with deliberate, heavy pace (configured via `PatrolStandingSpeed`)
- Stops at each destination for a random duration between `MinStopDuration` and `MaxStopDuration` to listen/look around

If you want to customize or extend the patrol behavior, you can override `ExecutePatrolStandingBehavior_Implementation`:

```cpp
void AMyMonsterAI::ExecutePatrolStandingBehavior_Implementation(float DeltaTime)
{
    // Call the parent implementation for default patrol behavior
    Super::ExecutePatrolStandingBehavior_Implementation(DeltaTime);
    
    // Add custom behavior here
    // For example: detect nearby players, trigger animations, etc.
}
```

Or implement completely custom patrol logic:

```cpp
void AMyMonsterAI::ExecutePatrolStandingBehavior_Implementation(float DeltaTime)
{
    if (!ControlledMonster) return;
    
    // Custom patrol implementation
    // For example: follow predefined waypoints instead of random locations
}
```

## Extending the Plugin

### Adding New States

To add new behavior states:

1. Add new enum value to `EMonsterBehaviorState` in `MonsterBehaviorState.h`
2. Add case in `AMonsterCharacter::GetMovementSpeedForState()` 
3. Add corresponding property for movement speed
4. Add execute function in `AMonsterAIController`
5. Add case in `AMonsterAIController::Tick()`

### Custom Events

Override these events in Blueprint or C++:
- `OnBehaviorStateChanged` - React to state changes in Monster Character
- `OnEnterState` / `OnExitState` - React to state transitions in AI Controller

## Requirements

- Unreal Engine 4.20 or later
- C++ development tools (Visual Studio)

## License

See LICENSE file in the repository root.
