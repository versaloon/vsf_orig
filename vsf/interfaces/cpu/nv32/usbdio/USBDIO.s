		SECTION .text:CODE:NOROOT:REORDER(2)

// introduced from configuration
#define IFS_USBDIO_DBG_PORT			0
#define IFS_USBDIO_DBG_PIN			0
#define IFS_USBDIO_PORT				1
#define IFS_USBDIO_DP				0
#define IFS_USBDIO_DM				1

// define CLK_48M for 48MHz clock, otherwise 24MHz clock
// if CLK_48M is defined, CRC16 check will be enabled
#define CLK_48M

#define DPDM_MASK					((1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM))

#define DEBUG

#define DEBUG_REG					0xF800000C
#define DEBUG_VALUE					0x01
#define CRC_BASE					0x40032000
#if IFS_USBDIO_PORT == 0
#	define GPIO_IN_REG				0xF8000010
#	define GPIO_TOGGLE_REG			0xF800000C
#	define GPIO_SET_REG				0xF8000004
#	define GPIO_CLEAR_REG			0xF8000008
#elif IFS_USBDIO_PORT == 1
#	define GPIO_IN_REG				0xF8000050
#	define GPIO_TOGGLE_REG			0xF800004C
#	define GPIO_SET_REG				0xF8000044
#	define GPIO_CLEAR_REG			0xF8000048
#endif
#define IRQSC_REG					0x40031000

		EXTERN nv32_usbdio_rx
		PUBLIC USBDIO_DP_Handler, USBDIO_Tx

		DATA

		THUMB
// DP fall edge interrupt handler
// [cycles after previous sample before current instruction : cycles of current instruction]
// code below is for CortexM0+ with fast io running at 24MHz
// because some delay in interrupt, bit0 will be skipped in the first run
// "increase buffer pointer" MUST be executed in the first run
// 3 cycles in bit0: save data
// in bit1 - bit4: SE0 check
// in bit4(48M): crc16
// 3 cycles in bit5: increase buffer pointer
// 4 cycles in bit6: buffer boundary check
// 3 cycles in bit7: calculate data
USBDIO_DP_Handler
		PUSH	{R4-R7, LR}
		// R1: uint8_t *buff;
		SUB		SP, SP, #0x14
		MOV		R1, SP
		SUBS	R1, R1, #1
		// R2, R3: uint32_t sample0 = (1 << IFS_USBDIO_DM), sample1;
		MOVS	R2, #1
		LSLS	R2, R2, #IFS_USBDIO_DM
#ifdef DEBUG
		// R4: uint32_t *debug_io;
		LDR		R4, =DEBUG_REG
#else
		// R4: uint32_t *input;
		LDR		R4, =GPIO_IN_REG
#endif
		// R5: uint32_t mask = (1 << IFS_USBDIO_DP) | (1 << IFS_USBDIO_DM);
		LDR		R5, =DPDM_MASK
		// R6: uint8_t byte = 0xFF;
		MOVS	R6, #0xFF
		// R7: uint8_t stuff = 0xFF;
		MOVS	R7, #0xFF

		// TODO: adjust number of NOP here, to center the sample point
