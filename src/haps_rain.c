#include "haps_rain.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

/*
 * ITU-R P.838-3
 *
 * Specific attenuation model for rain for use
 * in prediction methods.
 *
 * gamma_R = k * R^alpha
 *
 * The numerical coefficients below are taken from:
 *
 * Table 1: k_H
 * Table 2: k_V
 * Table 3: alpha_H
 * Table 4: alpha_V
 */

#define HAPS_RAIN_PI 3.14159265358979323846

typedef struct {
  double a;
  double b;
  double c;
} haps_p838_gaussian_term_t;

/*
 * ITU-R P.838-3 Table 1:
 * coefficients for log10(k_H).
 */
static const haps_p838_gaussian_term_t p838_kh_terms[] = {
    {-5.33980, -0.10008, 1.13098},
    {-0.35351,  1.26970, 0.45400},
    {-0.23789,  0.86036, 0.15354},
    {-0.94158,  0.64552, 0.16817}
};

static const double p838_kh_m = -0.18961;
static const double p838_kh_c =  0.71147;

/*
 * ITU-R P.838-3 Table 2:
 * coefficients for log10(k_V).
 */
static const haps_p838_gaussian_term_t p838_kv_terms[] = {
    {-3.80595,  0.56934, 0.81061},
    {-3.44965, -0.22911, 0.51059},
    {-0.39902,  0.73042, 0.11899},
    { 0.50167,  1.07319, 0.27195}
};

static const double p838_kv_m = -0.16398;
static const double p838_kv_c =  0.63297;

/*
 * ITU-R P.838-3 Table 3:
 * coefficients for alpha_H.
 */
static const haps_p838_gaussian_term_t p838_alphah_terms[] = {
    {-0.14318,  1.82442, -0.55187},
    { 0.29591,  0.77564,  0.19822},
    { 0.32177,  0.63773,  0.13164},
    {-5.37610, -0.96230,  1.47828},
    {16.17210, -3.29980,  3.43990}
};

static const double p838_alphah_m =  0.67849;
static const double p838_alphah_c = -1.95537;

/*
 * ITU-R P.838-3 Table 4:
 * coefficients for alpha_V.
 */
static const haps_p838_gaussian_term_t p838_alphav_terms[] = {
    { -0.07771, 2.33840, -0.76284},
    {  0.56727, 0.95545,  0.54039},
    { -0.20238, 1.14520,  0.26809},
    {-48.29910, 0.791669, 0.116226},
    { 48.58330, 0.791459, 0.116479}
};

static const double p838_alphav_m = -0.053739;
static const double p838_alphav_c =  0.83433;

static double degrees_to_radians(
    const double angle_deg)
{
  return angle_deg * HAPS_RAIN_PI / 180.0;
}

/*
 * Evaluate the Gaussian-sum curve-fitting expression
 * used by ITU-R P.838-3 equations (2) and (3).
 */
static double evaluate_p838_curve(
    const double frequency_ghz,
    const haps_p838_gaussian_term_t *terms,
    const size_t number_of_terms,
    const double linear_slope,
    const double constant)
{
  const double log_frequency =
      log10(frequency_ghz);

  double value =
      linear_slope * log_frequency +
      constant;

  for (size_t index = 0;
       index < number_of_terms;
       ++index) {

    const double normalized_difference =
        (log_frequency - terms[index].b) /
        terms[index].c;

    value +=
        terms[index].a *
        exp(
            -normalized_difference *
            normalized_difference);
  }

  return value;
}

