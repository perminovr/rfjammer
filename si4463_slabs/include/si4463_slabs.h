/*
 * si4463.h
 *
 *  Created on: 30 ���. 2017 �.
 *      Author: MINI
 */

#ifndef INC_SI4463_H_
#define INC_SI4463_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <functional>

/* Types section */
typedef enum
{
	cmdErrorNone = 0,
	cmdErrorBadCommand = 16,
	cmdErrorBadArg = 17,
	cmdErrorErrorCommandBusy = 18,
	cmdErrorBadBootMode = 49,
	cmdErrorBadProperty = 64
} si4463_cmd_err_status_t;

typedef enum
{
	noState,
	sleepState,
	spiActiveState,
	readyState,
	ready2State,
	txTuneState,
	rxTuneState,
	txState,
	rxState
} si4463_state_t;

#pragma pack (push, 1)

struct si4463_interrupts_t
{
	/* PH interrupts */
	bool filterMatch;
	bool filterMiss;
	bool packetSent;
	bool packetRx;
	bool crcError;
	bool txFifoAlmostEmpty;
	bool rxFifoAlmostFull;
	/* Modem interrupts */
	bool postambleDetect;
	bool invalidSync;
	bool rssiJump;
	bool rssi;
	bool invalidPreamble;
	bool preambleDetect;
	bool syncDetect;
	/* Chip interrupts */
	bool cal;
	bool fifoUnderflowOverflowError;
	bool stateChange;
	bool cmdError;
	bool chipReady;
	bool lowBatt;
	bool wut;
};

#pragma pack (pop)

struct si4463_chip_status_t
{
	si4463_cmd_err_status_t cmdError;
	uint8_t cmdErrCmdId;
};

struct si4463_t
{
	std::function <void(const uint8_t * pTxData, uint8_t * pRxData, const uint16_t txSize)> WriteRead = nullptr;
	std::function <void(void)> SetShutdown = nullptr;
	std::function <void(void)> ClearShutdown = nullptr;
	std::function <void(void)> Select = nullptr;
	std::function <void(void)> Deselect = nullptr;
	std::function <void(uint32_t delayMs)> DelayMs = nullptr;
	std::function <uint8_t(void)> IsClearToSend = nullptr;
	// void (*WriteRead)(const uint8_t * pTxData, uint8_t * pRxData, const uint16_t txSize);
	// void (*SetShutdown)(void);
	// void (*ClearShutdown)(void);
	// void (*Select)(void);
	// void (*Deselect)(void);
	// void (*DelayMs)(uint32_t delayMs);
	// uint8_t (*IsClearToSend)(void);
	si4463_interrupts_t interrupts;
	si4463_chip_status_t chipStatus;
} ;

/* End types section */

/* Prototypes section */

extern uint8_t SI4463_WaitCTS(const si4463_t * si4463, uint8_t times, const uint8_t delayPerTime);
extern void SI4463_Reset(const si4463_t * si4463);
extern int8_t SI4463_SendCommand(const si4463_t * si4463, const uint8_t * cmdChain, const uint16_t cmdLen);
extern int8_t SI4463_ReadCommandBuffer(const si4463_t * si4463, uint8_t * cmdBuf, const uint8_t cmdBufLen);
extern void SI4463_SendNop(const si4463_t * si4463);
extern uint8_t SI4463_GetSwCts(const si4463_t * si4463);
extern int8_t SI4463_WaitSwCTS(const si4463_t * si4463, uint8_t times, const uint8_t delayPerTime);
extern int8_t SI4463_Init(const si4463_t * si4463);
extern int8_t SI4463_VerifyInit(const si4463_t * si4463);
extern int8_t SI4463_PowerUp(const si4463_t * si4463);
extern int8_t SI4463_GetPartInfo(const si4463_t * si4463, uint8_t * pRxData);
extern int8_t SI4463_GetChipStatus(si4463_t * si4463);
extern int8_t SI4463_ClearChipStatus(const si4463_t * si4463);
extern int8_t SI4463_GetInterrupts(si4463_t * si4463);
extern int8_t SI4463_ClearInterrupts(const si4463_t * si4463);
extern int8_t SI4463_ClearAllInterrupts(const si4463_t * si4463);
extern si4463_state_t SI4463_GetCurrentState(const si4463_t * si4463);
extern int8_t SI4463_SetCurrentState(const si4463_t * si4463, const si4463_state_t state);
extern int8_t SI4463_WriteTxFifo(const si4463_t * si4463, const uint8_t * msg, const uint16_t msgLen);
extern int8_t SI4463_ReadRxFifo(const si4463_t * si4463, uint8_t * msg, const uint16_t msgLen);
extern uint8_t SI4463_GetTxFifoRemainBytes(const si4463_t * si4463);
extern uint8_t SI4463_GetRxFifoReceivedBytes(const si4463_t * si4463);
extern int8_t SI4463_ClearRxFifo(const si4463_t * si4463);
extern int8_t SI4463_ClearTxFifo(const si4463_t * si4463);
extern int8_t SI4463_ClearSharedFifo(const si4463_t * si4463);
extern int8_t SI4463_Transmit(const si4463_t * si4463, const uint8_t * packet, const uint8_t len);
extern int8_t SI4463_StartTx(const si4463_t * si4463, const uint16_t len, const bool goToRxAfterTx);
extern int8_t SI4463_StartRx(const si4463_t * si4463, const uint16_t len, const bool goToRxAfterTimeout, const bool goToRxAfterValid, const bool goToRxAfterInvalid);
extern int8_t SI4463_GetProperty(const si4463_t * si4463, const uint8_t group, const uint8_t numProps, const uint8_t startProp, uint8_t * data);
extern int8_t SI4463_SetProperty(const si4463_t * si4463, const uint8_t group, const uint8_t numProps, const uint8_t startProp, const uint8_t * data);
extern int8_t SI4463_SetSplitFifo(const si4463_t * si4463);
extern int8_t SI4463_SetHalfDuplexFifo(const si4463_t * si4463);

/* Utils */
extern uint32_t SI4463_GetBytePosition(uint8_t neededByte, uint8_t * array, uint32_t arrayLen);

/* End of prototypes section */

/* Config */


#endif /* INC_SI4463_H_ */
