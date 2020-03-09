#!/usr/bin/env python
# coding : utf-8
import libstrategy as st
import sys
import time
import json
import threading
import signal

keymap = \
{
    "0" : ["order", "rb1810", "market", "market", st.BUY, st.OPEN, 0],  # order instr price vol dir offset account_id
    "1" : ["cancel"],
    "2" : ["listOrder"]
}
    
engine_config = \
{
    "name": "GoldFinger",
    "MDHelper":
    {
        "type":"comm",
        "md_engine_name":"engine0",
        "timeout":2
    },
    "TDHelper":
    {
        "type":"comm",
        "td_engine_name":"td_engine",
        "timeout":2
    },
    "AccountDefault":
	{
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
			},
			"auto_offset_open_long_volume":
			{
				"default":1000,
				"rb1810":100
			},
			"auto_offset_open_short_volume":
			{
				"default":1000,
				"rb1810":10
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
	}
}


class GoldFinger(st.SimpleStrategy):
    def __init__(self):
        super(GoldFinger, self).__init__()
    
    def on_rtn(self, rtn):
        track = self.getOrderTrack(rtn.local_id)
        print "get rtn: %s, msg:%s" % (self.reprOrderTrack(track), rtn.msg.decode("gbk"))
    
    def on_time(self, nano):
        time.sleep(0.1)
        
    def order(self, instr, price, vol, dir, off, acc, *args):
        if price == "market":
            if dir == st.BUY:
                price = self.getLastTick(instr).ask
            elif dir == st.SELL:
                price = self.getLastTick(instr).bid
        if vol == "market":
            if dir == st.BUY:
                vol = self.getLastTick(instr).ask_vol
            elif dir == st.SELL:
                vol = self.getLastTick(instr).bid_vol
        id = self.sendOrder(instr, price, vol, dir, off, acc)
        print "order %s: [%s %s %s %s %s %s]" % (id, instr, price, vol, dir, off, acc)
        return id
    
    def on_tick(self, tick):
        pass #print "tick: %s bid:%s %s|ask:%s %s" % (tick.instr_str, tick.bid_px, tick.bid_vol, tick.ask_px, tick.ask_vol)

    def cancel(self, *args):
        res = []
        for i in args: 
            res.append(self.cancelOrder(int(i)))
        return res
    
    def listOrder(self, *args):
        for i in xrange(self.getOrderTrackCnt()):
            print "%s:%s" % (i, self.reprOrderTrack(self.getOrderTrack(i)))
    
    def reprOrderTrack(self, ot):
        return "{ref:%s, status:%s, instr:%s, price:%s, vol:%s, dir:%s, off:%s, traded:%s, accid:%s}" \
          % (ot.order_ref, ot.status, ot.instr_str, ot.price, ot.vol, st.Dir.values[ot.dir], st.Offset.values[ot.off], ot.vol_traded, ot.acc_id)


def run():
    finger = GoldFinger()
    if not finger.initStr(json.dumps(engine_config)):
        print "init failed"
        return

    subs = set()
    for k, v in keymap.iteritems():
        if v[0] == "order":
            subs.add(v[1])
    
    if not finger.subscribe(','.join(subs)):
        print "subscribe instruments %s failed." % (','.join(subs))
        return
    
    t = threading.Thread(target=finger.run)
    t.setDaemon(True)
    t.start()
    
    def _raise_int(signum, frame):
        raise KeyboardInterrupt();
    
    signal.signal(signal.SIGTERM, _raise_int)
    signal.signal(signal.SIGINT, _raise_int)
    signal.signal(signal.SIGHUP, _raise_int)
    signal.signal(signal.SIGQUIT, _raise_int)
    
    try:
        while True:
            x = raw_input()
            x = x.split()
            if len(x) == 0: continue
            elif x[0] == "quit":
                break
            elif x[0] in keymap:
                v = keymap[x[0]]
                f = getattr(finger, v[0], None)
                if f:
                    args = v[1:]
                    args.extend(x[1:])
                    res = f(*args)
                    if res: print "res:", res
    except KeyboardInterrupt:
        pass
            
    finger.stop()
    t.join()
    finger.release()
    

if __name__ == "__main__":
    run()

