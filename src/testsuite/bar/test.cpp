#include "stdio.h"
#include "string.h"
#include "BarMaker.h"
#include "ATStructure.h"

void inputTick(BarMaker* p_bar_maker, long exch_sec, double px, int cum_vol)
{
	UnitedMarketData* p_umd = new UnitedMarketData;
	Bar* p_bar = nullptr;
	p_umd->exch_time = exch_sec * 1000000000L;
	p_umd->last_px = px;
	p_umd->cum_vol = cum_vol;
	p_bar_maker->OnTick(p_umd);
	while ((p_bar = p_bar_maker->GetBar()) != nullptr)
	{
			printf("bar: %02ld:%02ld:%02ld--%02ld:%02ld:%02ld\n", 
				(8+p_bar->bob/1000000000L/3600)%24, p_bar->bob/1000000000L%3600/60, p_bar->bob/1000000000L%60, 
				(8+p_bar->eob/1000000000L/3600)%24, p_bar->eob/1000000000L%3600/60, p_bar->eob/1000000000L%60);
				
			printf("%.2lf, %.2lf, %.2lf, %.2lf; %.2lf, %d\n",
				p_bar->open, p_bar->high, p_bar->low, p_bar->close, p_bar->delta_close, p_bar->vol);
	}
}

int main(int argc, char* argv[])
{
	tInstrumentInfo* p_instr_info = new tInstrumentInfo;
	strcpy(p_instr_info->instr, "ZC805");
	strcpy(p_instr_info->product, "ZC");
	p_instr_info->exch = EXCHANGEID_CZCE;
	BarMaker* p_bar_maker = new BarMaker(p_instr_info, 2);
	
	// yd
	inputTick(p_bar_maker, 1520254540, 624.2, 0);
	
	// auction
	inputTick(p_bar_maker, 1520254740, 624.8, 582);
	
	// continuous
	inputTick(p_bar_maker, 1520254800, 625.2, 784);
	inputTick(p_bar_maker, 1520254801, 625.4, 1086);
	inputTick(p_bar_maker, 1520254801, 625.0, 1270);
	
	inputTick(p_bar_maker, 1520254802, 625.2, 1304);
	inputTick(p_bar_maker, 1520254802, 625.2, 1346);
	inputTick(p_bar_maker, 1520254803, 625.2, 1364);
	inputTick(p_bar_maker, 1520254803, 625.0, 1394);
	
	inputTick(p_bar_maker, 1520254804, 625.2, 1396);
	inputTick(p_bar_maker, 1520254804, 625.2, 1414);
	inputTick(p_bar_maker, 1520254805, 625.2, 1458);
	inputTick(p_bar_maker, 1520254805, 625.0, 1472);
	inputTick(p_bar_maker, 1520254805, 625.0, 1504);
	
	inputTick(p_bar_maker, 1520254806, 625.2, 1554);
	inputTick(p_bar_maker, 1520254806, 625.2, 1596);
	inputTick(p_bar_maker, 1520254807, 625.2, 1614);
	
	inputTick(p_bar_maker, 1520254808, 625.0, 1626);
	inputTick(p_bar_maker, 1520254808, 625.0, 1692);
	inputTick(p_bar_maker, 1520254808, 625.2, 1724);
	inputTick(p_bar_maker, 1520254809, 625.0, 1746);
	
	inputTick(p_bar_maker, 1520254810, 625.0, 1754);
	inputTick(p_bar_maker, 1520254810, 625.0, 1798);
	
	getchar();
	return 0;
}