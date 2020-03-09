#!/usr/bin/env python
import libstrategy as stg
w=stg.WareHouseReader()
#w.loadFiles(["/mnt/warehouse/China/SHFE/rb/CTP_${DAY}_SHFE_rb.data", "/mnt/warehouse/China/SHFE/cu/CTP_${DAY}_SHFE_cu.data"], "20180601", "20180807")
w.loadFileList("/mnt/warehouse/China/SHFE/cu/CTP_${DAY}_SHFE_cu.data", "20180601", "20180807")