#ifdef CLK_48M
		// CRC init
		LDR		R0, =CRC_BASE
		LDR		R3, =0x06000000
		STR		R3, [R0, #8]
		LDR		R3, =0x0000FFFF
		STRH	R3, [R0, #4]
		LDR		R3, =0x04000000
		STR		R3, [R0, #8]
		LDRB	R0, [R1]

		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

		// not fast enough to sample bit0 and bit1, start from bit2
		// bit0 and bit1 is the first part of SYNC, which is 'KJ' state
		B		SAMPLE_BIT2

SAMPLE_BIT0
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]						// [0 : 1]
#else
		// sample0 = *input & mask;
		LDR		R2, [R4]						// [0 : 1]
		// remove SE0 check to get 3 more cycles for saving data
//		ANDS	R2, R2, R5						// [1 : 1]
		// if (!sample0) break;
//		BNE		BIT0_OK							// [2 : 2]
//		B		RX_SE0
#endif
		// 3 cycles to save data and init stuff
		// *buff = byte;
		STRB	R7, [R1]						// [1 : 2]
		// stuff = 0xFF;
		MOVS	R7, #0xFF						// [3 : 1]
BIT0_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP										// [4 : 1]
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R2, R2, R5						// [0 : 1]
		// if (!sample0) break;
		BNE		BIT0_OK_48						// [1 : 1/2]
		B		RX_SE0
BIT0_OK_48
		NOP										// [3 : 1]
		LDRB	R0, [R1]						// [4 : 2]
		LDRB	R0, [R1]						// [6 : 2]
		LDRB	R0, [R1]						// [8 : 2]
		LDRB	R0, [R1]						// [10: 2]
		LDRB	R0, [R1]						// [12: 2]
		LDRB	R0, [R1]						// [14: 2]
#endif
#ifndef DEBUG
		MOVS	R0, #0x01						// [5 : 1]
#endif
		// sample1 ^= sample0;
		EORS	R3, R3, R2						// [6 : 1]
		// if (sample1 & (1 << IFS_USBDIO_DP))
		LSLS	R3, R3, #(31 - IFS_USBDIO_DP)	// [7 : 1]
		BPL		BIT0_SAMPLE_0					// [8 : 1/2]
		// sample 1: byte |= 0x01;
		ORRS	R6, R6, R0						// [9 : 1]
		B		BIT0_BITSTUFFING				// [10 : 2]
BIT0_SAMPLE_0
		// sample 0: byte &= ~0x01;
		BICS	R6, R6, R0						// [10 : 1]
		NOP		// for balance if delay			// [11 : 1]
BIT0_BITSTUFFING
		// if (!(byte & 0xF9))
		MOVS	R0, #0xF9						// [12 : 1]
		TST		R6, R0							// [13 : 1]
		BNE		SAMPLE_BIT1						// [14 : 1/2]
		NOP		// for balance the BNE			// [15 : 1]
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]						// [0 : 1]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R2, R2, R5						// [1 : 1]
		ANDS	R0, R0, R5						// [2 : 1]
		CMP		R0, R2							// [3 : 1]
		BNE		BIT0_STUFF_OK					// [4 : 1/2]
		B		EXIT_RX
BIT0_STUFF_OK
		// sample0 = tmp
		MOV		R2, R0							// [6 : 1]
		MOVS	R0, #0x01						// [7 : 1]
#endif
		// stuff &= ~0x01;
		BICS	R7, R7, R0						// [8 : 1]
		// byte |= 0x01;
		ORRS	R6, R6, R0						// [9 : 1]
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]						// [10: 2]
		LDRB	R0, [R1]						// [12: 2]
		LDRB	R0, [R1]						// [14: 2]
#ifdef CLK_48M
		LDRB	R0, [R1]						// [0 : 2]
		LDRB	R0, [R1]						// [2 : 2]
		LDRB	R0, [R1]						// [4 : 2]
		LDRB	R0, [R1]						// [6 : 2]
		LDRB	R0, [R1]						// [8 : 2]
		LDRB	R0, [R1]						// [10: 2]
		LDRB	R0, [R1]						// [12: 2]
		LDRB	R0, [R1]						// [14: 2]
#endif

SAMPLE_BIT1
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		NOP
		NOP
		NOP
#else
		// sample1 = *input & mask;
		LDR		R3, [R4]
		ANDS	R3, R3, R5
		// if (!sample) break;
		BNE		BIT1_OK
		B		RX_SE0
#endif
BIT1_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R3, R3, R5
		// if (!sample0) break;
		BNE		BIT1_OK_48
		B		RX_SE0
BIT1_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x02
#endif
		EORS	R2, R2, R3
		// if (sample0 & (1 << IFS_USBDIO_DP))
		LSLS	R2, R2, #(31 - IFS_USBDIO_DP)
		BPL		BIT1_SAMPLE_0
		// sample 1: byte |= 0x02;
		ORRS	R6, R6, R0
		B		BIT1_BITSTUFFING
BIT1_SAMPLE_0
		// sample 0: byte &= ~0x02;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT1_BITSTUFFING
		// if (!(byte & 0xF3))
		MOVS	R0, #0xF3
		TST		R6, R0
		BNE		SAMPLE_BIT2
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R3, R3, R5
		ANDS	R0, R0, R5
		CMP		R0, R3
		BNE		BIT1_STUFF_OK
		B		EXIT_RX
BIT1_STUFF_OK
		// sample0 = tmp
		MOV		R3, R0
		MOVS	R0, #0x02
#endif
		// stuff &= ~0x02;
		BICS	R7, R7, R0
		// byte |= 0x02;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT2
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		NOP
		NOP
		NOP
