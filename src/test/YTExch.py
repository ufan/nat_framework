#!/usr/bin/env python
#-*- coding:utf8 -*-
import sys
import time
import json
import math
import libstrategy as st

NOT_SET                 = 0
CLOSED                  = 1
SEND                    = (1 << 1)
TDSEND                  = (1 << 2)
CXLING                  = (1 << 3)
ACCEPT                  = (1 << 4)
REJECT                  = (1 << 5) | 1
MARKET_ACCEPT           = (1 << 6)
MARKET_REJECT           = (1 << 7) | 1
EXECUTION               = (1 << 8)
CANCEL_REJECT           = (1 << 9)
CANCELED                = (1 << 10) | 1
ERR                     = (1 << 11) | 1

EXCHANGEID_SSE          = 100 #上海证券交易所
EXCHANGEID_SZE          = 101 #深圳证券交易所
EXCHANGEID_CFFEX        = 102 #中国金融期货交易所
EXCHANGEID_SHFE         = 103 #上海期货交易所
EXCHANGEID_DCE          = 104 #大连商品交易所
EXCHANGEID_CZCE         = 105 #郑州商品交易所
EXCHANGEID_INE          = 106 #能源中心
EXCHANGEID_SGE          = 107 #上海金交所

def getNano():
    return int(time.time() * 1000000000)

def output_channel1(rep):
    sys.stdout.write(json.dumps(rep) + '\n')
    sys.stdout.flush()

def output_channel2(rep):
    sys.stderr.write(json.dumps(rep) + '\n')
    sys.stderr.flush()

    
class TradeBaseInfo(object):
    def __init__(self):
        object.__init__(self)
    
    @staticmethod
    def getBaseInfo():
        info = {}
        info["trading_day"] = st.getTradingDay()
        info["instr_info"] = [
            {
                "instr" : i.instr,
                "exch": i.exch,
                "product": i.product,
                "vol_multiple": i.vol_multiple,
                "tick_price": i.tick_price,
                "expire_date": i.expire_date,
                "is_trading": i.is_trading
            }
            for i in st.getInstrumentInfo()
        ]
        return info
    
    @staticmethod
    def getInfo(instr):
        for i in st.getInstrumentInfo():
            if i.instr == instr:
                return {
                    "instr" : i.instr,
                    "exch": i.exch,
                    "product": i.product,
                    "vol_multiple": i.vol_multiple,
                    "tick_price": i.tick_price,
                    "expire_date": i.expire_date,
                    "is_trading": i.is_trading
                }
        return None
    
    
# return bidvol, askvol
def getRealTradeVol(tick, tick1):
    if tick is None:
        return tick1["bid_vol"], tick1["ask_vol"]
    turnover = tick1["cum_turnover"] - tick["cum_turnover"]
    vol = tick1["cum_vol"] - tick["cum_vol"]
    bt, bt1, at, at1 = tick["bid_px"], tick1["bid_px"], tick["ask_px"], tick1["ask_px"]
    avg = turnover / vol if vol > 0 else (bt1 + at1) / 2
    bvt, avt = tick["bid_vol"], tick["ask_vol"]

    def _solveLinear(vol, turnover, pl, pu):
        av = round((turnover - pl * vol) / (pu - pl))
        return vol - av, av

    if avg < min(bt, bt1): #1
        w = (avg - bt) / (avg - bt + avg - bt1)
        bv, av =  w * vol, (1 - w) * vol
    if avg > max(at, at1): #2
        w = (avg - at) / (avg - at + avg + at1)
        bv, av =  (1 - w) * vol, w * vol
    if bt <= bt1 < avg <= at1 <= at: #3
        bv, av =  _solveLinear(vol, turnover, bt1, at1)
    if bt <= avg <= bt1 and at1 <= at: #4
        bv, av = _solveLinear(vol, turnover, bt, at1)
    if bt <= bt1 and at1 <= avg <= at: #5
        bv, av = _solveLinear(vol, turnover, bt1, at)
    if bt <= bt1 <= avg < at <= at1: #6
        bv, av = _solveLinear(vol, turnover, bt1, at)
    if bt <= avg <= bt1 <= at <= at1: #7
        bv, av = _solveLinear(vol, turnover, bt, at)
    if bt <= bt1 <= at <= avg <= at1: #8
        bv, av = _solveLinear(vol, turnover, bt1, at1)
    if at <= bt1: #9
        bv, av = 0, vol
    if bt1 <= bt < avg <= at <= at1: #10
        bv, av = _solveLinear(vol, turnover, bt, at)
    if bt1 <= avg <= bt and at <= at1: #11
        bv, av = _solveLinear(vol, turnover, bt1, at)
    if bt1 <= bt and at <= avg <= at1: #12
        bv, av = _solveLinear(vol, turnover, bt, at1)
    if bt1 <= bt <= avg < at1 <= at: #13
        bv, av = _solveLinear(vol, turnover, bt, at1)
    if bt1 <= avg <= bt <= at1 <= at: #14
        bv, av = _solveLinear(vol, turnover, bt1, at1)
    if bt1 <= bt <= at1 <= avg <= at: #15
        bv, av = _solveLinear(vol, turnover, bt, at)
    if at1 <= bt: #16
        bv, av = vol, 0
    if vol == 0:
        bv, av = 0, 0
    if abs(avg - at) < 0.00001 and vol >= avt: #17
        bv, av = 0, vol
    if abs(avg - bt) < 0.00001 and vol >= bvt: #18
        bv, av = vol, 0
    if abs(avg - at) < 0.00001 and abs(avg - at1) < 0.00001: #19
        bv, av = 0, vol
    if abs(avg - bt) < 0.00001 and abs(avg - bt1) < 0.00001: #20
        bv, av = vol, 0
    return bv, av


