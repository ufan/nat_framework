#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "Constant.h"
#include "string.h"

struct Instrument {
	char instr_str[32] = {0};
	char exch_str[9] = {0};
	char prd_str[32] = {0};
	ProductClassType prd_cls;
	int volume_multiple = 0;
	double price_tick = 0.0;
	char underlying_instr_str[32] = {0};
	char open_date[9] = {0};
	char expire_date[9] = {0};
	double strike_px = 0.0;
	OptionsType opt_type;
	int prd_idx = -1;
	
	bool operator< (const Instrument& instr) const {
		if (instr_str[4] < instr.instr_str[4]) {
			return true;
		} else if (instr_str[4] > instr.instr_str[4]) {
			return false;
		} else {
			if (strcmp(instr_str, instr.instr_str) < 0) {
				return true;
			} else {
				return false;
			}
		}
	}
};

struct Instrument_v1 {
	char instr_str[32] = {0};
	char exch_str[9] = {0};
	char prd_str[32] = {0};
	ProductClassType prd_cls;
	int volume_multiple = 0;
	double price_tick = 0.0;
	char underlying_instr_str[32] = {0};
	char open_date[9] = {0};
	char expire_date[9] = {0};
	double strike_px = 0.0;
	OptionsType opt_type;
	int prd_idx = -1;
	
	bool operator< (const Instrument& instr) const {
		if (instr_str[4] < instr.instr_str[4]) {
			return true;
		} else if (instr_str[4] > instr.instr_str[4]) {
			return false;
		} else {
			if (strcmp(instr_str, instr.instr_str) < 0) {
				return true;
			} else {
				return false;
			}
		}
	}
};

#endif
