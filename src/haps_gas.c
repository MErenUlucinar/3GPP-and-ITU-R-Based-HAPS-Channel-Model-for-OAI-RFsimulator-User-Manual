#include "haps_gas.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#define HAPS_DEG_TO_RAD \
  0.01745329251994329577

typedef struct {
  double f0;
  double a1;
  double a2;
  double a3;
  double a4;
  double a5;
  double a6;
} p676_oxygen_line_t;

typedef struct {
  double frequency_ghz;
  double c1;
  double c2;
  double c3;
  double c4;
} p676_coeff_row_t;

/*
 * ITU-R P.676-13 Annex 1, Table 1:
 * Oxygen spectroscopic data.
 */
static const p676_oxygen_line_t oxygen_lines[] = {
    {50.474214, 0.975, 9.651, 6.690,
     0.0, 2.566, 6.850},

    {50.987745, 2.529, 8.653, 7.170,
     0.0, 2.246, 6.800},

    {51.503360, 6.193, 7.709, 7.640,
     0.0, 1.947, 6.729},

    {52.021429, 14.320, 6.819, 8.110,
     0.0, 1.667, 6.640},

    {52.542418, 31.240, 5.983, 8.580,
     0.0, 1.388, 6.526},

    {53.066934, 64.290, 5.201, 9.060,
     0.0, 1.349, 6.206},

    {53.595775, 124.600, 4.474, 9.550,
     0.0, 2.227, 5.085},

    {54.130025, 227.300, 3.800, 9.960,
     0.0, 3.170, 3.750},

    {54.671180, 389.700, 3.182, 10.370,
     0.0, 3.558, 2.654},

    {55.221384, 627.100, 2.618, 10.890,
     0.0, 2.560, 2.952},

    {55.783815, 945.300, 2.109, 11.340,
     0.0, -1.172, 6.135},

    {56.264774, 543.400, 0.014, 17.030,
     0.0, 3.525, -0.978},

    {56.363399, 1331.800, 1.654, 11.890,
     0.0, -2.378, 6.547},

    {56.968211, 1746.600, 1.255, 12.230,
     0.0, -3.545, 6.451},

    {57.612486, 2120.100, 0.910, 12.620,
     0.0, -5.416, 6.056},

    {58.323877, 2363.700, 0.621, 12.950,
     0.0, -1.932, 0.436},

    {58.446588, 1442.100, 0.083, 14.910,
     0.0, 6.768, -1.273},

    {59.164204, 2379.900, 0.387, 13.530,
     0.0, -6.561, 2.309},

    {59.590983, 2090.700, 0.207, 14.080,
     0.0, 6.957, -0.776},

    {60.306056, 2103.400, 0.207, 14.150,
     0.0, -6.395, 0.699},

    {60.434778, 2438.000, 0.386, 13.390,
     0.0, 6.342, -2.825},

    {61.150562, 2479.500, 0.621, 12.920,
     0.0, 1.014, -0.584},

    {61.800158, 2275.900, 0.910, 12.630,
     0.0, 5.014, -6.619},

    {62.411220, 1915.400, 1.255, 12.170,
     0.0, 3.029, -6.759},

    {62.486253, 1503.000, 0.083, 15.130,
     0.0, -4.499, 0.844},

    {62.997984, 1490.200, 1.654, 11.740,
     0.0, 1.856, -6.675},

    {63.568526, 1078.000, 2.108, 11.340,
     0.0, 0.658, -6.139},

    {64.127775, 728.700, 2.617, 10.880,
     0.0, -3.036, -2.895},

    {64.678910, 461.300, 3.181, 10.380,
     0.0, -3.968, -2.590},

    {65.224078, 274.000, 3.800, 9.960,
     0.0, -3.528, -3.680},

    {65.764779, 153.000, 4.473, 9.550,
     0.0, -2.548, -5.002},

    {66.302096, 80.400, 5.200, 9.060,
     0.0, -1.660, -6.091},

    {66.836834, 39.800, 5.982, 8.580,
     0.0, -1.680, -6.393},

    {67.369601, 18.560, 6.818, 8.110,
     0.0, -1.956, -6.475},

    {67.900868, 8.172, 7.708, 7.640,
     0.0, -2.216, -6.545},

    {68.431006, 3.397, 8.652, 7.170,
     0.0, -2.492, -6.600},

    {68.960312, 1.334, 9.650, 6.690,
     0.0, -2.773, -6.650},

    {118.750334, 940.300, 0.010, 16.640,
     0.0, -0.439, 0.079},

    {368.498246, 67.400, 0.048, 16.400,
     0.0, 0.000, 0.000},

    {424.763020, 637.700, 0.044, 16.400,
     0.0, 0.000, 0.000},

    {487.249273, 237.400, 0.049, 16.000,
     0.0, 0.000, 0.000},

    {715.392902, 98.100, 0.145, 16.000,
     0.0, 0.000, 0.000},

    {773.839490, 572.300, 0.141, 16.200,
     0.0, 0.000, 0.000},

    {834.145546, 183.100, 0.145, 14.700,
     0.0, 0.000, 0.000},
};

