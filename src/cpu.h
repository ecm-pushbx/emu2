/*
 * This is based on code by David Hedley, from pcemu.
 *
 * Most of the CPU emulation was rewritten and code was extended to support
 * 80186 and some 81280 instructions.
 */

#pragma once
#include <stdint.h>

// Enable/disable 80286 stack emulation, 80286 and higher push the old value of
// SP, 8086/80186 push new value.
//
// This is used by some software to detect extra instructions that are present
// in the 80186 also, so we emulate this even if no 80286 instructions are
// supported.
// #define CPU_PUSH_80286
// ecm: Better to present as an 186+HMA+XMS instead of a 286 that
//  faults on any 0Fh-prefixed instruction. So disable this.

// Enable 80186 shift behaviour - shift count is modulo 32.
// This is used in some software to detect 80186 and higher.
#define CPU_SHIFT_80186

enum
{
    AX = 0,
    CX,
    DX,
    BX,
    SP,
    BP,
    SI,
    DI
};

enum
{
    ES = 0,
    CS,
    SS,
    DS,
    NoSeg
};

#define SetZFB(x) (ZF = !(uint8_t)(x))
#define SetZFW(x) (ZF = !(uint16_t)(x))
#define SetPF(x)  (PF = parity_table[(uint8_t)(x)])
#define SetSFW(x) (SF = (x)&0x8000)
#define SetSFB(x) (SF = (x)&0x80)

extern uint16_t fl_mask_on;
extern uint16_t fl_mask_preserve;
extern uint16_t fl_preserve;

#define CompressFlags()                                                                  \
    (uint16_t)(CF | (PF << 2) | (!(!AF) << 4) | (ZF << 6) | (!(!SF) << 7) |          \
               (TF << 8) | (IF << 9) | (DF << 10) | (!(!OF) << 11) | \
               fl_mask_on | (fl_preserve & fl_mask_preserve))
/* 80286 detection checks flags like so.
It expects the top 4 bits be forced set for 8086/186, after writing zeroes:

		xor ax,ax
		push ax
		popf	; try to clear all bits
		pushf
	        pop ax
	and ax,0f000h
	cmp ax,0f000h
	jnz is286		; 4 msb stuck to 1: 808x or 80186

80386 detection checks that the top 4 bits are not all forced clear:

		mov ax,0f000h
		push ax
		popf	; try to set 4 msb
		pushf
		pop ax
	test ax,0f000h
	jz noid		; 4 msb stuck to 0: 80286
	mov byte [family],3	; at least 386

From https://hg.pushbx.org/ecm/cpulevel/file/43b74982baeb/cpulevel.asm

*/

#define ExpandFlags(f)                                                                   \
    {                                                                                    \
        CF = (f)&1;                                                                      \
        PF = ((f)&4) == 4;                                                               \
        AF = (f)&16;                                                                     \
        ZF = ((f)&64) == 64;                                                             \
        SF = (f)&128;                                                                    \
        TF = ((f)&256) == 256;                                                           \
        IF = ((f)&512) == 512;                                                           \
        DF = ((f)&1024) == 1024;                                                         \
        OF = (f)&2048;                                                                   \
        fl_preserve = (f) & fl_mask_preserve; \
    }