#else
		// sample0 = *input & mask;
		LDR		R2, [R4]
		// remove SE0 check to get 3 more cycles for increasing buffer pointer
		ANDS	R2, R2, R5
		// if (!sample0) break;
		BNE		BIT2_OK
		B		RX_SE0
#endif
BIT2_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R2, R2, R5
		// if (!sample0) break;
		BNE		BIT2_OK_48
		B		RX_SE0
BIT2_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x04
#endif
		// sample1 ^= sample0
		EORS	R3, R3, R2
		// if (sample1 & (1 << IFS_USBDIO_DP))
		LSLS	R3, R3, #(31 - IFS_USBDIO_DP)
		BPL		BIT2_SAMPLE_0
		// sample 1: byte |= 0x04;
		ORRS	R6, R6, R0
		B		BIT2_BITSTUFFING
BIT2_SAMPLE_0
		// sample 0: byte &= ~0x04;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT2_BITSTUFFING
		// if (!(byte & 0xE7))
		MOVS	R0, #0xE7
		TST		R6, R0
		BNE		SAMPLE_BIT3
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R2, R2, R5
		ANDS	R0, R0, R5
		CMP		R0, R2
		BNE		BIT2_STUFF_OK
		B		EXIT_RX
BIT2_STUFF_OK
		// sample0 = tmp
		MOV		R2, R0
		MOVS	R0, #0x04
#endif
		// stuff &= ~0x04;
		BICS	R7, R7, R0
		// byte |= 0x04;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT3
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		NOP
		NOP
		NOP
#else
		// sample1 = *input & mask;
		LDR		R3, [R4]
		ANDS	R3, R3, R5
		// if (!sample) break;
		BNE		BIT3_OK
		B		RX_SE0
#endif
BIT3_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R3, R3, R5
		// if (!sample0) break;
		BNE		BIT3_OK_48
		B		RX_SE0
BIT3_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x08
#endif
		EORS	R2, R2, R3
		// if (sample0 & (1 << IFS_USBDIO_DP))
		LSLS	R2, R2, #(31 - IFS_USBDIO_DP)
		BPL		BIT3_SAMPLE_0
		// sample 1: byte |= 0x08;
		ORRS	R6, R6, R0
		B		BIT3_BITSTUFFING
BIT3_SAMPLE_0
		// sample 0: byte &= ~0x08;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT3_BITSTUFFING
		// if (!(byte & 0xCF))
		MOVS	R0, #0xCF
		TST		R6, R0
		BNE		SAMPLE_BIT4
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R3, R3, R5
		ANDS	R0, R0, R5
		CMP		R0, R3
		BNE		BIT3_STUFF_OK
		B		EXIT_RX
BIT3_STUFF_OK
		// sample0 = tmp
		MOV		R3, R0
		MOVS	R0, #0x08
#endif
		// stuff &= ~0x08;
		BICS	R7, R7, R0
		// byte |= 0x08;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT4
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		NOP
		NOP
		NOP
#else
		// sample0 = *input & mask;
		LDR		R2, [R4]
		ANDS	R2, R2, R5
		// if (!sample0) break;
		BNE		BIT4_OK
		B		RX_SE0
#endif
BIT4_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		MOV		R0, SP							// [0 : 1]
		SUBS	R0, R1, R0						// [1 : 1]
		CMP		R0, #0x5						// [2 : 1]
		BGE		CALC_CRC						// [3 : 1/2]
		LDRB	R0, [R1]						// [4 : 2]
		LDRB	R0, [R1]						// [6 : 2]
		LDRB	R0, [R1]						// [8 : 2]
		NOP										// [10: 1]
		B		CRC_DELAY						// [11: 2]
CALC_CRC
		LDRB	R0, [R1]						// [5 : 2]
		MOV		LR, R1							// [7 : 1]
		LDR		R1, =CRC_BASE					// [8 : 2]
		STRB	R0, [R1]						// [10: 2]
		MOV		R1, LR							// [12: 1]
CRC_DELAY
		LDRB	R0, [R1]						// [13: 2]
		NOP										// [15: 1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x10
#endif
		// sample1 ^= sample0
		EORS	R3, R3, R2
		// if (sample1 & (1 << IFS_USBDIO_DP))
		LSLS	R3, R3, #(31 - IFS_USBDIO_DP)
		BPL		BIT4_SAMPLE_0
		// sample 1: byte |= 0x10;
		ORRS	R6, R6, R0
		B		BIT4_BITSTUFFING
