# AuraMonster Plugin - Quick Start Guide

## Quick Implementation Examples

### 1. Basic Setup in Blueprint

**Step 1: Create Monster Blueprint**
1. In Content Browser: Right-click → Blueprint Class
2. Search for and select "MonsterCharacter"
3. Name it "BP_MyMonster"

**Step 2: Create AI Controller Blueprint**
1. In Content Browser: Right-click → Blueprint Class
2. Search for and select "MonsterAIController"
3. Name it "BP_MyMonsterAI"

**Step 3: Link AI to Monster**
1. Open "BP_MyMonster"
2. In Details panel, find "Pawn" section
3. Set "AI Controller Class" to "BP_MyMonsterAI"
4. Set "Auto Possess AI" to "Placed in World or Spawned"

**Step 4: Configure Movement Speeds**
1. In "BP_MyMonster", find "Monster | Movement" section
2. Set "Patrol Standing Speed": 300.0 (or your preferred speed)
3. Set "Patrol Crawling Speed": 150.0 (or your preferred speed)

### 2. Implementing Patrol Logic in Blueprint

**In BP_MyMonsterAI:**

1. Override "Execute Patrol Standing Behavior":
   ```
   Event Execute Patrol Standing Behavior
   → AI Move To (Location or Actor)
   ```

2. Override "Execute Patrol Crawling Behavior":
   ```
   Event Execute Patrol Crawling Behavior
   → AI Move To (Location or Actor)
   → (Use slower/different movement pattern)
   ```

3. Override "On Enter State":
   ```
   Event On Enter State
   → Branch (check NewState)
     → If PatrolStanding: Initialize standing patrol
     → If PatrolCrawling: Initialize crawling patrol
     → If Idle: Stop movement
   ```

### 3. Implementing Idle Animations

**Step 1: Configure Idle Behavior in BP_MyMonsterAI**
1. Open "BP_MyMonsterAI"
2. In Details panel, find "Monster AI | Idle" section:
   - `Min Idle Duration`: 5.0 (minimum seconds to stay idle)
   - `Max Idle Duration`: 15.0 (maximum seconds to stay idle)
   - `Min Subtle Movement Interval`: 2.0 (min seconds between twitches)
   - `Max Subtle Movement Interval`: 6.0 (max seconds between twitches)
   - `Breathing Cycle Duration`: 4.0 (one breath cycle in seconds)
   - `Patrol Transition Chance`: 0.3 (30% chance to patrol after idle)

**Step 2: Implement Animation Events in BP_MyMonster**

**Override "On Neck Twitch":**
```
Event On Neck Twitch
→ Play Animation Montage (select your neck twitch montage)
```

**Override "On Finger Shift":**
```
Event On Finger Shift
→ Play Animation Montage (select your finger shift montage)
```

**Override "On Breathing Update":**
```
Event On Breathing Update (BreathingIntensity input)
→ Get Mesh Component
→ Set Morph Target (e.g., "Chest_Expand", multiply BreathingIntensity by scale)
OR
→ Get Bone Transform (e.g., "Chest" bone)
→ Add Relative Scale (scale based on BreathingIntensity)
```

**Example Breathing Implementation:**
```
On Breathing Update
→ BreathingIntensity * 0.1 (scale down the intensity)
→ Make Vector (X=1.0 + result, Y=1.0 + result, Z=1.0 + result)
→ Set Relative Scale 3D (Chest bone, computed vector)
```

### 4. State Transitions

**From Blueprint (anywhere with reference to Monster or AI):**

```
Get AI Controller → Cast To MonsterAIController
→ Transition To State → Select State (Idle/PatrolStanding/PatrolCrawling)
```

**From C++:**
```cpp
AMonsterAIController* AIController = Cast<AMonsterAIController>(Monster->GetController());
if (AIController)
{
    AIController->TransitionToState(EMonsterBehaviorState::PatrolStanding);
}
```

### 5. Example: Simple Waypoint Patrol

**In BP_MyMonsterAI:**

**Variables:**
- `PatrolPoints` (Array of Vectors)
- `CurrentPatrolIndex` (Integer)

