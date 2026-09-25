# Thermal Balance Model

## Governing equation

For a single thermal zone:

\[
C \frac{dT_\text{in}}{dt} = Q_\text{internal} + Q_\text{solar} + UA (T_\text{out} - T_\text{in}) - Q_\text{HVAC}
\]

| Symbol | Meaning | Units |
|--------|---------|-------|
| \(C\) | Thermal capacitance | J/K |
| \(T_\text{in}\) | Indoor air temperature | °C |
| \(T_\text{out}\) | Outdoor air temperature | °C |
| \(UA\) | Envelope conductance | W/K |
| \(Q_\text{internal}\) | Internal gains | W |
| \(Q_\text{solar}\) | Solar gains through aperture | W |
| \(Q_\text{HVAC}\) | Net heat removed by HVAC (positive when heating) | W |

## Discretization

Forward Euler with fixed timestep \(\Delta t\):

\[
T_\text{in}^{n+1} = T_\text{in}^{n} + \frac{\Delta t}{C} Q_\text{net}^{n}
\]

Suitable for the time scales used (minutes) when \(C\) is large.

## HVAC

Cooling and heating setpoints define a deadband. Required load is computed with a proportional term plus a bias, then saturated at equipment capacity. Electrical power:

- Cooling: \(P_\text{elec} = Q_\text{cool} / \text{COP}\)
- Heating: \(P_\text{elec} = Q_\text{heat} / \eta\)

## Limitations

- Single zone, well-mixed air
- Constant COP / efficiency (no part-load or outdoor-temperature dependence in the present implementation)
- No latent loads or humidity control
- Solar model is a simple aperture × irradiance product