BIT4_SAMPLE_0
		// sample 0: byte &= ~0x10;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT4_BITSTUFFING
		// if (!(byte & 0x9F))
		MOVS	R0, #0x9F
		TST		R6, R0
		BNE		SAMPLE_BIT5
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R2, R2, R5
		ANDS	R0, R0, R5
		CMP		R0, R2
		BNE		BIT4_STUFF_OK
		B		EXIT_RX
BIT4_STUFF_OK
		// sample0 = tmp
		MOV		R2, R0
		MOVS	R0, #0x10
#endif
		// stuff &= ~0x10;
		BICS	R7, R7, R0
		// byte |= 0x10;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT5
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
#else
		// sample1 = *input & mask;
		LDR		R3, [R4]
//		ANDS	R3, R3, R5
		// if (!sample) break;
//		BNE		BIT5_OK
//		B		RX_SE0
#endif
		// 3 cycles to increase buffer pointer
		// buf++;
		ADDS	R1, R1, #1
		LDRB	R0, [R1]
BIT5_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R3, R3, R5
		// if (!sample0) break;
		BNE		BIT5_OK_48
		B		RX_SE0
BIT5_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x20
#endif
		EORS	R2, R2, R3
		// if (sample0 & (1 << IFS_USBDIO_DP))
		LSLS	R2, R2, #(31 - IFS_USBDIO_DP)
		BPL		BIT5_SAMPLE_0
		// sample 1: byte |= 0x20;
		ORRS	R6, R6, R0
		B		BIT5_BITSTUFFING
BIT5_SAMPLE_0
		// sample 0: byte &= ~0x20;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT5_BITSTUFFING
		// if (!(byte & 0x3F))
		MOVS	R0, #0x3F
		TST		R6, R0
		BNE		SAMPLE_BIT6
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R3, R3, R5
		ANDS	R0, R0, R5
		CMP		R0, R3
		BNE		BIT5_STUFF_OK
		B		EXIT_RX
BIT5_STUFF_OK
		// sample0 = tmp
		MOV		R3, R0
		MOVS	R0, #0x20
#endif
		// stuff &= ~0x20;
		BICS	R7, R7, R0
		// byte |= 0x20;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT6
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
#else
		// sample0 = *input & mask;
		LDR		R2, [R4]
		// remove SE0 check to get 3 more cycles for buffer boundary
//		ANDS	R2, R2, R5
		// if (!sample0) break;
//		BNE		BIT6_OK
//		B		RX_SE0
#endif
		// 4 cycles for buffer boundary check
		MOV		R0, SP
		SUBS	R0, R1, R0
		CMP		R0, #0x10
		BGE		EXIT_RX
BIT6_OK
		// TODO: adjust number of NOP here for no stuff delay
//		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R2, R2, R5
		// if (!sample0) break;
		BNE		BIT6_OK_48
		B		RX_SE0
BIT6_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x40
#endif
		// sample1 ^= sample0
		EORS	R3, R3, R2
		// if (sample1 & (1 << IFS_USBDIO_DP))
		LSLS	R3, R3, #(31 - IFS_USBDIO_DP)
		BPL		BIT6_SAMPLE_0
		// sample 1: byte |= 0x40;
		ORRS	R6, R6, R0
		B		BIT6_BITSTUFFING
BIT6_SAMPLE_0
		// sample 0: byte &= ~0x40;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT6_BITSTUFFING
		// if (!(byte & 0x7E))
		MOVS	R0, #0x7E
		TST		R6, R0
		BNE		SAMPLE_BIT7
		NOP		// for balance the BNE
		// stuff bit
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R2, R2, R5
		ANDS	R0, R0, R5
		CMP		R0, R2
		BNE		BIT6_STUFF_OK
		B		EXIT_RX
BIT6_STUFF_OK
		// sample0 = tmp
		MOV		R2, R0
		MOVS	R0, #0x40
#endif
		// stuff &= ~0x40;
		BICS	R7, R7, R0
		// byte |= 0x40;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif

SAMPLE_BIT7
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
#else
		// sample1 = *input & mask;
		LDR		R3, [R4]
		// remove SE0 check to get 3 more cycles for calculating data
//		ANDS	R3, R3, R5
		// if (!sample) break;
//		BNE		BIT7_OK
//		B		RX_SE0
#endif
BIT7_OK
		// TODO: adjust number of NOP here for no stuff delay
		NOP
