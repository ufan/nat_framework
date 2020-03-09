#ifndef SRC_RISK_RISKINSTRTOP_H
#define SRC_RISK_RISKINSTRTOP_H

#include "RiskInstr.h"
#include <boost/intrusive/treap.hpp>
#include <vector>
#include "ATConstants.h"
#include "ATStructure.h"

using namespace std;

struct TreapNode : public boost::intrusive::bs_set_base_hook<boost::intrusive::link_mode<boost::intrusive::normal_link>> 
{
	TreapNode(int ord, double px, int di) : order_ref(ord), price(px), dir(di) {}

	friend bool operator< (const TreapNode& a, const TreapNode& b) {
		return a.order_ref < b.order_ref;
	}
	
	friend bool priority_order(const TreapNode& a, const TreapNode& b) {
		if (a.dir == AT_CHAR_Buy) {
			return a.price > b.price;
		} else {
			return a.price < b.price;
		}
	}

	int order_ref = 0;
	double price = 0.0;
	int dir;
};

class RiskInstrTop : public RiskInstr
{
public:
	RiskInstrTop() {vec_treap_node.reserve(1024);}
	void onNew(int dir, double px, int order_ref, long nano);
	void onCxl(const tOrderTrack* p_ord_trk);
	void onTrd(const tOrderTrack* p_ord_trk);
	int check(int dir, int off, double px, int vol, long nano);
	void onSwitchDay();
	
protected:
	bool IsIntensityNormal(long exch_time);
	
	vector<TreapNode> vec_treap_node;
	boost::intrusive::treap<TreapNode> ask_treap;
	boost::intrusive::treap<TreapNode> bid_treap;
};

#endif