## Current Prototype State & Known Technical Debt

> **Note to Reviewers:** This repository represents an active prototype showcasing architecture, state isolation, and Network Prediction plugin integration. The items below are known prototype shortcuts currently queued for production hardening.

### 1. Math & Floating-Point Precision
* **Direct Float Comparisons:** Certain checks (e.g., `VerticalDot < 0.98f` or exact `0.0f` checks) are currently used in `AetherAimDirectorComponent` and simulation ticks. Production builds will replace these with UE tolerance helpers (`FMath::IsNearlyZero`, `FMath::IsNearlyEqual`) to ensure cross-platform precision consistency.
* **Hardcoded Magic Numbers:** Operational constants (such as terminal velocity multipliers, lift curves, and dot-product thresholds) are hardcoded inline in `AetherSimulation.cpp`. These will be moved into `FAetherAuxState` and `UDataAsset` configs so designers can tweak flight characteristics without re-compiling.

### 2. Network Prediction & Resimulation Overhead
* **In-Tick Physics Traces:** `SweepSimulation` and `CalculateLandingGearForces` currently execute `ComponentSweepMulti` and `LineTraceSingleByChannel` directly within `FAetherSimulation::SimulationTick`. 
  * *Planned Optimization:* Because NPP rolls back and re-simulates multiple frames during client reconciliation/packet loss, doing raw scene queries inside the simulation loop multiplies raycast costs. Production updates will pre-cache trace hit data prior to the rollback buffer evaluation or limit sweep ranges.

### 3. Input Clamping & DPI Handling
* **Mouse Delta Rejection:** `AetherAimDirectorComponent::AddAimInput` currently discards mouse deltas greater than `50.f` to prevent rotational snaps during long frames. 
  * *Planned Fix:* Replace input discarding with delta clamping and frame-rate independent scaling to properly support high-DPI mice and sudden frame hitches.

### 4. Component Lifecycle & Memory Safety
* **External Reference Tracking:** `UAetherMovementComponent` currently tracks active gravity sources via `TArray<UAetherGravityComponent*>`. 
  * *Planned Fix:* Transition external actor components to `TWeakObjectPtr<UAetherGravityComponent>` to explicitly guard against sudden actor destruction in dynamic levels (e.g., moving gravity volumes or destroyed planetoids).