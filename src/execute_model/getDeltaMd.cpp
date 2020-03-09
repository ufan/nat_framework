#include "stdio.h"
#include "string.h"

struct MarketData
{
	double AskPrice[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	double BidPrice[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int AskVolume[5] = {0, 0, 0, 0, 0};
	int BidVolume[5] = {0, 0, 0, 0, 0};
};

struct DeltaMarketData
{
	double AskPrice[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double BidPrice[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	int AskVolume[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int BidVolume[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

void getDeltaMarketData(MarketData& md1, MarketData& md2, DeltaMarketData& delta_md)
{
	int i = 0, j = 0, k = 0;
	while (i < 5 && md1.BidPrice[i] > md2.BidPrice[0])
	{		 
		++i;
	}
	while (j < 5 && md2.BidPrice[j] > md1.BidPrice[0])
	{
		++j;
	}
	while (i < 5 && j < 5)
	{
		delta_md.BidPrice[k] = md1.BidPrice[i] > md2.BidPrice[j] ? md1.BidPrice[i] : md2.BidPrice[j];
		
		if (delta_md.BidPrice[k] == md1.BidPrice[i])
		{
			delta_md.BidVolume[k] -= md1.BidVolume[i];
			++i;
		}
		if (delta_md.BidPrice[k] == md2.BidPrice[j])
		{
			delta_md.BidVolume[k] += md2.BidVolume[j];
			++j;
		}
		++k;
	}
	
	i = j = k = 0;
	while (i < 5 && md1.AskPrice[i] < md2.AskPrice[0])
	{		 
		++i;
	}
	while (j < 5 && md2.AskPrice[j] < md1.AskPrice[0])
	{
		++j;
	}
	while (i < 5 && j < 5)
	{
		delta_md.AskPrice[k] = md1.AskPrice[i] < md2.AskPrice[j] ? md1.AskPrice[i] : md2.AskPrice[j];
		
		if (delta_md.AskPrice[k] == md1.AskPrice[i])
		{
			delta_md.AskVolume[k] -= md1.AskVolume[i];
			++i;
		}
		if (delta_md.AskPrice[k] == md2.AskPrice[j])
		{
			delta_md.AskVolume[k] += md2.AskVolume[j];
			++j;
		}
		++k;
	}
}

void printDeltaMarketData(DeltaMarketData& delta_md)
{
	for (int i = 0; i < 10; ++i)
	{
		if (delta_md.AskPrice[9-i] != 0.0)
		{
			printf("%.2lf\t%d\n", delta_md.AskPrice[9-i], delta_md.AskVolume[9-i]);
		}
	}
	printf("\n");
	for (int i = 0; i < 10; ++i)
	{
		if (delta_md.BidPrice[i] != 0.0)
		{
			printf("%.2lf\t%d\n", delta_md.BidPrice[i], delta_md.BidVolume[i]);
		}
	}
	printf("\n");
}

int main(int argc, char* argv[])
{
	printf("hello\n");
	MarketData md1, md2;
	md1.AskPrice[0] = 11.0;
	md1.AskPrice[1] = 12.0;
	md1.AskPrice[2] = 13.0;
	md1.AskPrice[3] = 14.0;
	md1.AskPrice[4] = 15.0;
	md1.BidPrice[0] = 10.0;
	md1.BidPrice[1] = 9.0;
	md1.BidPrice[2] = 8.0;
	md1.BidPrice[3] = 7.0;
	md1.BidPrice[4] = 6.0;
	md1.AskVolume[0] = 11;
	md1.AskVolume[1] = 12;
	md1.AskVolume[2] = 13;
	md1.AskVolume[3] = 14;
	md1.AskVolume[4] = 15;
	md1.BidVolume[0] = 10;
	md1.BidVolume[1] = 9;
	md1.BidVolume[2] = 8;
	md1.BidVolume[3] = 7;
	md1.BidVolume[4] = 6;
	
	md2.AskPrice[0] = 12.0;
	md2.AskPrice[1] = 13.0;
	md2.AskPrice[2] = 14.0;
	md2.AskPrice[3] = 15.0;
	md2.AskPrice[4] = 16.0;
	md2.BidPrice[0] = 11.0;
	md2.BidPrice[1] = 10.0;
	md2.BidPrice[2] = 9.0;
	md2.BidPrice[3] = 8.0;
	md2.BidPrice[4] = 7.0;
	md2.AskVolume[0] = 12;
	md2.AskVolume[1] = 12;
	md2.AskVolume[2] = 13;
	md2.AskVolume[3] = 14;
	md2.AskVolume[4] = 15;
	md2.BidVolume[0] = 10;
	md2.BidVolume[1] = 9;
	md2.BidVolume[2] = 8;
	md2.BidVolume[3] = 7;
	md2.BidVolume[4] = 7;
	
	DeltaMarketData delta_md;
	memset(&delta_md, 0, sizeof(DeltaMarketData));
	getDeltaMarketData(md1, md2, delta_md);
	printDeltaMarketData(delta_md);
	
	getchar();
	return 0;
}