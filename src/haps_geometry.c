#include "haps_geometry.h"

#include <math.h>
#include <string.h>

#define HAPS_PI 3.14159265358979323846
#define SPEED_OF_LIGHT_MPS 299792458.0

static double deg_to_rad(double degrees)
{
  return degrees * HAPS_PI / 180.0;
}

static double rad_to_deg(double radians)
{
  return radians * 180.0 / HAPS_PI;
}

void haps_geodetic_to_ecef(
    double latitude_deg,
    double longitude_deg,
    double altitude_m,
    double ecef_m[3])
{
  /*
   * WGS-84 parametreleri.
   */
  const double semi_major_axis_m = 6378137.0;
  const double flattening = 1.0 / 298.257223563;
  const double eccentricity_sq =
      flattening * (2.0 - flattening);

  const double latitude_rad = deg_to_rad(latitude_deg);
  const double longitude_rad = deg_to_rad(longitude_deg);

  const double sin_lat = sin(latitude_rad);
  const double cos_lat = cos(latitude_rad);

  const double sin_lon = sin(longitude_rad);
  const double cos_lon = cos(longitude_rad);

  const double prime_vertical_radius =
      semi_major_axis_m /
      sqrt(1.0 - eccentricity_sq * sin_lat * sin_lat);

  ecef_m[0] =
      (prime_vertical_radius + altitude_m) *
      cos_lat * cos_lon;

  ecef_m[1] =
      (prime_vertical_radius + altitude_m) *
      cos_lat * sin_lon;

  ecef_m[2] =
      (prime_vertical_radius *
       (1.0 - eccentricity_sq) +
       altitude_m) *
      sin_lat;
}

int haps_compute_geometry(
    const haps_config_t *cfg,
    haps_geometry_result_t *result)
{
  if (cfg == NULL || result == NULL)
    return -1;

  memset(result, 0, sizeof(*result));

  haps_geodetic_to_ecef(
      cfg->haps_latitude_deg,
      cfg->haps_longitude_deg,
      cfg->haps_altitude_m,
      result->haps_ecef_m);

  haps_geodetic_to_ecef(
      cfg->ue_latitude_deg,
      cfg->ue_longitude_deg,
      cfg->ue_altitude_m,
      result->ue_ecef_m);

  const double dx =
      result->haps_ecef_m[0] - result->ue_ecef_m[0];

  const double dy =
      result->haps_ecef_m[1] - result->ue_ecef_m[1];

  const double dz =
      result->haps_ecef_m[2] - result->ue_ecef_m[2];

  result->slant_range_m =
      sqrt(dx * dx + dy * dy + dz * dz);

  if (result->slant_range_m <= 0.0)
    return -1;

  const double latitude_rad =
      deg_to_rad(cfg->ue_latitude_deg);

  const double longitude_rad =
      deg_to_rad(cfg->ue_longitude_deg);

  /*
   * UE konumundaki yerel Up birim vektörü.
   */
  const double up_x =
      cos(latitude_rad) * cos(longitude_rad);

  const double up_y =
      cos(latitude_rad) * sin(longitude_rad);

  const double up_z =
      sin(latitude_rad);

  const double projection_up =
      dx * up_x + dy * up_y + dz * up_z;

  double sine_elevation =
      projection_up / result->slant_range_m;

  if (sine_elevation > 1.0)
    sine_elevation = 1.0;

  if (sine_elevation < -1.0)
    sine_elevation = -1.0;

  result->elevation_deg =
      rad_to_deg(asin(sine_elevation));

  result->propagation_delay_s =
      result->slant_range_m / SPEED_OF_LIGHT_MPS;

  return 0;
}
