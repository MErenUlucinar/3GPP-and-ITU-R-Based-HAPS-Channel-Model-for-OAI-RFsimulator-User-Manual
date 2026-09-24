#ifndef HAPS_GEOMETRY_H
#define HAPS_GEOMETRY_H

#include "haps_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double haps_ecef_m[3];
  double ue_ecef_m[3];

  double slant_range_m;
  double elevation_deg;

  double propagation_delay_s;
} haps_geometry_result_t;

void haps_geodetic_to_ecef(
    double latitude_deg,
    double longitude_deg,
    double altitude_m,
    double ecef_m[3]);

int haps_compute_geometry(
    const haps_config_t *cfg,
    haps_geometry_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
