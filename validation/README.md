# Validation Results

This directory contains reference results used to check the numerical behavior
and RFsimulator integration of the HAPS channel model.

The dataset is intended for regression testing and result comparison. It is not
a formal 3GPP or ITU-R conformance test suite.

## Files

| File | Purpose |
|---|---|
| [`reference_results.csv`](reference_results.csv) | Reference geometry, delay, propagation-loss, and link-status observations |

## Reference Setup

Unless a row states otherwise, the reference cases use:

| Parameter | Value |
|---|---:|
| HAPS altitude | 20 km |
| Carrier frequency | 3.6192 GHz |
| Sample rate | 61.44 Msps |
| Scenario | Rural |
| Terminal | Outdoor |

The baseline geometry cases cover elevation angles of 90°, 60°, 30°, and 10°.

## Validation Coverage

The `test_group` column identifies the purpose of each row:

| Test group | Validation target |
|---|---|
| `geometry_delay` | Slant range, one-way delay, integer-sample delay, quantization error, and FSPL |
| `los_probability` | OAI LOS probability against the 3GPP reference value |
| `probabilistic_los` | LOS/NLOS realization from the random draw and LOS probability |
| `shadow_fading_los` | LOS shadow-fading standard deviation |
| `shadow_fading_clutter_nlos` | NLOS shadow-fading standard deviation and clutter loss |
| `shadow_fading_seed` | Repeatability of seeded shadow-fading realizations |
| `bel_elevation` | Building entry loss across elevation angles |
| `bel_terminal_type` | Outdoor and indoor terminal-type comparison |
| `bel_percentile` | Building entry loss at selected percentiles |
| `nlos_scenario` | Rural, urban, and dense-urban NLOS comparison |
| `rain_attenuation` | Rain-path length and rain attenuation |
| `gas_attenuation` | Atmospheric gas attenuation |

## Delay Validation

The exact one-way propagation delay and sample count are calculated as:

```math
\tau = \frac{d_{\mathrm{slant}}}{c}
```

```math
N_{\mathrm{exact}} = \tau F_s
```

The current implementation uses ceiling quantization for the RFsimulator
integer-sample delay:

```math
N_{\mathrm{integer}} = \left\lceil N_{\mathrm{exact}} \right\rceil
```

```math
\tau_{\mathrm{applied}} = \frac{N_{\mathrm{integer}}}{F_s}
```

```math
\varepsilon_q = \tau_{\mathrm{applied}} - \tau
```

At 61.44 Msps, one sample is approximately 16.276 ns. Therefore, ceiling
quantization should satisfy:

```math
0 \leq \varepsilon_q < 16.276\ \mathrm{ns}
```

The quantization error is not expected to change linearly with elevation. It
depends on the fractional part of `N_exact`, while elevation changes the slant
range and the exact delay.

### Baseline Geometry Results

| Elevation | Slant range | Exact delay | Integer samples | Applied delay | Quantization error | FSPL |
|---:|---:|---:|---:|---:|---:|---:|
| 90° | 20.000 km | 66.712819 µs | 4099 | 66.715495 µs | 2.676 ns | 129.64 dB |
| 60° | 23.082 km | 76.993292 µs | 4731 | 77.001953 µs | 8.661 ns | 130.89 dB |
| 30° | 39.814 km | 132.806663 µs | 8160 | 132.812500 µs | 5.837 ns | 135.62 dB |
| 10° | 109.910 km | 366.619393 µs | 22526 | 366.634115 µs | 14.721 ns | 144.44 dB |

The delay, sample count, and FSPL increase as the elevation angle decreases
because the HAPS–UE slant range increases. All four quantization errors remain
below one sample period.

## How to Read the Dataset

- `record_id` is the unique identifier for a validation observation.
- `test_group` identifies the validation family.
- `reference_standard` identifies the main model or standard used for the
  comparison.
- Columns ending in `_oai` contain model output, while matching `_reference`
  columns contain the expected value.
- `channel_mode` records forced or probabilistic LOS/NLOS operation.
- `realized_state` records the LOS/NLOS state selected for that observation.
- `source_sheet` and `source_row` preserve the origin of manually consolidated
  results.
- Empty cells mean that a field is not applicable or was not recorded for that
  test.
- `link_status` records the observed end-to-end connection result. For
  `no_connection` rows, `snr_db` is intentionally left empty.

## Comparison Rules

Use the following checks when repeating a validation case:

1. Keep the carrier frequency, sample rate, geometry, scenario, LOS mode,
   terminal type, and random seed equal to the reference row.
2. Compare OAI output with the corresponding reference fields at the precision
   stored in the CSV.
3. For probabilistic LOS, verify that the realized state follows
   `random_draw < plos_oai` for LOS and otherwise NLOS.
4. For delay, verify the `ceil` rule and confirm that the quantization error is
   non-negative and less than one sample period.
5. Treat `link_status` as an integration observation, not as proof that an
   individual propagation formula is correct.

## Adding New Results

When adding a validation row:

1. Use a unique, descriptive `record_id`.
2. Reuse an existing `test_group`, or document a new group in this file.
3. Store numeric values without unit text; units are defined in column names.
4. Leave non-applicable fields empty instead of inserting zero.
5. Record the standard, scenario, channel mode, realized state, and input
   conditions required to reproduce the result.
6. Use `connected` or `no_connection` in `link_status`.
7. Add a short note when the row needs interpretation or has a special test
   condition.

## Scope and Limitations

The current validation data covers static geometry, integer-sample delay, and
selected large-scale propagation effects. It does not validate fractional
delay, dynamic geometry, geometric Doppler, NTN-TDL fast fading, MIMO, cloud
attenuation, scintillation, or HAPS-specific SIB19 signaling.

