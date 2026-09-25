# Model Risks and Limitations

## Parameter uncertainty

Failure rates, aging coefficients, thermal time constants, and COP values are not calibrated to field data. Outcomes are sensitive to these choices.

## Structural model uncertainty

- DC power flow omits voltage and reactive effects that can matter under stress.
- Single-zone thermal model cannot capture multi-zone or stratified behavior.
- Cascade logic redistributes load immediately; real protection and operator actions have delays and discrete rules.

## Synthetic scenarios

Weather is generated from simple periodic functions plus noise. It is not a downscaled climate projection and carries no claim of geographic realism.

## Numerical error

Fixed-step Euler integration and dense linear solves on small systems are used. For the intended timesteps and network sizes, truncation error is expected to be secondary to model-form error, but this has not been quantified exhaustively.

## Extrapolation risk

Results obtained on the included synthetic city-scale network must not be transferred to real systems without independent validation, site-specific data, and appropriate professional engineering review.

## Intended use

Research, education, software-engineering demonstration, and methodological experimentation. Not for safety-critical design, regulatory filings, or operational control.
