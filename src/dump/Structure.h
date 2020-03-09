#ifndef DUMP_STRUCTURE_H
#define DUMP_STRUCTURE_H

// don't change value!!
enum UnitedMarketDataType {
	CTP_v638=101
};

enum ProductClassType {
	PC_Futures='1',
	PC_Options='2',
	PC_Combination='3',
	PC_Spot='4',
	PC_EFP='5',
	PC_SpotOption='6'
};

enum OptionsType {
	OT_Call='1',
	OT_Put='2'
};

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
};

struct MarketDataHead {
	long local_time = 0;
};


#endif