/*
 * ITU-R P.676-13 related data file Part 1.
 *
 * Columns:
 * frequency, a_o, b_o, c_o, d_o
 *
 * 1-10 GHz subset.
 */
static const p676_coeff_row_t
    oxygen_height_coeffs[] = {
        {1.0, -2.700258e+00, 2.724587e-02,
         5.971574e-04, 5.130385e-04},

        {1.5, -2.462805e+00, 2.715168e-02,
         -1.203684e-05, -7.200561e-05},

        {2.0, -2.378094e+00, 2.717326e-02,
         -2.529529e-04, -2.835906e-04},

        {2.5, -2.338989e+00, 2.719666e-02,
         -3.702004e-04, -3.836344e-04},

        {3.0, -2.318075e+00, 2.721466e-02,
         -4.355725e-04, -4.391716e-04},

        {3.5, -2.305844e+00, 2.722870e-02,
         -4.756334e-04, -4.736482e-04},

        {4.0, -2.298307e+00, 2.724034e-02,
         -5.019467e-04, -4.969635e-04},

        {4.5, -2.293550e+00, 2.725062e-02,
         -5.201752e-04, -5.138778e-04},

        {5.0, -2.290567e+00, 2.726019e-02,
         -5.333499e-04, -5.269089e-04},

        {5.5, -2.288783e+00, 2.726944e-02,
         -5.432096e-04, -5.374891e-04},

        {6.0, -2.287853e+00, 2.727865e-02,
         -5.508087e-04, -5.464816e-04},

        {6.5, -2.287558e+00, 2.728797e-02,
         -5.568161e-04, -5.544314e-04},

        {7.0, -2.287754e+00, 2.729753e-02,
         -5.616726e-04, -5.616960e-04},

        {7.5, -2.288343e+00, 2.730739e-02,
         -5.656783e-04, -5.685178e-04},

        {8.0, -2.289256e+00, 2.731762e-02,
         -5.690426e-04, -5.750656e-04},

        {8.5, -2.290444e+00, 2.732823e-02,
         -5.719159e-04, -5.814595e-04},

        {9.0, -2.291868e+00, 2.733927e-02,
         -5.744078e-04, -5.877869e-04},

        {9.5, -2.293501e+00, 2.735073e-02,
         -5.766000e-04, -5.941126e-04},

        {10.0, -2.295323e+00, 2.736264e-02,
         -5.785541e-04, -6.004850e-04},
};

/*
 * ITU-R P.676-13 related data file Part 2.
 *
 * Columns:
 * frequency, a_V, b_V, c_V, d_V
 *
 * 1-10 GHz subset.
 */
static const p676_coeff_row_t
    water_kv_coeffs[] = {
        {1.0, 1.175406e-05, 5.186823e-08,
         -4.146202e-08, 5.531196e-09},

        {1.5, 2.645736e-05, 1.167400e-07,
         -9.332594e-08, 1.246423e-08},

        {2.0, 4.706231e-05, 2.076291e-07,
         -1.660041e-07, 2.220644e-08},

        {2.5, 7.358981e-05, 3.246062e-07,
         -2.595672e-07, 3.479479e-08},

        {3.0, 1.060677e-04, 4.677639e-07,
         -3.741083e-07, 5.027821e-08},

        {3.5, 1.445313e-04, 6.372198e-07,
         -5.097459e-07, 6.871883e-08},

        {4.0, 1.890246e-04, 8.331178e-07,
         -6.666258e-07, 9.019336e-08},

        {4.5, 2.396014e-04, 1.055632e-06,
         -8.449248e-07, 1.147950e-07},

        {5.0, 2.963266e-04, 1.304969e-06,
         -1.044855e-06, 1.426360e-07},

        {5.5, 3.592784e-04, 1.581375e-06,
         -1.266670e-06, 1.738499e-07},

        {6.0, 4.285505e-04, 1.885137e-06,
         -1.510668e-06, 2.085959e-07},

        {6.5, 5.042551e-04, 2.216592e-06,
         -1.777203e-06, 2.470624e-07},

        {7.0, 5.865269e-04, 2.576134e-06,
         -2.066693e-06, 2.894730e-07},

        {7.5, 6.755272e-04, 2.964222e-06,
         -2.379632e-06, 3.360922e-07},

        {8.0, 7.714504e-04, 3.381387e-06,
         -2.716601e-06, 3.872341e-07},

        {8.5, 8.745313e-04, 3.828249e-06,
         -3.078288e-06, 4.432716e-07},

        {9.0, 9.850547e-04, 4.305524e-06,
         -3.465507e-06, 5.046487e-07},

        {9.5, 1.103368e-03, 4.814040e-06,
         -3.879222e-06, 5.718950e-07},

        {10.0, 1.229897e-03, 5.354749e-06,
         -4.320579e-06, 6.456442e-07},
};

