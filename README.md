# Climate Infrastructure Resilience Simulator

A C++20 simulation framework for exploring how interconnected infrastructure responds to environmental stress. The project models synthetic electrical, thermal, and water-related dependencies under configurable weather scenarios, tracks component degradation and cascading failures, and supports scenario comparison and basic intervention analysis. This was, by far, a more exploratory project compared to my other work, but I figured I could put my skills to good use. Below I explain my motivation & further details that you may be wondering.

📢 **Release Notice:** This repository contains the complete codebase for this project, engineered between *August 18, 2025* and *September 25, 2026*. The work was developed/engineered intermittently alongside other research software and has been packaged in full for public viewing and use.

**Author:** Nabil Khondaker  
**License:** MIT

## Overview

Modern infrastructure systems are tightly coupled. Extreme heat can increase cooling demand, which raises electrical loading, which stresses transformers, which accelerates aging and raises failure probability. Failures then redistribute load and can cascade. This repository provides a modular computational environment to study those interactions on synthetic networks.

The simulator is intended for research, education, methodology development, and portfolio demonstration of large-scale C++ engineering software. It is **not** a climate forecast model, **not** a substitute for certified engineering analysis, and **not** a tool for operational decision-making on real infrastructure.

## Motivation

Environmental stress propagates through physical and operational dependencies:

```
Extreme heat
  → higher building cooling demand
  → higher electrical demand
  → higher transformer loading and temperature
  → accelerated degradation
  → elevated failure probability
  → load redistribution
  → secondary stress / possible cascade
  → reduced service performance
```

Engineers can evaluate interventions (transformer upgrades, storage, efficiency improvements, maintenance) by re-running scenarios and comparing resilience metrics.

## What This Project Does

- Synthetic weather generation (seasonal + diurnal + anomaly + noise)
- Dynamic single-zone building thermal model (RC formulation)
- Graph-based electrical network with simplified DC power flow
- Transformer thermal state and aging acceleration
- Component health tracking and stochastic failure evaluation
- Cascading failure event logging
- Resilience curve metrics (minimum performance, AUC loss, recovery time)
- Scenario comparison via CLI
- JSON result export
- Unit, integration, and numerical tests
- CMake-based build with FetchContent dependencies

## What This Project Does Not Do

- Predict real-world climate or weather for any specific location
- Provide engineering certification or regulatory compliance analysis
- Model full AC power flow, detailed hydraulics, or traffic assignment
- Use real utility network data (all networks are synthetic)
- Require Python for core simulation or analysis

## System Architecture

```
Scenario Configuration
        ↓
Environmental Engine (synthetic weather)
        ↓
Infrastructure Model (electrical + buildings + transformers)
        ↓
Thermal / Electrical coupling
        ↓
Component degradation & reliability
        ↓
Failure evaluation & cascade engine
        ↓
Resilience metrics & export
```

Major modules live under `include/resilience/` and `src/` with corresponding tests under `tests/`.

## Engineering Models (Summary)

**Thermal (building):**  
`C dT/dt = Q_internal + Q_solar + UA (T_out − T_in) − Q_HVAC`  
Forward Euler integration. HVAC power derived from cooling/heating load and COP/efficiency.

**Electrical:**  
Simplified DC power flow `P = B θ`. Susceptance-based branch flows. Explicit assumptions and limitations documented in `docs/electrical/electrical_model.md`.

**Transformer:**  
First-order thermal lag toward load-dependent ultimate oil/hot-spot rise. Arrhenius-style aging acceleration. Health index reduced by accumulated aging. Stochastic failure probability depends on health and overload.

**Reliability:**  
Exponential and Weibull closed forms; stress-adjusted hazard for interval failure probability.

**Resilience metrics:**  
Time series of system performance → minimum, time-to-minimum, recovery time, area-under-curve loss.

All models are deliberately simplified. See `docs/engineering/` and `docs/limitations/`.

## Installation & Build

### Requirements

- CMake ≥ 3.20
- C++20 compiler (GCC 11+, Clang 14+, MSVC 19.3+)
- Git (for FetchContent)
- Internet access on first configure (downloads Eigen, nlohmann/json, yaml-cpp, spdlog, fmt, CLI11, Catch2)

### Build

```bash
git clone <repository-url>
cd climate-infrastructure-resilience-simulator

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Sanitizer build (optional):

```bash
cmake -S . -B build-san -DRESILIENCE_ENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-san --parallel
```

## Quick Start

```bash
# Run a 3-day baseline simulation
./build/apps/resilience-sim run --scenario baseline --days 3 --output results/baseline.json

# Heat-wave scenario
./build/apps/resilience-sim run --scenario heat_wave --days 5 --output results/heat_wave.json

# Extreme heat
./build/apps/resilience-sim run --scenario extreme_heat --days 3 --seed 99 -o results/extreme.json

# Compare scenarios
./build/apps/resilience-sim compare --output results/comparison.json

# Version
./build/apps/resilience-sim version
```

## CLI Commands

| Command | Description |
|---------|-------------|
| `run` | Execute a named scenario |
| `compare` | Run baseline / heat_wave / extreme_heat and write metrics |
| `validate-config` | Parse a YAML or JSON config |
| `version` | Print version and author |

## Configuration

Scenarios and parameters are controlled via CLI flags and (optionally) YAML/JSON files under `configs/` and `scenarios/`. Core simulation settings (start, end, timestep, seed) are not hard-coded throughout the library; they are passed through `SimulationConfig`.

## Project Structure

- `include/resilience/` — public headers (units, time, environment, thermal, electrical, cascades, simulation, metrics, …)
- `src/` — implementation files
- `apps/` — CLI application
- `tests/` — unit, integration, numerical tests (Catch2)
- `benchmarks/` — micro-benchmarks
- `configs/`, `scenarios/`, `experiments/` — configuration and experiment definitions
- `docs/` — architecture, engineering equations, limitations
- `examples/` — runnable example descriptions
- `cmake/` — project options, warnings, dependencies, sanitizers
- `.github/workflows/` — CI skeletons

## Testing

```bash
ctest --test-dir build --output-on-failure
```

Coverage includes units conversion, time/event queue, thermal monotonicity, reliability formulas, DC power flow on small networks, resilience metrics, and a short end-to-end simulator run.

## Reproducibility

Given identical configuration, seed, and software version, stochastic elements (weather noise, failure draws) are intended to be reproducible via seeded RNGs. Document any remaining nondeterminism (floating-point reduction order, parallel runs) when extending the code.

## Limitations

- Synthetic weather only; not a climate model
- Simplified DC power flow; no voltage magnitude solution, no reactive power
- Single-zone thermal model; no multi-zone or detailed HVAC dynamics
- Failure and aging parameters are illustrative, not calibrated to field data
- Water and transportation subsystems are present in the architecture but only lightly exercised in the current release
- No real geographic or utility data
- Not suitable for safety-critical or regulatory use

See `docs/limitations/model_risks.md` for a fuller discussion.

## Results

Results are generated by the experiment scripts and CLI; they are not hard-coded into this repository. Run the commands in the Quick Start section to produce JSON outputs under `results/`.

## Research Directions

- Higher-fidelity AC power flow or distribution-system solvers
- Multi-zone building models and measured load profiles
- Coupling to real weather reanalysis datasets
- Explicit hydraulic network solution
- Multi-objective intervention optimization with Pareto export
- Parallel Monte Carlo with independent RNG streams
- Uncertainty quantification and global sensitivity methods

## Citation

See `CITATION.cff`.

## Author

Nabil Khondaker

## License

MIT License. See `LICENSE`.