class OrderMatch(object):
    def __init__(self, tick_price, multi):
        object.__init__(self)
        self.tick_price = tick_price
        self.multi = multi
        self.buys = []
        self.sells = []
        self.tick = None
    
    def addOrder(self, order_track):
        price = order_track["price"]
        if order_track["dir"] == ord('0'): #buy
            for i in xrange(len(self.buys)):
                if self.buys[i]["price"] >= price:
                    self.buys.insert(i, order_track)
                    break
            else:
                self.buys.append(order_track)
        else: # sell
            for i in xrange(len(self.sells)):
                if self.sells[i]["price"] <= price:
                    self.sells.insert(i, order_track)
                    break
            else:
                self.sells.append(order_track)
    
    def delOrder(self, ref):
        for i in xrange(len(self.buys)):
            if self.buys[i]["order_ref"] == ref:
                del self.buys[i]
                return
        for i in xrange(len(self.sells)):
            if self.sells[i]["order_ref"] == ref:
                del self.sells[i]
                return        
        
    def match(self, tick_org):
        tick = tick_org.copy()
        tick["cum_turnover"] = int(tick["cum_turnover"] / self.tick_price / self.multi)
        tick["last_px"] = int(tick["last_px"] / self.tick_price)
        tick["avg_px"] = int(tick["avg_px"] / self.tick_price)
        tick["ask_px"] = int(tick["ask_px"] / self.tick_price)
        tick["bid_px"] = int(tick["bid_px"] / self.tick_price)
        
        turnover = tick["cum_turnover"] - self.tick["cum_turnover"] if self.tick else 0
        vol = tick["cum_vol"] - self.tick["cum_vol"] if self.tick else 0
        avg = turnover / vol if vol > 0 else (tick["bid_px"] + tick["ask_px"]) / 2
        lstP = tick["last_px"]
        lowP = min(math.floor(avg + 0.00001), lstP)
        highP = max(math.floor(avg - 0.00001) + 1, lstP)
        real_bid_vol, real_ask_vol = getRealTradeVol(self.tick, tick)
        
        res = []
        # try buy side
        lst_level = tick["bid_px"] + 1
        while len(self.buys) > 0:
            ot = self.buys[-1]
            price = int(ot["price"] / self.tick_price)
            vol = ot["vol"] - ot["vol_traded"]
            if price >= tick["ask_px"]:
                ot["vol_traded"] += vol
                res.append((ot["order_ref"], vol))
                del self.buys[-1]
            elif price >= lowP and real_bid_vol > 0:
                real_bid_vol -= (lst_level - price) * tick["bid_vol"]
                lst_level = price
                if real_bid_vol <= 0: break
                traded_vol = min(real_bid_vol, vol)
                ot["vol_traded"] += traded_vol
                real_bid_vol -= traded_vol
                res.append((ot["order_ref"], traded_vol))
                if(ot["vol"] <= ot["vol_traded"]): del self.buys[-1]
                if(real_bid_vol <= 0): break
            else: break
        
        #try sell side
        lst_level = tick["ask_px"] - 1
        while len(self.sells) > 0:
            ot = self.sells[-1]
            price = int(ot["price"] / self.tick_price)
            vol = ot["vol"] - ot["vol_traded"]
            if price <= tick["bid_px"]:
                ot["vol_traded"] += vol
                res.append((ot["order_ref"], vol))
                del self.sells[-1]
            elif price <= highP and real_ask_vol > 0:
                real_ask_vol -= (price - lst_level) * tick["ask_vol"]
                lst_level = price
                if real_ask_vol <= 0: break
                traded_vol = min(real_ask_vol, vol)
                ot["vol_traded"] += traded_vol
                real_ask_vol -= traded_vol
                res.append((ot["order_ref"], traded_vol))
                if(ot["vol"] <= ot["vol_traded"]): del self.sells[-1]
                if(real_ask_vol <= 0): break
            else: break
        
        self.tick = tick
        return res
        

