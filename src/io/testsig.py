#!/usr/bin/env python
#-*- coding:utf8 -*-

import libstrategy as st
import time

def test_write():
	w = st.SignalWriter("signal1")
	s = "*" * 8000
	while True:
		w.write(s)
		time.sleep(0.02)


if __name__ == "__main__":
	test_write()


