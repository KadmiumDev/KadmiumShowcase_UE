## Articles & Community Discussions
* Read the full background story and technical write-up on DEV.to | https://dev.to/kadmium/aether-framework-c-architecture-showcase-2f20

> [!IMPORTANT]
> **Showcase & Intellectual Property Notice**
> 
> * **Not Open Source:** This repository is a **read-only architectural showcase** and proof-of-concept. It is not licensed for open-source use, distribution, or commercial integration.
> * **Purpose:** Created to verify the existence and design of a production-ready 6-DOF movement system built on Unreal Engine 5's experimental **Network Prediction Plugin (NPP)**.
> * **Code Availability:** All `.h` interfaces, state definitions, and pipeline setups are fully visible to demonstrate C++ code quality and memory layout. Simulation math inside `.cpp` files is stubbed. Full NDA-based source access is available for formal technical audits upon request (`legal@kadmium.dev`).


Implementation Verification
The public repository is intended as an architectural showcase and therefore does not expose the complete implementation.

For teams requiring deeper technical verification, the underlying C++ implementation can be made available for review under a standard NDA. This includes the simulation, networking, reconciliation, prediction and supporting framework code required to conduct a proper technical audit.

For NDA-based source access or a formal architectural/implementation audit, please contact: legal@kadmium.dev



# Aether Framework — C++ Architecture Showcase

A deterministic, 6-DOF vehicle movement architecture built for Unreal Engine 5 using the experimental **Network Prediction Plugin (NPP)**.

This repository serves as a **read-only architectural showcase** for technical leads and senior engineers evaluating C++ code quality, memory layout, network reconciliation, and large-world scale (LWC) stability.

---

## Technical Motivation

Standard Unreal Engine solutions like `UCharacterMovementComponent` (CMC) are designed around 2D/3D bipedal movement with implicit 90-degree gravity constraints. Adapting CMC to true 6-DOF spaceflight or custom pawn physics leads to severe maintenance overhead, network jitter under latency, and instability at extreme velocities in Large World Coordinates (LWC).

**Aether** provides a reusable, network-predicted 6-DOF core where the **prediction proxy owns network plumbing**, allowing the **simulation math to remain decoupled and deterministic**.

---

## Frame Flow & Pipeline

To prevent client resimulation rollbacks from snapping player vision, the visual mesh is detached from the physics root at `BeginPlay()`. World state queries are isolated to `ProduceInput()`, ensuring `FAetherSimulation` remains a pure function during client rollbacks.

```text
[ Player Inputs ]
       │
       ▼
[ UAetherAimDirectorComponent ]  ──> Smooths camera/aim vectors
       │
       ▼
[ UAetherMovementComponent ]     ──> ProduceInput() [Samples gravity & environment into FAetherInputCmd]
       │
       ▼
[ FAetherSimulation ]            ──> Pure function tick (Physics, Aerodynamics, Suspension sweeps)
       │
   ┌───┴────────────────────────┐
   ▼                            ▼
[ FinalizeFrame() ]      [ FinalizeSmoothingFrame() ]
(Root Physics Actor)     (Detached Visual Mesh & Camera)
```

---

## Core Architecture

### 1. State Isolation (Input, Sync, Aux)

State data is decoupled into three distinct memory structures to minimize wire payload and separate high-frequency movement ticks from static vessel attributes.

| Struct | Frequency | Replicated Payload / Key Fields | Architectural Purpose |
| :--- | :--- | :--- | :--- |
| **`FAetherInputCmd`** | High (Per Tick) | 6-DOF movement axes, AimDirection, GravityForce, EnvironmentDensity | Local sampled control & pre-evaluated environmental state. |
| **`FAetherSyncState`** | High (Per Tick) | Location (`FVector`), Rotation (`FQuat`), LinearVelocity, GForce, AOA | Authority transform & dynamic flight telemetry. |
| **`FAetherAuxState`** | Low (On Change) | Mass, MaxForwardThrust, PitchRate, YawRate, RollRate, Damping | Static vessel specs; updated via `WriteAuxState<FAetherAuxState>()`. |

```cpp
struct FAetherModelDef : FNetworkPredictionModelDef
{
    NP_MODEL_BODY();

    using StateTypes = TNetworkPredictionStateTypes<FAetherInputCmd, FAetherAuxState, FAetherSyncState>;
    using Simulation = class FAetherSimulation;
    using Driver     = class UAetherMovementComponent;

    static const TCHAR* GetName() { return TEXT("AetherMovement"); }
    static constexpr int32 GetSortPriority() { return (int32)ENetworkPredictionSortPriority::KinematicMovers; }
};
```

### 2. Deterministic Resimulation & World Querying
* **Pure Function Simulation:** Calling `GetWorld()` or performing overlap queries inside `SimulationTick()` breaks determinism during client rollbacks. `ProduceInput()` samples surrounding `UAetherGravityComponent` volumes and serializes `GravityForce` and `EnvironmentDensity` directly into `FAetherInputCmd`.
* **Aerodynamic Scaling:** `EnvironmentDensity` scales lift vectors, drag, and coupled/decoupled flight modes dynamically without executing scene queries during resimulation frames.

### 3. Precision Reconciliation Thresholds
To eliminate visual snapping at high speeds in UE5 LWC (double precision), `ShouldReconcile` relies on squared distances and quaternion angular deltas to avoid gimbal lock edge cases:

```cpp
bool ShouldReconcile(const FAetherSyncState& AuthorityState) const
{
    return FVector::DistSquared(Location, AuthorityState.Location) > 100.0f ||
           Rotation.AngularDistance(AuthorityState.Rotation) > 0.05f ||
           bLandingGearDeployed != AuthorityState.bLandingGearDeployed;
}
```

### 4. Suspension Model
* Raycast-based spring-damper model executed inside `CalculateLandingGearForces`.
* Converts spring compression, leverage arms, and velocity-aligned damping into linear force and angular acceleration applied to vessel mass.

---

## Source Code Access & Technical Audit (NDA)

To comply with licensing and commercial protection, this public mirror contains the complete header layout while stubbing proprietary tick math inside `AetherSimulation.cpp`.

* **Public Mirror Includes:** 100% complete `.h` interface definitions, `UPROPERTY` reflections, state structs, and network proxy bindings.
* **Full C++ Source Access:** For engineering teams requiring a complete technical audit or implementation review, full source access (including complete simulation, networking, and reconciliation implementations) is available under a standard NDA.

For source access requests or architectural audits, contact: **`legal@kadmium.dev`**

---

## Repository Structure

```text
│─ Aether_Showcase
│   └─ Source/
│       └── Aether/
│            ├── Public/        # Complete C++ headers and state definitions
│            └── Private/       # Implementation stubs & network proxy bindings
├── KNOWN_ISSUES.md             # Technical debt, trade-offs, and optimization queue
├── Aether.uplugin               # Plugin descriptor (NetworkPrediction dependency)
└── README.md
```

---

## Known Trade-Offs & Roadmap

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for current engineering debt, including:
* In-tick suspension raycast pre-caching during NPP client resimulations.
* Direct float comparison refactoring to `FMath::IsNearlyZero`.
* Migration of inline simulation constants to `UDataAsset` definitions.

---

## License & Contact

* **License Terms:** [Kadmium License Agreement](https://www.kadmium.dev/license)
* **Author:** Emil Fredrik Sjöstedt (Kadmium)
* **Contact & Inquiries:** `emil@kadmium.dev` | `legal@kadmium.dev`