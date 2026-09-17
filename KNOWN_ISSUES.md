# Known Technical Debt & Architecture Roadmap

> **Note to Reviewers:** This public showcase repository demonstrates the high-level C++ architecture, API layout, state isolation, and Unreal Engine Network Prediction plugin integration. Proprietary tick physics, vector simulation math, and internal live-build optimizations are omitted or stubbed in the public showcase files in accordance with licensing agreements.

---

## Showcase vs. Live Build Differences & Active Refactoring

The list below outlines key architectural differences between this public showcase mirror and the internal live build (Aether Live), alongside optimizations currently under active development in the internal codebase:

### 1. Network Payload & Bandwidth Optimization (Live Build)
* **Input Cmd Serialization & Quantization:** While the showcase uses uncompressed floats in `FAetherInputCmd`, the live build quantizes control axes (`ForwardCmd`, `RightCmd`, `UpCmd`, `PitchCmd`, `YawCmd`, `RollCmd`) to 8-bit integers (`int8`) in `NetSerialize`.
* **Bit-Packed Flags & Quantized Normal Aiming:** Booleans (`bIsCoupledMode`, `bWantsLandingGear`) are bit-packed into a 2-bit field mask, and the aim direction uses `FVector_NetQuantizeNormal` instead of raw vectors.
* **Deterministic Environment Parameters:** `GravityForce` and `EnvironmentDensity` are removed from network serialization and evaluated deterministically per-tick via the gravity subsystem across both client and server.

### 2. Gravity Subsystem & Deterministic Resolution (Live Build)
* **Subsystem-Driven Gravity (`UAetherGravitySubsystem`):** Showcase uses component overlap tracking. The live build utilizes a dedicated `UWorldSubsystem` that eliminates component overlap polling and supports both Spherical planets and Directional OBB (Oriented Bounding Box) volumes.
* **Direct SyncState Lookup:** Pulls world transforms directly from `UAetherMovementComponent`'s `FAetherSyncState` to maintain strict determinism during network prediction rollbacks.
* **Per-Frame Transform Caching & Batch Registration:** Implemented `GFrameCounter` checks to cap transform updates for moving zones to once per frame and added `RegisterZonesBatch()` for mass-spawning large object groups (e.g., asteroid fields) in a single pass.

### 3. Suspension & Landing Gear Refactoring (Live Build)
* **POD Struct Caching:** Replaced runtime component array iterations in simulation ticks with flat `TArray<FAetherGearData>` POD structs initialized at `BeginPlay`.
* **Three-Stage Early-Exit Traces:** 
  1. *Altitude Exit:* Bypasses raycasts when gear is retracted or altitude is out of range.
  2. *Hysteresis Center Probe:* Runs a single sphere trace along the ship's local Z-axis (`bCenterProbeActive`: activation <15m, deactivation >18m) before evaluating individual suspension components.
  3. *POD Execution:* Executes short individual line traces per gear only when the center probe registers ground contact.
* **Single-Pass Mass Scaling:** Removed duplicate pre-loop passes for gear counting; mass distribution scales directly via cached array size.

### 4. Input, Aiming & Precision Fixes (Live Build)
* **Sensitivity & Clamped Aim Delta:** `UAetherAimDirectorComponent::AddAimInput` includes smooth sensitivity scaling and soft clamping to prevent rotational snaps during hitches without discarding user input.
* **Dynamic Reconciliation Tolerances:** `FAetherSyncState::ShouldReconcile` dynamically queries `UAetherDeveloperSettings` (`ReconcileMaxPosError`, `ReconcileMaxRotError`) and logs telemetry via `RecordTelemetryEvent()`.
* **Rotation Matrix Stale State Fix:** Recalculates local space vectors in `SimulationTick` immediately after landing gear torque and collision deflection to ensure mouse aim and horizon leveling use updated transforms.
* **Visual Smoothing Fix:** Uses velocity-compensated dynamic error offset (`SmoothingTranslationOffset`) in `FinalizeSmoothingFrame()` to eliminate visual snapping; offset reset is driven directly inside `TeleportTo()`.

### 5. Developer Tooling & Simulation Diagnostics (Live Build)
* **Dedicated Engine Settings (`UAetherDeveloperSettings`):** Integrated a developer settings panel into the engine (`Project Settings -> Kadmium Framework -> AETHER`) for real-time diagnostic control and 10 network emulation scenarios via `aether.net.scenario`.
* **Visual 3D Sync Overlay (`SAetherDebugOverlay`):** Slate-based editor overlay providing real-time telemetry (Sim time in µs, G-Force, AOA, Net Lag, Bandwidth, Desync cm, Total Rollbacks) and 3D sync trails comparing local client trajectories against server prediction vectors.

---

## Future Roadmap

### Immediate Milestones
* **Public Showcase Video:** https://youtu.be/szciROm9ppU
* **Playable Test Demo:** A standalone compiled test environment executable for reviewers to evaluate flight feel and network handling directly.
* **Aether availablity:** The aether source code is available now on gumroad and stripe - https://kadmium.gumroad.com/l/AetherFramework/aetherlaunch26 or for studios/Enterprise that need extended licensing or custom architecture vistit https://www.kadmium.dev/services

### Extended Expansion (Funding Dependent - see [kadmium.dev/aether](https://www.kadmium.dev/dev-tech/ue-frameworks/aether) or [Gumroad](https://kadmium.gumroad.com/l/AetherFramework/aetherlaunch26)
Subject to securing additional development funding, planned architecture expansions include:

* **Advanced Atmospheric & Turbulence Simulation:** Expanding `UAetherGravityComponent` and environmental handlers to support volumetric wind, dynamic turbulence, and weather-driven flight interference.
* **Dedicated Ground Vehicle Simulation:** Extending Aether's NPP engine to handle wheeled and tracked vehicle physics, suspension dynamics, and surface traction under custom gravity fields.
* **Advanced 6-DOF Character Movement:** A custom predicted character movement model built for high-mobility gameplay (featuring wall-running, thruster mechanics, and dynamic orientation).
* **Dedicated VR Motion Comfort System:** VR-tailored camera stabilization, dynamic horizon-locking, and visual comfort anchors to eliminate motion sickness during aggressive high-G maneuvers in HMD cockpits.
* **Console DevKit Hardening:** Direct testing and optimization across Console DevKits, including platform-specific hardware profiling, memory tuning, and native gamepad integration beyond basic wrappers.



## Crucial Network Prediction Setup (DefaultNetworkPrediction.ini)

IMPORTANT: I'm so sorry! I got too stuck in the live build! 
To prevent client-server state desync and resimulation fights, your UE project must configure Config/DefaultNetworkPrediction.ini with the following parameters:

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

Why these specific flags matter:

PreferredTickingPolicy=Independent (Critical): Decouples the simulation tick from both the render thread and standard engine tick groups. This prevents client and server timelines from fighting each other during framerate fluctuations.

SimulatedProxyNetworkLOD=Interpolated & bEnableFixedTickSmoothing=True (Critical): Forces remote entities and simulated proxies (e.g., other players' ships) to smoothly interpolate between network state updates instead of snapping visually.

IndependentTickInputSendCount=6: Sends 6 redundant historical input frames with every UDP packet. This is the core reason Aether absorbs up to 70% packet loss without suffering input starvation.