**Execute Patrol Standing Behavior:**
```
1. Check if reached destination
   → If Yes:
     → Increment CurrentPatrolIndex
     → Wrap around if exceeded array length
     → Get next patrol point
     → AI Move To Location
   → If No:
     → Continue moving
```

### 6. Example: Detection and State Change

**In BP_MyMonsterAI (or BP_MyMonster):**

**On Tick or Timer:**
```
1. Sphere Trace for actors (detect player)
2. If Player Detected:
   → Get AI Controller
   → Transition To State: PatrolStanding (to chase faster)
3. If Player Lost:
   → Transition To State: Idle or PatrolCrawling
```

### 7. Animation Integration

**In Monster Animation Blueprint:**

**Variables:**
- Get "Behavior State" from Monster Character
- Use it to drive Animation State Machine

**Animation States:**
- Idle State → Play idle animation
- Patrol Standing State → Play walk/run cycle
- Patrol Crawling State → Play crawl animation

**Example AnimGraph:**
```
State Machine:
  - Entry → Idle
  - Idle → PatrolStanding (when BehaviorState == PatrolStanding)
  - Idle → PatrolCrawling (when BehaviorState == PatrolCrawling)
  - PatrolStanding → Idle (when BehaviorState == Idle)
  - PatrolCrawling → Idle (when BehaviorState == Idle)
```

### 8. C++ Extension Example

```cpp
// Custom Monster Class
UCLASS()
class YOURGAME_API AAggressiveMonster : public AMonsterCharacter
{
    GENERATED_BODY()
    
public:
    virtual void OnBehaviorStateChanged_Implementation(
        EMonsterBehaviorState OldState, 
        EMonsterBehaviorState NewState) override
    {
        Super::OnBehaviorStateChanged_Implementation(OldState, NewState);
        
        // Play sound effects
        if (NewState == EMonsterBehaviorState::PatrolStanding)
        {
            PlaySound(StandingSound);
        }
        else if (NewState == EMonsterBehaviorState::PatrolCrawling)
        {
            PlaySound(CrawlingSound);
        }
    }
};

// Custom AI Controller
UCLASS()
class YOURGAME_API AAggressiveMonsterAI : public AMonsterAIController
{
    GENERATED_BODY()
    
protected:
    virtual void ExecutePatrolStandingBehavior_Implementation(float DeltaTime) override
    {
        // Custom patrol logic
        if (bDetectedPlayer)
        {
            // Chase player
            MoveToActor(PlayerCharacter);
        }
        else
        {
            // Resume patrol
            MoveToNextWaypoint();
        }
    }
};
```

### 8. Debug Tips

**Display Current State:**
```
In Event Tick (BP_MyMonster):
  → Get Behavior State
  → Print String (to see current state)
```

**Log State Transitions:**
```
In On Behavior State Changed (BP_MyMonster):
  → Print String: "Changed from [OldState] to [NewState]"
```

## Common Patterns

### Time-Based State Cycling
```
Timer → 
  If CurrentState == Idle: Transition to PatrolStanding
  Else If CurrentState == PatrolStanding: Transition to PatrolCrawling
  Else: Transition to Idle
```

### Distance-Based State Selection
```
Get Distance To Player →
  If Distance < 500: Transition to PatrolCrawling (stealth)
  Else If Distance < 1500: Transition to PatrolStanding (active)
  Else: Transition to Idle
```

### Health-Based State Changes
```
On Take Damage →
  If Health < 30%: Transition to PatrolCrawling (wounded)
  Else: Transition to PatrolStanding
```

## Troubleshooting

**Monster not moving:**
- Check if AI Controller is assigned
- Verify Navigation Mesh is present in level
- Check movement speed values are > 0
- Ensure state is not Idle

**States not changing:**
- Verify TransitionToState is being called
- Check AI Controller is properly possessed
- Add debug prints to track state changes

**Animations not working:**
- Verify Animation Blueprint is assigned
- Check state machine transitions
- Ensure behavior state is properly exposed to Animation BP