static int interpolate_coefficients(
    const p676_coeff_row_t *table,
    size_t count,
    double frequency_ghz,
    double *c1,
    double *c2,
    double *c3,
    double *c4)
{
  if (table == NULL ||
      count < 2 ||
      c1 == NULL ||
      c2 == NULL ||
      c3 == NULL ||
      c4 == NULL) {
    return -1;
  }

  if (frequency_ghz <
          table[0].frequency_ghz ||
      frequency_ghz >
          table[count - 1].frequency_ghz) {
    return -2;
  }

  if (frequency_ghz ==
      table[count - 1].frequency_ghz) {
    *c1 = table[count - 1].c1;
    *c2 = table[count - 1].c2;
    *c3 = table[count - 1].c3;
    *c4 = table[count - 1].c4;

    return 0;
  }

  for (size_t i = 0;
       i + 1 < count;
       ++i) {

    if (frequency_ghz >=
            table[i].frequency_ghz &&
        frequency_ghz <=
            table[i + 1].frequency_ghz) {

      const double x0 =
          table[i].frequency_ghz;

      const double x1 =
          table[i + 1].frequency_ghz;

      const double alpha =
          (frequency_ghz - x0) /
          (x1 - x0);

      *c1 =
          table[i].c1 +
          alpha *
              (table[i + 1].c1 -
               table[i].c1);

      *c2 =
          table[i].c2 +
          alpha *
              (table[i + 1].c2 -
               table[i].c2);

      *c3 =
          table[i].c3 +
          alpha *
              (table[i + 1].c3 -
               table[i].c3);

      *c4 =
          table[i].c4 +
          alpha *
              (table[i + 1].c4 -
               table[i].c4);

      return 0;
    }
  }

  return -3;
}

static int
oxygen_specific_attenuation_db_per_km(
    double frequency_ghz,
    double total_pressure_hpa,
    double temperature_k,
    double water_vapour_density_g_m3,
    double *gamma_o_db_per_km)
{
  if (gamma_o_db_per_km == NULL ||
      frequency_ghz <= 0.0 ||
      total_pressure_hpa <= 0.0 ||
      temperature_k <= 0.0 ||
      water_vapour_density_g_m3 < 0.0) {
    return -1;
  }

  /*
   * P.676-13 equation (4).
   */
  const double e =
      water_vapour_density_g_m3 *
      temperature_k /
      216.7;

  const double p =
      total_pressure_hpa - e;

  if (p <= 0.0)
    return -2;

  const double theta =
      300.0 / temperature_k;

  double n_oxygen = 0.0;

  for (size_t i = 0;
       i <
       sizeof(oxygen_lines) /
           sizeof(oxygen_lines[0]);
       ++i) {

    const p676_oxygen_line_t *line =
        &oxygen_lines[i];

    /*
     * P.676-13 equation (3):
     * oxygen line strength.
     */
    const double strength =
        line->a1 *
        1.0e-7 *
        p *
        pow(theta, 3.0) *
        exp(
            line->a2 *
            (1.0 - theta));

    /*
     * P.676-13 equation (6a).
     */
    double delta_f =
        line->a3 *
        1.0e-4 *
        (
            p *
                pow(
                    theta,
                    0.8 - line->a4)
            +
            1.1 *
                e *
                theta
        );

    /*
     * P.676-13 equation (6b).
     */
    delta_f =
        sqrt(
            delta_f * delta_f +
            2.25e-6);

    /*
     * P.676-13 equation (7).
     */
    const double delta =
        (
            line->a5 +
            line->a6 * theta
        ) *
        1.0e-4 *
        (p + e) *
        pow(theta, 0.8);

    const double lower =
        line->f0 - frequency_ghz;

    const double upper =
        line->f0 + frequency_ghz;

    /*
     * P.676-13 equation (5):
     * oxygen line-shape factor.
     */
    const double line_shape =
        (frequency_ghz / line->f0) *
        (
            (
                delta_f -
                delta * lower
            ) /
            (
                lower * lower +
                delta_f * delta_f
            )
            +
            (
                delta_f -
                delta * upper
            ) /
            (
                upper * upper +
                delta_f * delta_f
            )
        );

    n_oxygen +=
        strength * line_shape;
  }

  /*
   * P.676-13 equation (9).
   */
  const double d =
      5.6e-4 *
      (p + e) *
      pow(theta, 0.8);

  /*
   * P.676-13 equation (8):
   * dry-air continuum.
   */
  const double dry_continuum =
      frequency_ghz *
      p *
      theta *
      theta *
      (
          6.14e-5 /
          (
              d *
              (
                  1.0 +
                  pow(
                      frequency_ghz / d,
                      2.0)
              )
          )
          +
          1.4e-12 *
              p *
              pow(theta, 1.5) /
          (
              1.0 +
              1.9e-5 *
              pow(
                  frequency_ghz,
                  1.5)
          )
      );

  n_oxygen += dry_continuum;

  /*
   * P.676-13 equation (30).
   */
  *gamma_o_db_per_km =
      0.1820 *
      frequency_ghz *
      n_oxygen;

  return
      isfinite(*gamma_o_db_per_km)
          ? 0
          : -3;
}

