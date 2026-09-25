#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${ROOT}/build/apps/resilience-sim"
if [[ ! -x "$BIN" ]]; then
  echo "Build the project first: cmake -S . -B build && cmake --build build"
  exit 1
fi
"$BIN" run --scenario baseline --days 3 --output "${ROOT}/results/baseline.json"
echo "Baseline results written to results/baseline.json"
