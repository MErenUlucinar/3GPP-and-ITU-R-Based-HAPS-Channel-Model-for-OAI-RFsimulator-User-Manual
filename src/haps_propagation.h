#ifndef HAPS_PROPAGATION_H
#define HAPS_PROPAGATION_H

#include <stdbool.h>

#include "haps_config.h"
#include "haps_geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /*
   * 3GPP büyük ölçekli kanal durumu.
   */
  bool is_los;

  /*
   * Gerçek elevation yerine kullanılan en yakın
   * 3GPP referans açısı: 10, 20, ..., 90 derece.
   */
  double reference_elevation_deg;

  /*
   * 3GPP TR 38.811 LOS olasılığı.
   */
  double los_probability;

  /*
   * Probabilistic modda yapılan rastgele çekiliş.
   * Forced modlarda -1.0 değerindedir.
   */
  double los_uniform_draw;

  /*
   * Serbest uzay kaybı.
   */
  double fspl_db;

  /*
   * 3GPP clutter ve shadow fading bileşenleri.
   */
  double clutter_loss_db;
  double shadow_sigma_db;
  double shadow_fading_db;

  /*
   * Sonraki aşamalarda eklenecek bileşenler.
   */
  double building_entry_loss_db;
 
  double oxygen_gas_loss_db;
  double water_vapour_gas_loss_db;
  double gas_loss_db;

   double rain_k;
  double rain_alpha;

  double rain_specific_attenuation_db_per_km;
  double rain_path_km;
  

  double rain_loss_db;
  double cloud_loss_db;
  double scintillation_loss_db;

  /*
   * Fiziksel toplam kanal kaybı.
   */
  double total_loss_db;

  /*
   * OAI normalize IQ alanında uygulanan net kayıp:
   *
   * applied_loss_db =
   *     total_loss_db - link_budget_offset_db
   */
  double applied_loss_db;

  /*
   * IQ örneklerine uygulanan genlik katsayısı.
   */
  double linear_amplitude_gain;
} haps_propagation_result_t;

double haps_fspl_db(
    double distance_m,
    double frequency_hz);

int haps_compute_propagation(
    const haps_config_t *cfg,
    const haps_geometry_result_t *geometry,
    haps_propagation_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