int haps_gas_compute_p676_13(
    double carrier_frequency_hz,
    double elevation_deg,
    double surface_pressure_hpa,
    double surface_temperature_k,
    double surface_water_vapour_density_g_m3,
    double integrated_water_vapour_kg_m2,
    haps_gas_result_t *result)
{
  if (result == NULL)
    return -1;

  memset(
      result,
      0,
      sizeof(*result));

  const double frequency_ghz =
      carrier_frequency_hz /
      1.0e9;

  /*
   * Bu uygulamada gömülü katsayılar
   * 1-10 GHz aralığıyla sınırlıdır.
   */
  if (frequency_ghz < 1.0 ||
      frequency_ghz > 10.0) {
    return -2;
  }

  /*
   * Annex 2 sin(theta) yaklaşımı için
   * ilk sürümde 5-90 derece aralığı.
   */
  if (elevation_deg < 5.0 ||
      elevation_deg > 90.0) {
    return -3;
  }

  if (surface_pressure_hpa <= 0.0 ||
      surface_temperature_k <= 0.0 ||
      surface_water_vapour_density_g_m3 < 0.0 ||
      integrated_water_vapour_kg_m2 < 0.0) {
    return -4;
  }

  const double sin_elevation =
      sin(
          elevation_deg *
          HAPS_DEG_TO_RAD);

  if (sin_elevation <= 0.0)
    return -5;

  double ao = 0.0;
  double bo = 0.0;
  double co = 0.0;
  double do_ = 0.0;

  int rc =
      interpolate_coefficients(
          oxygen_height_coeffs,
          sizeof(oxygen_height_coeffs) /
              sizeof(
                  oxygen_height_coeffs[0]),
          frequency_ghz,
          &ao,
          &bo,
          &co,
          &do_);

  if (rc != 0)
    return -6;

  rc =
      oxygen_specific_attenuation_db_per_km(
          frequency_ghz,
          surface_pressure_hpa,
          surface_temperature_k,
          surface_water_vapour_density_g_m3,
          &result->
              oxygen_specific_db_per_km);

  if (rc != 0)
    return -7;

  /*
   * P.676-13 equation (31):
   * oxygen equivalent height.
   */
  result->oxygen_equivalent_height_km =
      ao +
      bo * surface_temperature_k +
      co * surface_pressure_hpa +
      do_ *
          surface_water_vapour_density_g_m3;

  if (result->
          oxygen_equivalent_height_km <=
      0.0) {
    return -8;
  }

  /*
   * P.676-13 equation (29).
   */
  result->oxygen_loss_db =
      result->
          oxygen_specific_db_per_km *
      result->
          oxygen_equivalent_height_km /
      sin_elevation;

  double av = 0.0;
  double bv = 0.0;
  double cv = 0.0;
  double dv = 0.0;

  rc =
      interpolate_coefficients(
          water_kv_coeffs,
          sizeof(water_kv_coeffs) /
              sizeof(water_kv_coeffs[0]),
          frequency_ghz,
          &av,
          &bv,
          &cv,
          &dv);

  if (rc != 0)
    return -9;

  /*
   * P.676-13 equation (39):
   *
   * K_V =
   *   a_V +
   *   b_V rho +
   *   c_V T +
   *   d_V P
   */
  result->water_kv_db_per_kg_m2 =
      av +
      bv *
          surface_water_vapour_density_g_m3 +
      cv *
          surface_temperature_k +
      dv *
          surface_pressure_hpa;

  if (result->
          water_kv_db_per_kg_m2 <
      0.0) {
    return -10;
  }

  /*
   * P.676-13 equation (38).
   */
  result->water_loss_db =
      result->
          water_kv_db_per_kg_m2 *
      integrated_water_vapour_kg_m2 /
      sin_elevation;

  result->total_loss_db =
      result->oxygen_loss_db +
      result->water_loss_db;

  return
      isfinite(result->total_loss_db)
          ? 0
          : -11;
}