#ifdef CLK_48M
		// sample0 = *input & mask;
		ANDS	R3, R3, R5
		// if (!sample0) break;
		BNE		BIT7_OK_48
		B		RX_SE0
BIT7_OK_48
		NOP
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
#ifndef DEBUG
		MOVS	R0, #0x80
#endif
		EORS	R2, R2, R3
		// if (sample0 & (1 << IFS_USBDIO_DP))
		LSLS	R2, R2, #(31 - IFS_USBDIO_DP)
		BPL		BIT7_SAMPLE_0
		// sample 1: byte |= 0x80;
		ORRS	R6, R6, R0
		B		BIT7_BITSTUFFING
BIT7_SAMPLE_0
		// sample 0: byte &= ~0x80;
		BICS	R6, R6, R0
		NOP		// for balance if delay
BIT7_BITSTUFFING
		// if (!(byte & 0xFC))
		MOVS	R0, #0xFC
		TST		R6, R0
		BNE		BIT7_NOSTUFF_DELAY
		NOP		// for balance the BNE
		// stuff bit
		NOP
		NOP
		NOP
#ifdef DEBUG
		MOVS	R0, #DEBUG_VALUE
		STR		R0, [R4]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#else
		// tmp = *input
		LDR		R0, [R4]
		// if (tmp & mask) == (sample0 & mask)
		ANDS	R3, R3, R5
		ANDS	R0, R0, R5
		CMP		R0, R3
		BNE		BIT7_STUFF_OK
		B		EXIT_RX
BIT7_STUFF_OK
		// sample0 = tmp
		MOV		R3, R0
		MOVS	R0, #0x80
#endif
		// stuff &= ~0x80;
		BICS	R7, R7, R0
		// byte |= 0x80;
		ORRS	R6, R6, R0
		// TODO: adjust stuff delay
		// use 2-cycle instructions to save same space
		LDRB	R0, [R1]
		NOP
#ifdef CLK_48M
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
		LDRB	R0, [R1]
#endif
BIT7_NOSTUFF_DELAY
		// 3 cycles calculate data and jump to SAMPLE_BIT0
		// stuff ^= byte;
		EORS	R7, R7, R6
/*
		// not enough cycles for code below
		// data calculating implemented in sample bit7
		// data saving implemented in sample bit0
		// buffer pointer increasing implemented in sample bit2
		// buffer boundary check implemented in sample bit6
		MOV		R0, SP
		SUBS	R0, R0, R1
		CMP		R0, #0x10
		BGE		EXIT_RX
		// stuff ^= byte;
		EORS	R7, R7, R6
		// *buff = byte;
		STRB	R7, [R1]
		// stuff = 0xFF;
		MOVS	R7, #0xFF
		// buf++;
		ADD		R1, R1, #1
*/		B		SAMPLE_BIT0

RX_SE0	// SE0 received, callback to non-timing-critical C code to process data
#ifdef CLK_48M
		// check CRC16
		MOV		R0, SP
		SUBS	R0, R1, R0
		CMP		R0, #0x5
		BGE		CHECK_CRC
		B		CRC_END