class TDExch(object):
    def __init__(self):
        object.__init__(self)
        self.order_track = []
        self.rtn_msg = []
        self.omm = {}
        
    def sendOrder(self, order):
        ot = {
            "status" : TDSEND | ACCEPT | MARKET_ACCEPT,
            "instr" : order["instr"],
            "price" : order["price"],
            "vol" : order["vol"],
            "dir" : order["dir"],
            "off" : order["off"],
            "vol_traded" : 0,
            "amount_traded" : 0,
            "from" : order["from"],
            "local_id" : order["local_id"],
            "acc_id" : order["acc_idx"],
            "stg_id" : order["stg_id"],
            "order_ref" : len(self.order_track),
            "front_id" : 0,
            "session_id" : 0,
        }
        self.order_track.append(ot)
        if order["instr"] not in self.omm:
            info = TradeBaseInfo.getInfo(order["instr"])
            self.omm[order["instr"]] = OrderMatch(info["tick_price"], info["vol_multiple"])
        self.omm[order["instr"]].addOrder(ot)
        
        msg = {
            "msg_type" : TDSEND,
            "local_id" : ot["local_id"],
            "instr_str" : ot["instr"],
            "price" : ot["price"],
            "vol" : ot["vol"],
            "dir" : ot["dir"],
            "off" : ot["off"],
            "order_ref" : ot["order_ref"],
            "front_id" : ot["front_id"],
            "session_id" : ot["session_id"],
            "errid" : 0,
            "msg" : "succ",
        }
        self.rtn_msg.append(msg.copy())
        
        msg["msg_type"] = ACCEPT
        self.rtn_msg.append(msg.copy())
        
        msg["msg_type"] = MARKET_ACCEPT
        self.rtn_msg.append(msg.copy())
    
    def delOrder(self, order):
        ot = self.order_track[order["order_ref"]]
        msg = {
            "msg_type" : CANCEL_REJECT,
            "local_id" : ot["local_id"],
            "instr_str" : ot["instr"],
            "price" : ot["price"],
            "vol" : 0,
            "dir" : ot["dir"],
            "off" : ot["off"],
            "order_ref" : ot["order_ref"],
            "front_id" : ot["front_id"],
            "session_id" : ot["session_id"],
            "errid" : -1,
            "msg" : "all traded",
        }
        if (ot["status"] & CLOSED) == 0 and (ot["vol"] > ot["vol_traded"]):
            msg["msg_type"] = CANCELED
            msg["vol"] = ot["vol"] - ot["vol_traded"]
            msg["errid"] = 0
            msg["msg"] = "succ"
            ot["status"] |= CANCELED
        self.rtn_msg.append(msg)
        self.omm[ot["instr"]].delOrder(ot["order_ref"])
        
    def getOrderTrack(self, sid):
        res = []
        for i in self.order_track:
            if i["from"] == sid:
                res.append(i)
        return res
    
    def match(self, md):
        if md["instr_str"] not in self.omm: return
        om = self.omm[md["instr_str"]]
        res = om.match(md)
        for ref, traded in res:
            ot = self.order_track[ref]
            msg = {
                "msg_type" : EXECUTION,
                "local_id" : ot["local_id"],
                "instr_str" : ot["instr"],
                "price" : ot["price"],
                "vol" : traded,
                "dir" : ot["dir"],
                "off" : ot["off"],
                "order_ref" : ot["order_ref"],
                "front_id" : ot["front_id"],
                "session_id" : ot["session_id"],
                "errid" : 0,
                "msg" : "succ",
            }
            self.rtn_msg.append(msg)
            ot["amount_traded"] += traded * ot["price"]
            ot["status"] |= EXECUTION
            if ot["vol_traded"] >= ot["vol"]: ot["status"] |= CLOSED
    
    def getRtn(self):
        if len(self.rtn_msg) > 0:
            res = self.rtn_msg[0]
            del self.rtn_msg[0]
            return res
        return None
    
    def onTick(self, md):
        self.match(md)
        rtn = self.getRtn()
        i = 0
        while rtn is not None:
            output_channel2(rtn)
            i += 1
            if i >= 10: break
            rtn = self.getRtn()
    
            