static int calculate_p838_coefficients(
    const double frequency_ghz,
    const double elevation_deg,
    const double polarization_tilt_deg,
    haps_rain_result_t *result)
{
  if (result == NULL)
    return -1;

  /*
   * Equation (2):
   *
   * log10(k) =
   * sum(a_j exp(-((log10(f)-b_j)/c_j)^2))
   * + m log10(f) + c
   */
  const double log10_k_h =
      evaluate_p838_curve(
          frequency_ghz,
          p838_kh_terms,
          sizeof(p838_kh_terms) /
              sizeof(p838_kh_terms[0]),
          p838_kh_m,
          p838_kh_c);

  const double log10_k_v =
      evaluate_p838_curve(
          frequency_ghz,
          p838_kv_terms,
          sizeof(p838_kv_terms) /
              sizeof(p838_kv_terms[0]),
          p838_kv_m,
          p838_kv_c);

  result->k_h = pow(10.0, log10_k_h);
  result->k_v = pow(10.0, log10_k_v);

  /*
   * Equation (3):
   *
   * alpha =
   * sum(a_j exp(-((log10(f)-b_j)/c_j)^2))
   * + m log10(f) + c
   */
  result->alpha_h =
      evaluate_p838_curve(
          frequency_ghz,
          p838_alphah_terms,
          sizeof(p838_alphah_terms) /
              sizeof(p838_alphah_terms[0]),
          p838_alphah_m,
          p838_alphah_c);

  result->alpha_v =
      evaluate_p838_curve(
          frequency_ghz,
          p838_alphav_terms,
          sizeof(p838_alphav_terms) /
              sizeof(p838_alphav_terms[0]),
          p838_alphav_m,
          p838_alphav_c);

  const double elevation_rad =
      degrees_to_radians(elevation_deg);

  const double polarization_tilt_rad =
      degrees_to_radians(
          polarization_tilt_deg);

  /*
   * Polarization/elevation term used in
   * ITU-R P.838-3 equations (4) and (5).
   */
  const double polarization_geometry_term =
      cos(elevation_rad) *
      cos(elevation_rad) *
      cos(2.0 * polarization_tilt_rad);

  /*
   * ITU-R P.838-3 equation (4).
   */
  result->k =
      0.5 *
      (
          result->k_h +
          result->k_v +
          (
              result->k_h -
              result->k_v
          ) *
          polarization_geometry_term
      );

  if (!isfinite(result->k) ||
      result->k <= 0.0) {
    return -2;
  }

  /*
   * ITU-R P.838-3 equation (5).
   */
  result->alpha =
      (
          result->k_h * result->alpha_h +
          result->k_v * result->alpha_v +
          (
              result->k_h * result->alpha_h -
              result->k_v * result->alpha_v
          ) *
          polarization_geometry_term
      ) /
      (2.0 * result->k);

  if (!isfinite(result->alpha))
    return -3;

  return 0;
}

int haps_rain_compute(
    const haps_rain_input_t *input,
    haps_rain_result_t *result)
{
  if (input == NULL ||
      result == NULL) {
    return -1;
  }

  memset(result, 0, sizeof(*result));

  if (!isfinite(input->frequency_ghz) ||
      input->frequency_ghz < 1.0 ||
      input->frequency_ghz > 1000.0) {
    return -2;
  }

  if (!isfinite(input->elevation_deg) ||
      input->elevation_deg <= 0.0 ||
      input->elevation_deg > 90.0) {
    return -3;
  }

  if (!isfinite(
          input->polarization_tilt_deg)) {
    return -4;
  }

  if (!isfinite(input->rain_rate_mm_per_h) ||
      input->rain_rate_mm_per_h < 0.0) {
    return -5;
  }

  if (!isfinite(input->rain_height_km) ||
      !isfinite(input->station_height_km)) {
    return -6;
  }

  const int coefficient_result =
      calculate_p838_coefficients(
          input->frequency_ghz,
          input->elevation_deg,
          input->polarization_tilt_deg,
          result);

  if (coefficient_result != 0)
    return -7;

  /*
   * No rain rate means zero rain attenuation.
   */
  if (input->rain_rate_mm_per_h == 0.0) {
    result->valid = true;
    return 0;
  }

  /*
   * If the station is above the specified rain layer,
   * the model contains no rain segment.
   */
  if (input->rain_height_km <=
      input->station_height_km) {
    result->valid = true;
    return 0;
  }

  /*
   * ITU-R P.838-3 equation (1):
   *
   * gamma_R = k * R^alpha
   */
  result->specific_attenuation_db_per_km =
      result->k *
      pow(
          input->rain_rate_mm_per_h,
          result->alpha);

  if (!isfinite(
          result->
              specific_attenuation_db_per_km) ||
      result->
              specific_attenuation_db_per_km <
          0.0) {
    return -8;
  }

  const double elevation_rad =
      degrees_to_radians(
          input->elevation_deg);

  const double sine_elevation =
      sin(elevation_rad);

  if (!isfinite(sine_elevation) ||
      sine_elevation <= 0.0) {
    return -9;
  }

  /*
   * Controlled uniform-rain-layer geometry.
   *
   * This is intentionally kept separate from the
   * full P.618/P.2041 statistical effective-path
   * calculation.
   */
  result->geometric_rain_path_km =
      (
          input->rain_height_km -
          input->station_height_km
      ) /
      sine_elevation;

  result->attenuation_db =
      result->specific_attenuation_db_per_km *
      result->geometric_rain_path_km;

  if (!isfinite(result->attenuation_db) ||
      result->attenuation_db < 0.0) {
    return -10;
  }

  result->valid = true;

  return 0;
}
