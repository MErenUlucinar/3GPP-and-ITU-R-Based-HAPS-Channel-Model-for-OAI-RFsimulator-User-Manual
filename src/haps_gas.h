#ifndef HAPS_GAS_H
#define HAPS_GAS_H

typedef struct {
  double oxygen_specific_db_per_km;
  double oxygen_equivalent_height_km;

  double oxygen_loss_db;

  double water_kv_db_per_kg_m2;
  double water_loss_db;

  double total_loss_db;
} haps_gas_result_t;

/*
 * ITU-R P.676-13 Annex 2 instantaneous
 * gaseous attenuation calculation.
 *
 * Current coefficient-table scope:
 *   1 GHz <= f <= 10 GHz
 *
 * Current project carrier:
 *   3.6192 GHz
 */
int haps_gas_compute_p676_13(
    double carrier_frequency_hz,
    double elevation_deg,
    double surface_pressure_hpa,
    double surface_temperature_k,
    double surface_water_vapour_density_g_m3,
    double integrated_water_vapour_kg_m2,
    haps_gas_result_t *result);

#endif
