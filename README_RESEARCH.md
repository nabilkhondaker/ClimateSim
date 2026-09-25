# Research Notes — Climate Infrastructure Resilience Simulator

## Motivation

Infrastructure systems exhibit interdependencies that become visible under concurrent environmental and operational stress. Heat-driven cooling load, transformer thermal aging, and stochastic component failure interact nonlinearly. This codebase provides a controllable synthetic environment in which those interactions can be examined, interventions compared, and numerical methods tested.

## Core Engineering Questions

1. How does prolonged elevated ambient temperature affect transformer hotspot temperature and cumulative aging on a synthetic distribution-like network?
2. Under what combinations of aging, load growth, and heat anomaly do cascading failures become probable?
3. How do simple interventions (capacity upgrade, demand reduction via building efficiency) alter the resilience curve (minimum performance, recovery time, AUC loss)?
4. Which model parameters dominate outcome variance under Monte Carlo sampling of weather noise and failure draws?

## Mathematical Models

### Building thermal balance

\[
C \frac{dT_\text{in}}{dt} = Q_\text{internal} + Q_\text{solar} + UA (T_\text{out} - T_\text{in}) - Q_\text{HVAC}
\]

Integrated with forward Euler. HVAC power derived from required cooling/heating load and a constant COP (cooling) or efficiency (heating).

### DC power flow

\[
\mathbf{P} = \mathbf{B} \boldsymbol{\theta}
\]

Slack bus angle fixed at zero. Branch flow \(P_{ij} = B_{ij}(\theta_i - \theta_j)\). Capacities enforced only as post-solve overload checks; no optimal power flow.

### Transformer thermal / aging

Ultimate oil and hotspot rises scale with load ratio raised to a power (~1.6). First-order lag toward those ultimates. Aging acceleration:

\[
F_{aa} = \exp\left(\frac{15000}{383} - \frac{15000}{273+\theta_{hs}}\right)
\]

Health index reduced by accumulated life consumption. Failure probability in an interval uses a stress- and health-dependent hazard.

### Reliability primitives

- Exponential: \(R(t)=\exp(-\lambda t)\)
- Weibull: \(R(t)=\exp(-(t/\eta)^\beta)\)
- Interval failure probability under constant hazard: \(1-\exp(-\lambda\Delta t)\)

## Assumptions (selected)

- Synthetic weather; no observational assimilation
- Single-phase DC approximation; neglect voltage magnitude and reactive power
- Single thermal zone per building
- Independent failure draws conditional on current state (no common-mode beyond shared weather)
- Instantaneous load redistribution after failure (no protection relay timing)
- Illustrative numerical parameters, not field-calibrated

## Uncertainty & Sensitivity

The current release supports seeded stochastic weather and failure evaluation. Full Monte Carlo aggregation, variance-based sensitivity, and parallel scenario execution are architectural targets documented for future work.

## Validation Status

- Unit tests for conversions, reliability closed forms, thermal monotonicity, small power-flow solves, resilience metrics
- Integration test of a short end-to-end run
- No comparison against measured utility data (none is included)

## Limitations

See `docs/limitations/model_risks.md`. Results are conditional on the synthetic network, chosen parameters, and model structure. They must not be extrapolated to real assets without substantial additional validation and site-specific data.

## Future Research

Higher-fidelity power flow, multi-zone buildings, real weather coupling, explicit optimization over intervention portfolios, and rigorous uncertainty quantification.
