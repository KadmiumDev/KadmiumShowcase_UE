# Known Technical Debt & Architecture Roadmap

> **Note to Reviewers:** This repository represents an architectural showcase of Aether's core layout, state isolation, and Unreal Engine Network Prediction plugin integration. Proprietary simulation math is omitted from public showcase files, but all prototype technical debt items have been resolved in the internal codebase.

---

## Resolved in Live Build

The following architectural optimizations and debt items from earlier prototype iterations are fully implemented in the live codebase:

### 1. Network Resimulation & Physics Trace Optimization
* **Pre-Cached Physics Traces:** Optimized `SweepSimulation` and `CalculateLandingGearForces` by pre-caching raycasts and collision sweep data prior to Network Prediction Proxy (NPP) rollback evaluation. This eliminates redundant trace overhead during client resimulation frames.

### 2. Configurable Flight Profiles & DataAsset Support
* **AuxState & DataAsset Integration:** Decoupled operational constants (such as lift curves, terminal velocity multipliers, and dot-product thresholds) into `FAetherAuxState` and `UDataAsset` configs. Flight characteristics can now be tuned by designers at runtime without C++ re-compilation.

### 3. Component Lifecycle & Memory Safety
* **Weak Pointer Tracking (`TWeakObjectPtr`):** Replaced raw C++ pointers (`TArray<UAetherGravityComponent*>`) with `TWeakObjectPtr` arrays for all external component references (`ActiveGravitySources` and `ActiveLandingGears`). This guards against dangling pointer crashes when gravity volumes or landing gear components are destroyed dynamically in the world.

### 4. Input Clamping & High-DPI Handling
* **Delta Clamping & Sensitivity Scaling:** Removed hard input rejection (which previously discarded mouse deltas `> 50.f`). Implemented smooth `FMath::Clamp` logic with customizable sensitivity scaling (`Sensitivity`) in `UAetherAimDirectorComponent::AddAimInput` to properly support high-DPI hardware and eliminate rotational snapping during frame drops.

### 5. Math & Floating-Point Precision
* **UE Tolerance Helpers:** Replaced direct float checks (such as raw `< 0.98f` comparisons) with standard Unreal Engine safety functions like `FMath::IsNearlyEqual` and `FMath::IsNearlyZero`. This guarantees consistency across different hardware architectures and platforms.

### 6. Developer Tooling & Simulation Diagnostics
* **Dedicated Engine Settings (`UAetherDeveloperSettings`):** Integrated a custom developer settings panel into the engine for real-time diagnostic control.
* **Visual 3D Sync Trails:** Added real-time 3D trail visualization comparing local client trajectories against server prediction vectors to instantly identify desync offsets.
* **Network Stress & Environmental Overrides:** Added editor tools to simulate packet loss (`SimulatedPacketLossPercent`), force client/server reconciliation scenarios, and override environmental parameters like atmospheric density on the fly.

---

## Future Roadmap

### Immediate Milestones
* **Public Showcase Video:** A video showcasing network reconciliation, atmospheric flight, and 6-DOF physics on wednesday 16 September - 14:00 CEST+1,Sweden. Thereafter a In-depth technical breakdown video.

* **Playable Test Demo:** A standalone compiled test environment executable for reviewers to evaluate flight feel and network handling directly, as soon as some time frees up for me.

### Extended Expansion (Funding Dependent - see https://www.kadmium.dev/dev-tech/ue-frameworks/aether or visit https://kadmium.gumroad.com/)
Subject to securing additional development funding, planned architecture expansions include:

* **Advanced Atmospheric & Turbulence Simulation:** Expanding `UAetherGravityComponent` and environmental handlers to support volumetric wind, dynamic turbulence, and weather-driven flight interference.
* **Dedicated Ground Vehicle Simulation:** Extending Aether's NPP engine to handle wheeled and tracked vehicle physics, suspension dynamics, and surface traction under custom gravity fields.
* **Advanced 6-DOF Character Movement:** A custom predicted character movement model built for high-mobility gameplay (featuring wall-running, thruster mechanics, and dynamic orientation).
* **Dedicated VR Motion Comfort System:** VR-tailored camera stabilization, dynamic horizon-locking, and visual comfort anchors to eliminate motion sickness during aggressive high-G maneuvers in HMD cockpits.
* **Console DevKit Hardening:** Direct testing and optimization across Console DevKits, including platform-specific hardware profiling, memory tuning, and native gamepad integration beyond basic wrappers.