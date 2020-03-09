#ifndef SRC_RISK_RISKERROR_H_
#define SRC_RISK_RISKERROR_H_

class RiskError {
public:
	static const char* strerror(int errnum) {
		switch (errnum) {
			case 0:
				return "succ";
			case -1:
				return "stop trading flag is open or instrument not subscribed.";
			case -100:
				return "too many unfilled order";
			case -110:
				return "long amount exceeds";
			case -111:
				return "short amount exceeds";
			case -112:
				return "net amount exceeds";
			case -200:
				return "instr is not allowed";
			case -201:
				return "gap between order px and bid1/ask1 px is too big";
			case -202:
				return "order size is too big";
			case -203:
				return "abnormal intensity";
			case -210:
				return "instr long vol exceeds";
			case -211:
				return "instr short vol exceeds";
			case -212:
				return "instr net vol exceeds";
			case -230:
				return "may self trade";
			case -231:
				return "may self trade";
			case -270:
				return "can't parse auto offset";
			case -271:
				return "can't parse auto offset";
			case -272:
				return "can't parse dir";
			case -300:
				return "prd long vol exceeds";
			case -301:
				return "prd short vol exceeds";
			case -302:
				return "prd net vol exceeds";
			case -401:
				return "prd is not registered";
			case -403:
				return "instr is not registered in stg";
			case -404:
				return "instr is not registered in top";
			case -405:
				return "can't parse auto offset in top";
			default:
				return "unknown err number";
		}
	}
};

#endif
