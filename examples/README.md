# Example OAI Configuration Set

The example set uses one UE configuration and two gNB profiles. The gNB
profile is selected according to whether integer-sample propagation delay is
being tested.

| File | Purpose |
|---|---|
| `gnb.sa.band78.fr1.106PRB.haps.rfsim.conf` | gNB profile for the baseline test with propagation delay disabled |
| `gnb.sa.band78.fr1.106PRB.haps.rfsim.backup.conf` | gNB profile for the propagation-delay test; includes static NTN/SIB19 timing-assistance fields |
| `ue.haps.conf` | Common UE profile used in both tests |
| `channelmod_haps_3gpp_itu.conf` | HAPS geometry, propagation, loss-component, delay, and Doppler parameters |

## Scenario Selection

| Scenario | gNB configuration | UE configuration | `enable_propagation_delay` |
|---|---|---|---:|
| Delay disabled | `gnb.sa.band78.fr1.106PRB.haps.rfsim.conf` | `ue.haps.conf` | `0` in both `hapsmod` records |
| Delay enabled | `gnb.sa.band78.fr1.106PRB.haps.rfsim.backup.conf` | `ue.haps.conf` | `1` in both `hapsmod` records |

Selecting the second gNB file does **not** enable the HAPS delay by itself.
The `enable_propagation_delay` flag in
`channelmod_haps_3gpp_itu.conf` controls whether the calculated delay is
transferred to RFsimulator. Keep the downlink and uplink records consistent.

## Values That Must Be Adapted

Before running the examples:

1. Set `amf_ip_address` in the selected gNB configuration to the address of
   your AMF.
2. Replace the demo `imsi`, `key`, and `opc` values in `ue.haps.conf` with a
   subscriber provisioned in your 5GC. Never commit operational credentials.
3. Ensure that the MCC, MNC, TAC, slice, and DNN values agree across the gNB,
   UE, and 5GC configurations.
4. Keep `channelmod_haps_3gpp_itu.conf` in the include path used by both OAI
   processes.
5. If gNB and UE run on different hosts, update the RFsimulator server
   addresses accordingly.

## Example Launch Commands

Run the following commands from the OAI build directory and replace
`<repo>/examples` with the absolute path to this directory.

Delay disabled:

```bash
./nr-softmodem \
  -O <repo>/examples/gnb.sa.band78.fr1.106PRB.haps.rfsim.conf \
  --rfsim
```

Delay enabled:

```bash
./nr-softmodem \
  -O <repo>/examples/gnb.sa.band78.fr1.106PRB.haps.rfsim.backup.conf \
  --rfsim
```

Use the same UE file in both cases:

```bash
./nr-uesoftmodem \
  -O <repo>/examples/ue.haps.conf \
  --rfsim -r 106 --numerology 1 --band 78 -C 3619200000
```

The delay-enabled profile assumes that the integer delay produced by
`haps_channel_get_delay_info()` is connected to OAI's `channel_offset`
mechanism. The SIB19 values in the example are static engineering-test
values, not dynamically generated HAPS ephemeris.
