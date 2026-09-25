# Electrical Network Model

## Representation

- Buses (nodes) with load and generation injection
- Branches (edges) with susceptance and thermal capacity
- Transformers attached to buses, carrying thermal state and health

## DC Power Flow

Under the standard DC approximations (flat voltage profile, small angle differences, neglect resistance and reactive power):

\[
\mathbf{P} = \mathbf{B} \boldsymbol{\theta}
\]

where \(\mathbf{B}\) is the bus susceptance matrix. The slack bus angle is fixed at zero; the reduced system is solved for the remaining angles. Branch flow:

\[
P_{ij} = B_{ij} (\theta_i - \theta_j)
\]

## Assumptions and Limitations

- No voltage magnitude solution
- No reactive power or voltage-dependent loads
- Losses neglected in the power balance
- Capacities checked after the solve; no security-constrained OPF
- Scaling between physical kW and per-unit susceptance is approximate and intended only for relative behavior on synthetic networks

This formulation is adequate for demonstrating load redistribution and overload detection after component outages. It is not a substitute for production-grade power-system analysis tools.
