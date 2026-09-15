## Articles & Community Discussions
* Read the full background story and technical write-up on DEV.to | https://dev.to/kadmium/aether-framework-c-architecture-showcase-2f20
* A showcase video from the livebuild will release on wednesday 16 September - 14:00 CEST+1,Sweden.
* A playeable demo will release short after as soon as some time frees up for me.

> [!IMPORTANT]
> **Showcase & Intellectual Property Notice**
> 
> * **Not Open Source:** This repository is a **read-only architectural showcase** and proof-of-concept. It is not licensed for open-source use, distribution, or commercial integration.
> * **Purpose:** Created to verify the existence and design of a production-ready 6-DOF movement system built on Unreal Engine 5's experimental **Network Prediction Plugin (NPP)**.
> * **Code Availability:** All `.h` interfaces, state definitions, and pipeline setups are fully visible to demonstrate C++ code quality and memory layout. Simulation math inside `.cpp` files is stubbed. Full NDA-based source access is available for formal technical audits upon request (`legal@kadmium.dev`).

* Made Unreal Engine 5.7.4

# Implementation Verification
The public repository is intended as an architectural showcase and therefore does not expose the complete implementation.

For teams requiring deeper technical verification, the underlying C++ implementation can be made available for review under a standard NDA. This includes the simulation, networking, reconciliation, prediction and supporting framework code required to conduct a proper technical audit.

For NDA-based source access or a formal architectural/implementation audit, please contact: legal@kadmium.dev


## Network Emulation & Profiling Results

All stress tests were conducted with a **60 Hz dedicated server tick rate** |  **Ship max speed was 55,000cm/s** using Unreal Engine's built-in packet simulation emulation and standard Network Prediction Plugin (NPP) reconciliation defaults.

| Scenario | 1-Way Latency / RTT | Packet Loss / Jitter / Reorder | Client Visual Experience | Network Prediction & Buffer Behavior |
| :--- | :--- | :--- | :--- | :--- |
| **Ideal** | 0 ms / 0 ms | 0% / 0 ms / 0% | Baseline Flawless | 0 Restore Events / 0 Hard Rollbacks |
| **GoodBroadband** | 20 ms / 40 ms | 0% / 2 ms / 0% | Imperceptible from Ideal | Zero network desync |
| **BadWiFi** | 40 ms / 80 ms | 8% / 20 ms / 3% | Imperceptible from Ideal | Zero network desync |
| **HighPing** | 180 ms / 360 ms | 0% / 10 ms / 0% | Imperceptible from Ideal | Local prediction handles input latency cleanly |
| **SevereJitter** | 80 ms / 160 ms | 2% / 60 ms / 10% | Imperceptible from Ideal | Jitter absorbed by prediction buffer |
| **PacketLossSpike** | 50 ms / 100 ms | 15% / 10 ms / 0% | Imperceptible from Ideal | Zero visual stuttering |
| **AetherStable** | 300 ms / 600 ms | 15% / 80 ms / 1% | Imperceptible from Ideal | Full 600 ms RTT absorbed with 0 hard rollbacks |
| **AetherUnstable** | 600 ms / 1200 ms | 25% / 100 ms / 3% | Visually Smooth | Minor `RESTORE EVENTS` triggered; no client snaps |
| **AetherExtreme** | 1200 ms / 2400 ms | 35% / 200 ms / 5% | Visually Smooth | Velocity-compensated `SmoothingTranslationOffset` dikes out historical adjustments |
| **JustNo** | 3000 ms / 6000 ms | 70% / 300 ms / 10% | Playable (micro-stutters); increasing reconciliation thresholds trades sim accuracy for visual smoothness | Heavy Input Starvation & frequent resimulations; input lag noticeable |

---

[!NOTE]
> **Developer's Note & Personal Reflection**
>
> Maybe a bit informal for a technical README, but I'm honestly way too hyped for tomorrow's video showcase! Pushing AETHER to its absolute limits in the 
> current live build has been incredible. Seeing `FAetherInputCmd`, the detached visual mesh smoothing, and UE5's Network Prediction Plugin work in harmony 
> inside a pure,  stateless simulation tick has yielded performance and deterministic accuracy beyond what I originally thought possible.
>
> Being able to throw **3,000 ms 1-way latency (6,000 ms RTT)** at a 6-DOF simulation while keeping the ship flyable and visually stable still leaves me in awe. 
> Future optimization passes might even let us test absurd thresholds like 6,000–8,000 ms 1-way—which is completely ridiculous for real-world networking, 
> but a fantastic testament to how far this architecture can be stretched!

---

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

## Known Trade-Offs & Architecture Roadmap

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for complete details on engineering debt, internal live-build optimizations, and the expansion roadmap. Key items include:

* **Showcase vs. Live Build Differences:** Network payload quantization (int8/bit-packing), subsystem-driven gravity (`UAetherGravitySubsystem`), three-stage early-exit suspension raycasting, and Slate-based debug tools (`SAetherDebugOverlay`).
* **Immediate Milestones:** Public showcase video (Wednesday 16 September - 14:00 CEST+1) and a standalone playable demo.
* **Extended Roadmap (Funding Dependent):** Advanced atmospheric turbulence, dedicated ground/wheeled vehicle simulation, predicted 6-DOF character movement, and VR motion comfort systems.

---

## License & Contact

* **License Terms:** https://www.kadmium.dev/legal/software-source-code-license-agreement | https://www.kadmium.dev/legal/eula | https://www.kadmium.dev/legal/privacy-policy | https://www.kadmium.dev/legal/terms-of-service
* **Project Page & Funding:** https://www.kadmium.dev/dev-tech/ue-frameworks/aether | https://kadmium.gumroad.com/
* **AI Friendly Documentation:** https://www.kadmium.dev/ai-summary
* **Author:** Emil Fredrik Sjöstedt (Kadmium)
* **Contact & Inquiries:** `emil@kadmium.dev` | `legal@kadmium.dev` | https://www.kadmium.dev/contact-links