#!/usr/bin/env python
import libstrategy as at
import libexecstg as ex
import sys
import datetime

class CDemoSingle(ex.ExecStrategyBase):
    def __init__(self, conf):
        super(CDemoSingle, self).__init__()
        self.cnt = 2
        if not self.init(conf):
            raise Exception("init failed")
        #single_demo.setReadPos(0)
        #self.lid = at.getNewLogger("fasttest")
        #at.fastLog(self.lid, "fastlog test")
        
    def on_tick(self, tick):
        nano = at.getCurMdTime()
        s = "md: %s %s %s %s" % (tick.instr_str, tick.exch_time, datetime.datetime.fromtimestamp(tick.exch_time/1000000000), datetime.datetime.fromtimestamp(nano/1000000000))
        print s
        #at.fastLog(self.lid, s)
        if self.cnt > 0:
            self.cnt -= 1
            print self.sendOrder("rb1810", 0, 1, at.BUY, at.OPEN, 0)
    
    def on_rtn(self, rtn):
        print "rtn:"#, rtn.msg.decode("gbk")
    
    def on_time(self, nano): pass
        #print "on_time:", nano

    def on_bar(self, bar):
        print "on_bar:", bar.instr_str, bar.open
    
    def on_switch_day(self, day):
        print "on_switch_day:", day
        self.subscribe("rb1810")
        #self.subsBar("IC", 5)
        

class CDemo(at.StrategyBase):
    def __init__(self):
        super(CDemo, self).__init__()
        self.cnt = 2
    
    def on_tick(self, tick):
        print tick.instr_str, tick.exch_time
        if self.cnt > 0:
            self.cnt -= 1
            self.sendOrder("rb1810", 0, 1, at.BUY, at.OPEN, 0)
    
    def on_rtn(self, rtn):
        print "rtn:"#, rtn.msg.decode("gbk")
   
    def on_bar(self, bar):
        print "on_bar:", bar.instr_str, bar.open
    
    def on_switch_day(self, day):
        print "on_switch_day:", day
        self.subscribe("rb1810")
        self.subsBar("rb1810", 5)
    
    
def runSingle():
    single_demo = CDemoSingle(sys.argv[1])
    single_demo.run()
    single_demo.release()
    
def runProcess():
    def _run():
        demo = CDemo()
        if not at.StrategyProcess.init(sys.argv[1]):
            print "init failed"
            return
        if demo.activate(0) < 0: 
            print "activate demo failed"
            return
        at.StrategyProcess.run()
    
    _run()
    at.StrategyProcess.release()


if __name__ == "__main__":
    runSingle()
    
