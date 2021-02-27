#ifndef SERVSTRUCTS_H
#define SERVSTRUCTS_H

#include <cstdint>

namespace ServStruct {

#pragma pack (push,1)

struct RadioParams {
	union {
		struct {
			uint16_t timeoutAck;
			uint16_t timeoutRepeat;
			uint16_t slavePause;
			uint16_t pollPause;
		};
		uint8_t _raw[8];
	};
	//
	static constexpr int size = sizeof(_raw);
	RadioParams() { timeoutAck = timeoutRepeat = slavePause = pollPause = 0; }
	RadioParams(const RadioParams &prm) { *this = prm; }
	~RadioParams() = default;
	void operator=(const RadioParams &prm) {
		timeoutAck = prm.timeoutAck;
		timeoutRepeat = prm.timeoutRepeat;
		slavePause = prm.slavePause;
		pollPause = prm.pollPause;
	}
	bool operator==(const RadioParams &prm) {
		return  timeoutAck == prm.timeoutAck &&
				timeoutRepeat == prm.timeoutRepeat &&
				slavePause == prm.slavePause &&
				pollPause == prm.pollPause;
	}
};

#pragma pack (pop)

};

#endif /* SERVSTRUCTS_H */
