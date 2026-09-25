# Transformer Thermal and Aging Model

## Thermal dynamics

Ultimate oil and hotspot temperature rises above ambient are approximated as power functions of load ratio. Actual temperatures follow a first-order lag with an oil time constant on the order of hours.

## Aging acceleration

An Arrhenius-style factor increases the rate of insulation life consumption at elevated hotspot temperature. The health index is reduced proportionally to cumulative life consumption.

## Failure probability

A simple hazard that increases with (1 − health) and with overload above a threshold yields an interval failure probability \(1 - \exp(-\lambda \Delta t)\). Draws are independent given the current state.

## Caveats

Parameters are illustrative. The model is adapted from textbook / standards concepts (e.g., IEEE C57.91 style thinking) but is not a certified implementation of any standard. Do not use results for asset management decisions on real transformers.