class MDExch(object):
    def __init__(self, files, start, end):
        object.__init__(self)
        self.subset = set()
        self.warehouse = st.WareHouseReader()
        self.warehouse.doSetTimer(True)
        self.warehouse.loadFiles(files, start, end)
        self.trading_day = st.getTradingDay()
        self.lst_tick = None
        
    def genMd(self):
        if self.lst_tick is not None:
            tick = self.lst_tick
            self.lst_tick = None
            return "md", tick
        else:
            lst_tick = self.warehouse.readOne()
            while lst_tick and lst_tick["instr_str"] not in self.subset:
                lst_tick = self.warehouse.readOne()
            if not lst_tick: return "done", None
            if st.getTradingDay() != self.trading_day:
                self.trading_day = st.getTradingDay()
                self.lst_tick = lst_tick
                return "updateBaseInfo", TradeBaseInfo.getBaseInfo()
            else:
                return "md", lst_tick 
        
    def subs(self, instr):
        for i in instr:
            if TradeBaseInfo.getInfo(i) is not None:
                self.subset.add(i)
            else:
                return "FAILED"
        return "OK"
    
    def unsubs(self, instr):
        self.subset -= set(instr)
        return "OK"
    
    def setReadPos(self, nano):
        self.warehouse.setReadPos(nano)
    
    def getSubsInstrument(self):
        return list(self.subset)
    
    def onSwitchDay(self):
        self.subset.clear()
    

class Exchange(object):
    def __init__(self, start, end, files):
        object.__init__(self)
        self.md = MDExch(files, start, end)
        self.td = TDExch()
        
    def parseCmd(self, cmd):
        if cmd["cmd"] == "queryBaseInfo":
            rep = TradeBaseInfo.getBaseInfo()
            rep["ret"] = "OK"
            output_channel1(rep)
        elif cmd["cmd"] == "read":
            tp, data = self.md.genMd()
            if data:
                rep = {"ret":"OK", "type":tp, "data":data}
                output_channel1(rep)
                if tp == "md":
                    self.td.onTick(data)
            else:
                rep = {"ret":"FINISH"}
                output_channel1(rep)
        elif cmd["cmd"] == "getEngineSubscribedInstrument":
            rep = {"ret":"OK", "instr":self.md.getSubsInstrument()}
            output_channel1(rep)
        elif cmd["cmd"] == "subscribe":
            rep = {"ret":self.md.subs(cmd["instr"])}
            output_channel1(rep)
        elif cmd["cmd"] == "unsubscribe":
            rep = {"ret":self.md.unsubs(cmd["instr"])}
            output_channel1(rep)
        elif cmd["cmd"] == "qryOrderTrack":
            rep = {"ret":"OK", "order_track":self.td.getOrderTrack(cmd["id"])}
            output_channel1(rep)
        elif cmd["cmd"] == "sendOrder":
            self.td.sendOrder(cmd)
        elif cmd["cmd"] == "delOrder":
            self.td.delOrder(cmd)
        elif cmd["cmd"] == "setReadPos":
            self.md.setReadPos(cmd["nano"])

    def run(self):
        while True:
            l = raw_input()
            if not l: continue
            cmd = json.loads(l)
            self.parseCmd(cmd)


def test():
    r = {"cmd":"read"}
    print json.dumps(r)
    
    r = {"cmd":"subscribe", "instr":["rb1901"]}
    print json.dumps(r)
    
    r = {"cmd":"queryBaseInfo"}
    print json.dumps(r)
    
    r = {"cmd":"qryOrderTrack", "id":68}
    print json.dumps(r)
    
    r = {"cmd":"sendOrder", "from":68, "local_id":1, "acc_idx":0, "instr_hash":123, "instr":"rb1810", \
         "price":1800.0, "vol":2, "dir":ord('0'), "off":ord('0'), "stg_id":0}
    print json.dumps(r)
    
    r = {"cmd":"delOrder", "order_ref":0}
    print json.dumps(r)
    

if __name__ == "__main__":
    #try:
    exch = Exchange(sys.argv[1], sys.argv[2], sys.argv[3:])
    exch.run()
    #except:
    #    sys.exit(-1)

