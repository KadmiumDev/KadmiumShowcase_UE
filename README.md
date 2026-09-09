# Aether Framework — C++ Architecture Showcase

Read-only code showcase for a custom Unreal Engine 5 movement implementation built on the experimental **Network Prediction** plugin.

This public repository serves as a **read-only architectural audit** for technical leads and senior engineers evaluating codebase quality, memory layout, and C++ design patterns.

---

## Technical Overview

Aether is a custom C++ movement component (`UAetherMovementComponent`) that integrates with UE5's Network Prediction Proxy (NPP) architecture to achieve deterministic client prediction and server reconciliation for multi-axis vehicle flight.

### Scope of Showcase

To comply with repository distribution policies, this public mirror contains the full API layout while stubbing proprietary tick math:

In accordance with the **Kadmium Software & Source Code License Agreement**, the actual simulation implementation details are proprietary.

* **Header Files (`.h`):** 100% complete. Contains class definitions, `UPROPERTY` setups, interface definitions, and state structs (`FAetherInputCmd`, `FAetherSyncState`, `FAetherAuxState`).
* **Implementation Files (`.cpp`):** Class lifecycle, network proxy initialization, and function signatures are intact. Physics equations and vector transforms inside `AetherSimulation.cpp` are replaced with structural comments.

---

## Architecture & Data Layout

### 1. Network Prediction Model (`FAetherModelDef`)
* **State Isolation:** Separates high-frequency networking data (`FAetherSyncState`: Location, Rotation, LinearVelocity) from low-frequency property updates (`FAetherAuxState`: Mass, Thrust, Damping, Control Rates).
* **Command Buffer (`FAetherInputCmd`):** Serializes raw 6-DOF input vectors, aim vectors, control states, and frame-local gravity forces.
* **Reconciliation:** Custom `ShouldReconcile` thresholds for distance and angular delta tolerances to minimize correction frequency.

### 2. Gravity Resolution (`UAetherGravityComponent`)
* Supports spherical (radial distance falloff) and directional (world-space/local vector) gravity types.
* Evaluates overlapping volumes via an explicit integer priority hierarchy (`Priority` field).

### 3. Suspension Simulation
* Raycast-based spring-damper model executed during the simulation tick (`CalculateLandingGearForces`).
* Calculates compression, velocity-aligned damping, and leverage-arm torque to update linear and angular acceleration states.

### 4. Aerodynamics & Flight State
* Computes Angle of Attack (AOA) and G-Force telemetry per frame.
* Density-scaled lift vectors and speed-dependent control authority scaling (aerodynamic damping vs. spaceflight coupled/decoupled modes).

---

## Repository Structure

│─Aether_Showcase
│  └─Source/
│      └── Aether/
│           ├── Public/         # Complete C++ headers and state definitions
│           └── Private/        # Implementation stubs & network proxy bindings
├── KNOWN_ISSUES.md         # Technical debt, prototype trade-offs, and optimization queue
├── Aether.uplugin          # Plugin descriptor (NetworkPrediction dependency)
└── README.md


---

## Known Trade-Offs & Technical Debt

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for details on:
* In-tick raycasting overhead during NPP client resimulation rollbacks.
* Direct float comparisons queued for `FMath::IsNearlyZero` refactoring.
* Hardcoded simulation constants targeted for `UDataAsset` exposure.

---

## Demo & Visuals

* **Playable Test Build (.exe):** *COMING SOON*
* **Video Deep-Dive:** *COMING SOON*

---

## License & Contact

* **License Terms:** [kadmium.dev/license](https://www.kadmium.dev/license)
* **Author:** Emil Fredrik Sjöstedt (Kadmium)
* **Contact:** `legal@kadmium.dev`
