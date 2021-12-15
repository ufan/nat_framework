/**
 * @file      test.cpp<test>
 * @brief     Header of
 * @date      Tue Dec 14 13:53:48 2021
 * @author    Yong
 * @copyright BSD-3-Clause
 *
 * This module tests risk management
 */

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include "ATConstants.h"
#include "ATStructure.h"
#include "CTradeBaseInfo.h"
#include "MurmurHash2.h"
#include "RiskStg.h"
#include "RiskTop.h"
#include "json.hpp"
#include "time.h"

using namespace std;
using json = nlohmann::json;

int main() {
  json j_conf = R"({
		"RiskForGeneral":
		{
			"allowed_instrument":["All"],
			"allowed_max_unfilled_order_number":1
		},
		"RiskForOrder":
		{
			"allowed_price_tick":
			{
				"default":100000
			},
			"allowed_order_size":
			{
				"default":200000
			},
			"intensity_cycle_order_count":
			{
				"default":10
			},
			"intensity_cycle_time_span":
			{
				"default":0
			}
		},
		"RiskForInstrument":
		{
			"allowed_long_volume":
			{
				"default":1000000
			},
			"allowed_short_volume":
			{
				"default":200000000
			},
			"allowed_net_volume":
			{
				"default":20000000
			}
		},
		"RiskForProduct":
		{
			"allowed_long_volume":
			{
				"default":200000000
			},
			"allowed_short_volume":
			{
				"default":20000000
			},
			"allowed_net_volume":
			{
				"default":200000000
			}
		},
		"RiskForAccount":
		{
			"allowed_long_amount":10000000000.00,
			"allowed_short_amount":10000000000.00,
			"allowed_net_amount":10000000000.00
		}
	})"_json;

  const char* p_instr_name = "rb1901";
  const char* p_prd_name = "rb";
  uint32_t instr_hash = INSTR_NAME_TO_HASH(p_instr_name);
  uint32_t prd_hash = INSTR_NAME_TO_HASH(p_prd_name);
  tInstrumentInfo* p_instr_info = &(CTradeBaseInfo::instr_info_[instr_hash]);
  p_instr_info->instr_hash = instr_hash;
  strcpy(p_instr_info->instr, p_instr_name);
  p_instr_info->product_hash = prd_hash;
  strcpy(p_instr_info->product, p_prd_name);
  p_instr_info->vol_multiple = 10;
  p_instr_info->tick_price = 1.0;

  RiskStg* p_risk = new RiskStg;
  const char* p_name = "test";

  if (!p_risk->init(p_name, j_conf)) {
    printf("can't init risk module!\n");
  }

  if (!p_risk->regInstr(p_instr_name)) {
    printf("can't reg instr %s!\n", p_instr_name);
  }

  int dir = AT_CHAR_Buy;
  int off = AT_CHAR_Open;
  tSubsInstrInfo* p_subs_info = new tSubsInstrInfo;
  p_subs_info->base_info = *p_instr_info;
  p_subs_info->lst_tick = *(new tLastTick);
  timespec ts1, ts2;
  long ts_sum = 0;
  for (int i = 0; i < 1000; ++i) {
    clock_gettime(CLOCK_REALTIME, &ts1);
    int ret = p_risk->check(dir, off, i + 3500.0, 1, p_subs_info);
    clock_gettime(CLOCK_REALTIME, &ts2);
    ts_sum +=
        (ts2.tv_sec - ts1.tv_sec) * 1000000000L + ts2.tv_nsec - ts1.tv_nsec;
    if (ret != 0) {
      printf("risk check failed: %d\n", ret);
      getchar();
    }
  }
  printf("ts_sum = %ld\n", ts_sum);

  return 0;
}
