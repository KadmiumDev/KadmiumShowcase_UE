# Architecture Scope & Proprietary Boundaries

> **Note to Reviewers:** This repository is an **architectural showcase** demonstrating high-level C++ structure, API layout, state isolation, and Unreal Engine Network Prediction plugin (NPP) integration. To comply with licensing and commercial protection, proprietary tick physics and vector simulation math inside `.cpp` files are stubbed or simplified. The `.h` interfaces, network state quantization, and developer tools are fully featured and represent the core framework design.

## Technical Advisory & Code Audits

For engineering teams working on custom Unreal Engine 5 movement, Network Prediction Plugin (NPP) physics, or high-performance C++ architecture, direct advisory and repository audits are available:

* **1-on-1 Advisory Call (1h):** Architecture Q&A and sanity checks for specific UE5 networking or simulation blockers.
* **Async Code Audit:** Deep-dive analysis of repository bottlenecks, resimulation rollbacks, and memory layouts.
* **Engineering Retainers & Custom Integration:** Dedicated C++ support and Aether framework integration for active production builds.

Full service details, availability, and scheduling are available at **[kadmium.dev/services](https://www.kadmium.dev/services)** or via direct contact at **`legal@kadmium.dev`**.

---

## Included Architecture & Showcase Features

This public mirror contains several robust systems designed to demonstrate code quality, memory management, and deterministic networking. The following features are actively included and verifiable within the repository's `.h` and `.cpp` files:

### 1. Network Payload & Bandwidth Optimization
* **Input Cmd Serialization & Quantization:** The `FAetherInputCmd::NetSerialize` function effectively compresses control axes (`ForwardCmd`, `RightCmd`, `UpCmd`, `PitchCmd`, `YawCmd`, `RollCmd`) down to 8-bit integers (`int8`) to minimize payload size.
* **Bit-Packed Flags & Quantized Normal Aiming:** Booleans such as `bIsCoupledMode` and `bWantsLandingGear` are packed into a 2-bit field mask. Aim directions utilize `FVector_NetQuantizeNormal` for efficient replication.
* **Deterministic Environment Parameters:** `GravityForce` and `EnvironmentDensity` are excluded from network serialization and are instead evaluated deterministically per-tick by the subsystem on both client and server.

### 2. Gravity Subsystem & Deterministic Resolution
* **Subsystem-Driven Gravity (`UAetherGravitySubsystem`):** Replaces expensive component overlap polling with a dedicated `UWorldSubsystem`. It supports both Spherical planets and Directional OBB (Oriented Bounding Box) volumes.
* **Per-Frame Transform Caching & Batch Registration:** Utilizes `GFrameCounter` to cap transform updates for moving zones to once per frame. Features `RegisterZonesBatch()` for mass-spawning large object groups efficiently.

### 3. Suspension & Landing Gear Refactoring
* **POD Struct Caching:** Replaces runtime component array iterations during simulation ticks with flat `TArray<FAetherGearData>` POD structs, initialized during `BeginPlay` for optimal performance.

### 4. Input, Aiming & Precision Fixes
* **Dynamic Reconciliation Tolerances:** `FAetherSyncState::ShouldReconcile` dynamically queries `UAetherDeveloperSettings` (e.g., `ReconcileMaxPosError`, `ReconcileMaxRotError`) and logs telemetry data via `RecordTelemetryEvent()`.
* **Visual Smoothing Fix:** Employs a velocity-compensated dynamic error offset (`SmoothingTranslationOffset`) within `FinalizeSmoothingFrame()` to eliminate visual snapping. The offset reset is directly managed inside `TeleportTo()`.

### 5. Developer Tooling & Simulation Diagnostics
* **Dedicated Engine Settings (`UAetherDeveloperSettings`):** Provides an integrated developer settings panel (`Project Settings -> Kadmium Framework -> AETHER`) allowing for real-time diagnostic control and network emulation scenarios via the `aether.net.scenario` console variable.
* **Visual 3D Sync Overlay (`SAetherDebugOverlay`):** A custom Slate-based editor overlay that displays real-time telemetry, including simulation time in µs, G-Force, AOA, network lag, bandwidth, desync distance, and total rollbacks.

---

## Release Roadmap & Milestones

### v1.0 Early Access Live (Core Completion & Final Refinements)

* **Dynamic Relative Docking & Landing (`In Progress`):** Player-to-Player and Player-to-Environment relative docking. Enables smooth landings and zero-gravity attachment to moving stations, hangars, and capital ships.
* **Atmospheric Breaches & Vacuum Mechanics (`In Progress`):** Simulated environment parameters handling hull breaches, pressure drops, and localized atmospheric transitions between space vacuum and planetary gravity.
* **Direct Parameter & Variable Injection (`Completed`):** Supports runtime Parameter Overrides via Flux Framework C++ True Reflection or direct component setters. Standalone native wrappers are being polished to allow seamless variable injection without requiring the Flux Stat system.
* **Core Deterministic NPP Flight Engine (`Completed`):** 6-DOF Newtonian flight model, planetary gravity fields, raycast suspension, and prediction reconciliation built natively on the Network Prediction Plugin.

### Funding Dependent Expansions (Future Pipeline)

* **6-DOF Character Movement Model (`Planned`):** Predicted character movement model built for high-mobility gameplay, featuring wall-running, EVA thruster controls, and dynamic orientation.
* **Dedicated Ground Vehicle Physics (`Planned`):** Extending the NPP engine to handle wheeled and tracked vehicle simulation, surface traction, and suspension under custom planetary gravity fields.
* **Advanced Weather & Environmental Simulation (`Planned`):** Volumetric atmospheric wind, dynamic turbulence systems, and environmental storms affecting flight stability and aerodynamic drag.
* **Multi-Crew Systems & VR Motion Comfort (`Planned`):** Multi-crew station delegation and VR-tailored camera stabilization with dynamic horizon-locking to eliminate motion sickness during high-G maneuvers.
* **Advanced Aerodynamic Stress & Structural Failure (`Planned`):**
  * *Current Capability:* Aether calculates real-time `CurrentGForce` and Angle of Attack (AoA) directly in the core simulation tick, which can already be fed into the Kinetix Framework to trigger procedural mesh destruction upon exceeding stress thresholds.
  * *Planned Dedicated Expansion:* Dedicated per-wing / per-surface aerodynamic load mapping. Calculates localized pressure distribution across individual airframe components (e.g., extreme high-speed pitch pulling wing-root snap) to trigger procedural structural tearing and partial airframe disintegration.

---

## Crucial Network Prediction Setup (DefaultNetworkPrediction.ini)

**IMPORTANT:** To prevent client-server state desync and resimulation fights, your UE project must configure `Config/DefaultNetworkPrediction.ini` with the following parameters:

```ini
[/Script/NetworkPrediction.NetworkPredictionSettings]
PreferredTickingPolicy=Independent
ReplicatedManagerClassOverride=/Script/NetworkPrediction.NetworkPredictionReplicatedManager
FixedTickFrameRate=60
bForceEngineFixTickForcePhysics=True
SimulatedProxyNetworkLOD=Interpolated
bEnableFixedTickSmoothing=True
FixedTickInterpolationBufferedMS=100
IndependentTickInterpolationBufferedMS=100
IndependentTickInterpolationMaxBufferedMS=250
FixedTickInputSendCount=6
IndependentTickInputSendCount=6
MaximumRemoteInputFaultLimit=6
```

**Why these specific flags matter:**

* **`PreferredTickingPolicy=Independent` (Critical):** Decouples the simulation tick from both the render thread and standard engine tick groups. This prevents client and server timelines from fighting each other during framerate fluctuations.
* **`SimulatedProxyNetworkLOD=Interpolated` & `bEnableFixedTickSmoothing=True` (Critical):** Forces remote entities and simulated proxies (e.g., other players' ships) to smoothly interpolate between network state updates instead of snapping visually.
* **`IndependentTickInputSendCount=6`:** Sends 6 redundant historical input frames with every UDP packet. This is the core reason Aether absorbs up to 70% packet loss without suffering input starvation.