CHECK_CRC
		LDR		R0, =CRC_BASE
		LDRH	R0, [R0]
		SUBS	R1, R1, #2
		LDRB	R2, [R1, #0]
		LDRB	R1, [R1, #1]
		LSLS	R1, R1, #8
		ORRS	R1, R1, R2
		CMP		R0, R1
		BNE		EXIT_RX
CRC_END
#endif
		// first 2 bits of SYNC is not sampled, which is both '0'
		MOV		R0, SP
		LDR		R1, [R0]
		MOVS	R2, #0x03
		BICS	R1, R1, R2
		STR		R1, [R0]
		// nv32_usbdio_rx(uint8_t *buff, int len)
		SUBS	R1, R1, R0
		BL		nv32_usbdio_rx

EXIT_RX
		// IRQ->SC |= IRQ_SC_IRQACK_MASK;
		LDR		R1, =IRQSC_REG
		LDRB	R0, [R1]
		MOVS	R2, #0x04
		ORRS	R0, R0, R2
		STRB	R0, [R1]

		ADD		SP, SP, #0x14
		POP		{R4-R7, PC}

USBDIO_Tx
		PUSH	{R4-R7, LR}
		// R0: uint8_t *txbuf;
		// R1: uint8_t *end = txbuf + len;
		ADDS	R1, R1, R0
		// txbuf--;
		SUBS	R0, R0, #1
		// R2: uint8_t byte;
		// R3: uint32_t *output;
		LDR		R3, =GPIO_TOGGLE_REG
		// R4: uint32_t mask;
		LDR		R4, =DPDM_MASK
		// R5: uint8_t stuffing = 0;
		MOVS	R5, #0
		// R6: uint8_t bitmask;

//												no_stuff			stuff
TX_BYTE
		ADDS	R0, R0, #1						// [8 : 1]
		// if (txbuf >= end) exit;
		CMP		R0, R1							// [9 : 1]
		BGE		TX_SE0							// [10: 1]
		// byte = *txbuf++;
		LDRB	R2, [R0]						// [11: 2]
		// bitmask = 1 << 0;
		MOVS	R6, #1							// [13: 1]

TX_BIT
		// if (byte & bitmask)
		TST		R2, R6							// [14: 1]
		BNE		STUFFING						// [15: 1/2]
		// toggle GPIO
		STR		R4, [R3]						// [0 : 1]
		// stuffing = 0;
		MOVS	R5, #0							// [1 : 1]
		B		NO_STUFFING						// [2 : 2]
STUFFING
		// stuffing++;
		ADDS	R5, R5, #1						//					[1 : 1]
		CMP		R5, #6							//					[2 : 1]
		BGE		TX_STUFF						//					[3 : 1/2]
		// bitmask <<= 1;
		LSLS	R6, R6, #1						//					[4 : 1]
		// if (bitmask >= 0x100)
		LSRS	R7, R6, #8						//					[5 : 1]
		BNE		TX_BYTE							//					[6 : 1/2]
		LDRB	R7, [R0]						// 					[7 : 2]
		LDRB	R7, [R0]						//					[9 : 2]
		NOP										//					[11: 1]
		B		TX_BIT							//					[12: 2]
TX_STUFF
		// stuffing = 0;
		MOVS	R5, #0							//					[5 : 1]
		LDRB	R7, [R0]						//					[6 : 2]
		LDRB	R7, [R0]						//					[8 : 2]
		LDRB	R7, [R0]						//					[10: 2]
		LDRB	R7, [R0]						//					[12: 2]
		LDRB	R7, [R0]						//					[14: 2]
		// toggle GPIO
		STR		R4, [R3]						//					[0 : 1]
		LDRB	R7, [R0]						//					[1 : 2]
		NOP										//					[3 : 1]

NO_STUFFING
		// bitmask <<= 1;
		LSLS	R6, R6, #1						// [4 : 1]			[4 : 1]
		// if (bitmask >= 0x100)
		LSRS	R7, R6, #8						// [5 : 1]			[5 : 1]
		BNE		TX_BYTE							// [6 : 1/2]		[6 : 1/2]
		LDRB	R7, [R0]						// [7 : 2]			[7 : 2]
		LDRB	R7, [R0]						// [9 : 2]			[9 : 2]
		NOP										// [11: 1]			[11: 1]
		B		TX_BIT							// [12: 2]			[12: 2]

TX_SE0
		LDR		R3, =GPIO_CLEAR_REG				// [12: 2]
		LDRB	R7, [R0]						// [14: 2]
		STR		R4, [R3]						// [0 : 1]
		LDR		R3, =GPIO_SET_REG				// [1 : 2]
		MOVS	R7, #1							// [3 : 1]
		LSLS	R4, R7, #IFS_USBDIO_DP			// [4 : 1]
		NOP										// [5 : 1]
		LDRB	R7, [R0]						// [6 : 2]
		LDRB	R7, [R0]						// [8 : 2]
		LDRB	R7, [R0]						// [10: 2]
		LDRB	R7, [R0]						// [12: 2]
		LDRB	R7, [R0]						// [14: 2]
		LDRB	R7, [R0]						// [16: 2]
		LDRB	R7, [R0]						// [18: 2]
		LDRB	R7, [R0]						// [20: 2]
		LDRB	R7, [R0]						// [22: 2]
		LDRB	R7, [R0]						// [24: 2]
		LDRB	R7, [R0]						// [26: 2]
		LDRB	R7, [R0]						// [28: 2]
		LDRB	R7, [R0]						// [30: 2]
		STR		R4, [R3]						// [0 : 1]

		POP		{R4-R7, PC}

		END
