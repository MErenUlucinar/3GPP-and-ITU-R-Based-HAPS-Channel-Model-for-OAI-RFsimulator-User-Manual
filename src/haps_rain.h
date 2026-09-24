#ifndef HAPS_RAIN_H
#define HAPS_RAIN_H

#include <stdbool.h>

/*
 * Rain attenuation calculation modes.
 *
 * DISABLED:
 *   Rain attenuation is not applied.
 *
 * FIXED_UNIFORM_LAYER:
 *   Rain rate and rain height are supplied from the
 *   HAPS configuration file. The specific attenuation
 *   is calculated according to ITU-R P.838-3.
 *
 * A future ITU_STATISTICAL mode will use:
 *   ITU-R P.837-8
 *   ITU-R P.839-4
 *   ITU-R P.618-14
 *   ITU-R P.2041-0
 */
typedef enum {
  HAPS_RAIN_DISABLED = 0,
  HAPS_RAIN_FIXED_UNIFORM_LAYER = 1,
  HAPS_RAIN_ITU_STATISTICAL = 2
} haps_rain_mode_t;

/*
 * Input parameters for ITU-R P.838-3 rain attenuation.
 */
typedef struct {
  /*
   * Carrier frequency in GHz.
   * ITU-R P.838-3 validity range: 1–1000 GHz.
   */
  double frequency_ghz;

  /*
   * HAPS-to-UE elevation angle in degrees.
   */
  double elevation_deg;

  /*
   * Polarization tilt angle relative to horizontal.
   *
   * 0 degrees:
   *   horizontal linear polarization
   *
   * 90 degrees:
   *   vertical linear polarization
   *
   * 45 degrees:
   *   circular polarization or polarization-neutral
   *   controlled test configuration
   */
  double polarization_tilt_deg;

  /*
   * Rain rate in mm/h.
   */
  double rain_rate_mm_per_h;

  /*
   * Upper boundary of the uniform rain layer.
   */
  double rain_height_km;

  /*
   * UE/site altitude above mean sea level.
   */
  double station_height_km;
} haps_rain_input_t;

/*
 * Output of the ITU-R P.838-3 calculation.
 */
typedef struct {
  bool valid;

  /*
   * Horizontal and vertical polarization coefficients.
   */
  double k_h;
  double k_v;
  double alpha_h;
  double alpha_v;

  /*
   * Combined coefficients for the selected elevation
   * and polarization.
   */
  double k;
  double alpha;

  /*
   * Specific attenuation:
   *
   * gamma_R = k * R^alpha
   */
  double specific_attenuation_db_per_km;

  /*
   * Geometrical distance travelled inside the
   * uniform rain layer.
   */
  double geometric_rain_path_km;

  /*
   * Total attenuation in the fixed uniform rain layer.
   */
  double attenuation_db;
} haps_rain_result_t;

/*
 * Calculate rain attenuation according to ITU-R P.838-3.
 *
 * The current implementation calculates the exact
 * frequency- and polarization-dependent P.838-3
 * coefficients and uses a fixed uniform rain layer.
 *
 * Return:
 *   0  success
 *  <0  invalid input or calculation error
 */
int haps_rain_compute(
    const haps_rain_input_t *input,
    haps_rain_result_t *result);

#endif
