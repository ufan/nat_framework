#include "RiskInstrTop.h"

#include "ATConstants.h"
#include "utils.h"

void RiskInstrTop::onNew(int dir, double px, int order_ref, long nano) {
  vec_treap_node.emplace_back(order_ref, px, dir);
  if (dir == AT_CHAR_Buy) {
    bid_treap.push_back(vec_treap_node[vec_treap_node.size() - 1]);
  } else {
    ask_treap.push_back(vec_treap_node[vec_treap_node.size() - 1]);
  }
  RiskInstr::onNew(nano);
}

void RiskInstrTop::onCxl(const tOrderTrack* p_ord_trk) {
  TreapNode node(p_ord_trk->order_ref, 0.0, AT_CHAR_Buy);
  if (p_ord_trk->dir == AT_CHAR_Buy) {
    bid_treap.erase(node);
  } else {
    ask_treap.erase(node);
  }
}

void RiskInstrTop::onTrd(const tOrderTrack* p_ord_trk) {
  TreapNode node(p_ord_trk->order_ref, 0.0, AT_CHAR_Buy);
  if (p_ord_trk->vol_traded == p_ord_trk->vol) {
    if (p_ord_trk->dir == AT_CHAR_Buy) {
      bid_treap.erase(node);
    } else {
      ask_treap.erase(node);
    }
  }
}

/**
 * @brief Add check for self-trade above conventional checks
 */
int RiskInstrTop::check(int dir, int off, double px, int vol, long nano) {
  // Prevent self-trading for the same trading account
  if (dir == AT_CHAR_Buy) {
    if (!ask_treap.empty()) {
      if (px >= ask_treap.top()->price) {
        return -230;
      }
    }
  } else {
    if (!bid_treap.empty()) {
      if (px <= bid_treap.top()->price) {
        return -231;
      }
    }
  }

  // Do conventional checks
  return RiskInstr::check(dir, off, vol, nano);
}

void RiskInstrTop::onSwitchDay() {
  bid_treap.clear();
  ask_treap.clear();
  vec_treap_node.clear();
}
