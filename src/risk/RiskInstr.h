/**
 * @file      RiskInstr.h
 * @brief     Header of RiskInstr
 * @date      Mon Dec 13 10:41:59 2021
 * @author    Yong
 * @copyright BSD-3-Clause
 *
 * This module provides risk management of a specific instrument.
 * Its instance is linked to each instrument of the trading account.
 */

#ifndef SRC_RISK_RISKINSTR_H
#define SRC_RISK_RISKINSTR_H

#include "ATStructure.h"
#include "AccBase.h"
#include "json.hpp"
using json = nlohmann::json;

class RiskInstr {
 public:
  bool init(const json& j_conf, const tInstrumentInfo* p_instr_info,
            AccBase* ab);
  int check(int dir, int off, int vol, long nano);
  int parseAutoOffset(int dir, int& off, int vol);

  void onNew(long nano);

  int intensity_cycle_order_count = 0;
  long intensity_cycle_time_record = 0;

  UnitVol* p_unit_vol = nullptr;

 protected:
  bool IsIntensityNormal(long nano);
  bool IsSizeCorrect(int size);
  bool IsInstrumentLongVolumeCorrect(int vol);
  bool IsInstrumentShortVolumeCorrect(int vol);
  bool IsInstrumentNetVolumeCorrect(int dir, int vol);

  int intensity_cycle_order_count_threshold = 0;
  double intensity_cycle_time_span_threshold = 0.0;
  bool is_allowed = false;
  int allowed_order_size_threshold = 0;
  int long_volume_threshold = 0;
  int short_volume_threshold = 0;
  int net_volume_threshold = 0;
};

#endif
