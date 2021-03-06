/***************************************************************************
 *   Copyright (C) 2011 by Broadcom Corporation                            *
 *   Evan Hunter - ehunter@broadcom.com                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rtos.h"
#include "target/armv7m.h"

#include "log.h"

static const struct stack_register_offset rtos_standard_RiscV32I_stack_offsets[] = {
	{ -1 , 32 },		/* X0 */
	{ 0x00, 32 },		/* x1 ra  */
	{ 0x04, 32 },		/* x2 sp  */
	{ 0x08, 32 },		/* x3 gp  */
	{ 0x0c, 32 },		/* x4 tp  */
	{ 0x10, 32 },		/* x5 t0  */
	{ 0x14, 32 },		/* x6 t1  */
	{ 0x18, 32 },		/* x7 t2  */
	{ 0x1c, 32 },		/* x8 s0/fp  */
	{ 0x20, 32 },		/* x9 s1  */
	{ 0x24, 32 },		/* x10 a0 */
	{ 0x28, 32 },		/* x11 a1 */
	{ 0x2c, 32 },		/* x12 a2 */
	{ 0x30, 32 },		/* x13 a3  */
	{ 0x34, 32 },		/* x14 a4  */
	{ 0x38, 32 },		/* x15 a5  */
	{ 0x3c, 32 },		/* x16 a6 */
	{ 0x40, 32 },		/* x17 a7  */
	{ 0x44, 32 },		/* x18 s2  */
	{ 0x48, 32 },		/* x19 s3  */
	{ 0x4c, 32 },		/* x20 s4  */
	{ 0x50, 32 },		/* x21 s5  */
	{ 0x54, 32 },		/* x22 s6  */
	{ 0x58, 32 },		/* x23 s7  */
	{ 0x5c, 32 },		/* x24 s8  */
	{ 0x60, 32 },		/* x25 s9  */
	{ 0x64, 32 },		/* x26 s10  */
	{ 0x68, 32 },		/* x27 s11 */
	{ 0x6c, 32 },		/* x28 t3 */
	{ 0x70, 32 },		/* x29 t4 */
	{ 0x74, 32 },		/* x30 t5  */
	{ 0x78, 32 },		/* x31 t6  */
	{ 0x80, 32 }		/* PC */
};
static const struct stack_register_offset rtos_standard_Cortex_M3_stack_offsets[ARMV7M_NUM_CORE_REGS] = {
	{ 0x20, 32 },		/* r0   */
	{ 0x24, 32 },		/* r1   */
	{ 0x28, 32 },		/* r2   */
	{ 0x2c, 32 },		/* r3   */
	{ 0x00, 32 },		/* r4   */
	{ 0x04, 32 },		/* r5   */
	{ 0x08, 32 },		/* r6   */
	{ 0x0c, 32 },		/* r7   */
	{ 0x10, 32 },		/* r8   */
	{ 0x14, 32 },		/* r9   */
	{ 0x18, 32 },		/* r10  */
	{ 0x1c, 32 },		/* r11  */
	{ 0x30, 32 },		/* r12  */
	{ -2,   32 },		/* sp   */
	{ 0x34, 32 },		/* lr   */
	{ 0x38, 32 },		/* pc   */
	{ 0x3c, 32 },		/* xPSR */
};

static const struct stack_register_offset rtos_standard_Cortex_M4F_stack_offsets[] = {
	{ 0x24, 32 },		/* r0   */
	{ 0x28, 32 },		/* r1   */
	{ 0x2c, 32 },		/* r2   */
	{ 0x30, 32 },		/* r3   */
	{ 0x00, 32 },		/* r4   */
	{ 0x04, 32 },		/* r5   */
	{ 0x08, 32 },		/* r6   */
	{ 0x0c, 32 },		/* r7   */
	{ 0x10, 32 },		/* r8   */
	{ 0x14, 32 },		/* r9   */
	{ 0x18, 32 },		/* r10  */
	{ 0x1c, 32 },		/* r11  */
	{ 0x34, 32 },		/* r12  */
	{ -2,   32 },		/* sp   */
	{ 0x38, 32 },		/* lr   */
	{ 0x3c, 32 },		/* pc   */
	{ 0x40, 32 },		/* xPSR */
};

static const struct stack_register_offset rtos_standard_Cortex_M4F_FPU_stack_offsets[] = {
	{ 0x64, 32 },		/* r0   */
	{ 0x68, 32 },		/* r1   */
	{ 0x6c, 32 },		/* r2   */
	{ 0x70, 32 },		/* r3   */
	{ 0x00, 32 },		/* r4   */
	{ 0x04, 32 },		/* r5   */
	{ 0x08, 32 },		/* r6   */
	{ 0x0c, 32 },		/* r7   */
	{ 0x10, 32 },		/* r8   */
	{ 0x14, 32 },		/* r9   */
	{ 0x18, 32 },		/* r10  */
	{ 0x1c, 32 },		/* r11  */
	{ 0x74, 32 },		/* r12  */
	{ -2,   32 },		/* sp   */
	{ 0x78, 32 },		/* lr   */
	{ 0x7c, 32 },		/* pc   */
	{ 0x80, 32 },		/* xPSR */
};


static const struct stack_register_offset rtos_standard_Cortex_R4_stack_offsets[] = {
	{ 0x08, 32 },		/* r0  (a1)   */
	{ 0x0c, 32 },		/* r1  (a2)  */
	{ 0x10, 32 },		/* r2  (a3)  */
	{ 0x14, 32 },		/* r3  (a4)  */
	{ 0x18, 32 },		/* r4  (v1)  */
	{ 0x1c, 32 },		/* r5  (v2)  */
	{ 0x20, 32 },		/* r6  (v3)  */
	{ 0x24, 32 },		/* r7  (v4)  */
	{ 0x28, 32 },		/* r8  (a1)  */
	{ 0x2c, 32 },		/* r9  (sb)  */
	{ 0x30, 32 },		/* r10 (sl) */
	{ 0x34, 32 },		/* r11 (fp) */
	{ 0x38, 32 },		/* r12 (ip) */
	{ -2,   32 },		/* sp   */
	{ 0x3c, 32 },		/* lr   */
	{ 0x40, 32 },		/* pc   */
	{ -1,   96 },		/* FPA1 */
	{ -1,   96 },		/* FPA2 */
	{ -1,   96 },		/* FPA3 */
	{ -1,   96 },		/* FPA4 */
	{ -1,   96 },		/* FPA5 */
	{ -1,   96 },		/* FPA6 */
	{ -1,   96 },		/* FPA7 */
	{ -1,   96 },		/* FPA8 */
	{ -1,   32 },		/* FPS  */
	{ 0x04, 32 },		/* CSPR */
};

static const struct stack_register_offset rtos_standard_NDS32_N1068_stack_offsets[] = {
	{ 0x88, 32 },		/* R0  */
	{ 0x8C, 32 },		/* R1 */
	{ 0x14, 32 },		/* R2 */
	{ 0x18, 32 },		/* R3 */
	{ 0x1C, 32 },		/* R4 */
	{ 0x20, 32 },		/* R5 */
	{ 0x24, 32 },		/* R6 */
	{ 0x28, 32 },		/* R7 */
	{ 0x2C, 32 },		/* R8 */
	{ 0x30, 32 },		/* R9 */
	{ 0x34, 32 },		/* R10 */
	{ 0x38, 32 },		/* R11 */
	{ 0x3C, 32 },		/* R12 */
	{ 0x40, 32 },		/* R13 */
	{ 0x44, 32 },		/* R14 */
	{ 0x48, 32 },		/* R15 */
	{ 0x4C, 32 },		/* R16 */
	{ 0x50, 32 },		/* R17 */
	{ 0x54, 32 },		/* R18 */
	{ 0x58, 32 },		/* R19 */
	{ 0x5C, 32 },		/* R20 */
	{ 0x60, 32 },		/* R21 */
	{ 0x64, 32 },		/* R22 */
	{ 0x68, 32 },		/* R23 */
	{ 0x6C, 32 },		/* R24 */
	{ 0x70, 32 },		/* R25 */
	{ 0x74, 32 },		/* R26 */
	{ 0x78, 32 },		/* R27 */
	{ 0x7C, 32 },		/* R28 */
	{ 0x80, 32 },		/* R29 */
	{ 0x84, 32 },		/* R30 (LP) */
	{ 0x00, 32 },		/* R31 (SP) */
	{ 0x04, 32 },		/* PSW */
	{ 0x08, 32 },		/* IPC */
	{ 0x0C, 32 },		/* IPSW */
	{ 0x10, 32 },		/* IFC_LP */
};

static int64_t rtos_generic_stack_align(struct target *target,
	const uint8_t *stack_data, const struct rtos_register_stacking *stacking,
	int64_t stack_ptr, int align)
{
	int64_t new_stack_ptr;
	int64_t aligned_stack_ptr;
	new_stack_ptr = stack_ptr - stacking->stack_growth_direction *
		stacking->stack_registers_size;
	aligned_stack_ptr = new_stack_ptr & ~((int64_t)align - 1);
	if (aligned_stack_ptr != new_stack_ptr &&
		stacking->stack_growth_direction == -1) {
		/* If we have a downward growing stack, the simple alignment code
		 * above results in a wrong result (since it rounds down to nearest
		 * alignment).  We want to round up so add an extra align.
		 */
		aligned_stack_ptr += (int64_t)align;
	}
	return aligned_stack_ptr;
}

int64_t rtos_generic_stack_align8(struct target *target,
	const uint8_t *stack_data, const struct rtos_register_stacking *stacking,
	int64_t stack_ptr)
{
	return rtos_generic_stack_align(target, stack_data,
			stacking, stack_ptr, 8);
}

/* The Cortex-M3 will indicate that an alignment adjustment
 * has been done on the stack by setting bit 9 of the stacked xPSR
 * register.  In this case, we can just add an extra 4 bytes to get
 * to the program stack.  Note that some places in the ARM documentation
 * make this a little unclear but the padding takes place before the
 * normal exception stacking - so xPSR is always available at a fixed
 * location.
 *
 * Relevant documentation:
 *    Cortex-M series processors -> Cortex-M3 -> Revision: xxx ->
 *        Cortex-M3 Devices Generic User Guide -> The Cortex-M3 Processor ->
 *        Exception Model -> Exception entry and return -> Exception entry
 *    Cortex-M series processors -> Cortex-M3 -> Revision: xxx ->
 *        Cortex-M3 Devices Generic User Guide -> Cortex-M3 Peripherals ->
 *        System control block -> Configuration and Control Register (STKALIGN)
 *
 * This is just a helper function for use in the calculate_process_stack
 * function for a given architecture/rtos.
 */
int64_t rtos_Cortex_M_stack_align(struct target *target,
	const uint8_t *stack_data, const struct rtos_register_stacking *stacking,
	int64_t stack_ptr, size_t xpsr_offset)
{
	const uint32_t ALIGN_NEEDED = (1 << 9);
	uint32_t xpsr;
	int64_t new_stack_ptr;

	new_stack_ptr = stack_ptr - stacking->stack_growth_direction *
		stacking->stack_registers_size;
	xpsr = (target->endianness == TARGET_LITTLE_ENDIAN) ?
			le_to_h_u32(&stack_data[xpsr_offset]) :
			be_to_h_u32(&stack_data[xpsr_offset]);
	if ((xpsr & ALIGN_NEEDED) != 0) {
		LOG_DEBUG("XPSR(0x%08" PRIx32 ") indicated stack alignment was necessary\r\n",
			xpsr);
		new_stack_ptr -= (stacking->stack_growth_direction * 4);
	}
	return new_stack_ptr;
}

static int64_t rtos_standard_Cortex_M3_stack_align(struct target *target,
	const uint8_t *stack_data, const struct rtos_register_stacking *stacking,
	int64_t stack_ptr)
{
	const int XPSR_OFFSET = 0x3c;
	return rtos_Cortex_M_stack_align(target, stack_data, stacking,
		stack_ptr, XPSR_OFFSET);
}

int64_t rtos_standard_RiscV32I_stack_align(struct target *target,
	const uint8_t *stack_data, const struct rtos_register_stacking *stacking,
	int64_t stack_ptr)
{
	int64_t new_stack_ptr;
	int64_t aligned_stack_ptr;
#if 0
	int i, *ptr=(int*)stack_data;
	for(i=1; i<32; i++)
	{
		LOG_INFO("X%02d = 0x%08x.", i, *ptr++);
	}
#endif
	new_stack_ptr = stack_ptr - stacking->stack_growth_direction *
		stacking->stack_registers_size;
	
	aligned_stack_ptr = new_stack_ptr & ~((int64_t)4 - 1);
	if (aligned_stack_ptr != new_stack_ptr &&
		stacking->stack_growth_direction == -1) {
		/* If we have a downward growing stack, the simple alignment code
		 * above results in a wrong result (since it rounds down to nearest
		 * alignment).  We want to round up so add an extra align.
		 */
		aligned_stack_ptr += (int64_t)4;
	}
	//LOG_INFO("raw 0x%lx,before 0x%lx, now 0x%lx.", stack_ptr, new_stack_ptr, aligned_stack_ptr);
	return aligned_stack_ptr;
}
#define ARRAY_LEN(a)	(sizeof(a)/sizeof((a)[0]))
const struct rtos_register_stacking rtos_standard_RiscV32I_stacking = {
	0x84,					/* stack_registers_size */
	ARRAY_LEN(rtos_standard_RiscV32I_stack_offsets), 
	-1,						/* stack_growth_direction */
	32,						/* num_output_registers */
	rtos_standard_RiscV32I_stack_align,	/* stack_alignment */
	rtos_standard_RiscV32I_stack_offsets	/* register_offsets */
};

const struct rtos_register_stacking rtos_standard_Cortex_M3_stacking = {
	0x40,					/* stack_registers_size */
	ARRAY_LEN(rtos_standard_Cortex_M3_stack_offsets),
	-1,						/* stack_growth_direction */
	ARMV7M_NUM_CORE_REGS,	/* num_output_registers */
	rtos_standard_Cortex_M3_stack_align,	/* stack_alignment */
	rtos_standard_Cortex_M3_stack_offsets	/* register_offsets */
};

const struct rtos_register_stacking rtos_standard_Cortex_M4F_stacking = {
	0x44,					/* stack_registers_size 4 more for LR*/
	ARRAY_LEN(rtos_standard_Cortex_M4F_stack_offsets),
	-1,						/* stack_growth_direction */
	ARMV7M_NUM_CORE_REGS,	/* num_output_registers */
	rtos_standard_Cortex_M3_stack_align,	/* stack_alignment */
	rtos_standard_Cortex_M4F_stack_offsets	/* register_offsets */
};

const struct rtos_register_stacking rtos_standard_Cortex_M4F_FPU_stacking = {
	0xcc,					/* stack_registers_size 4 more for LR + 48 more for FPU S0-S15 register*/
	ARRAY_LEN(rtos_standard_Cortex_M4F_FPU_stack_offsets),
	-1,						/* stack_growth_direction */
	ARMV7M_NUM_CORE_REGS,	/* num_output_registers */
	rtos_standard_Cortex_M3_stack_align,	/* stack_alignment */
	rtos_standard_Cortex_M4F_FPU_stack_offsets	/* register_offsets */
};

const struct rtos_register_stacking rtos_standard_Cortex_R4_stacking = {
	0x48,				/* stack_registers_size */
	ARRAY_LEN(rtos_standard_Cortex_R4_stack_offsets),
	-1,					/* stack_growth_direction */
	26,					/* num_output_registers */
	rtos_generic_stack_align8,	/* stack_alignment */
	rtos_standard_Cortex_R4_stack_offsets	/* register_offsets */
};

const struct rtos_register_stacking rtos_standard_NDS32_N1068_stacking = {
	0x90,				/* stack_registers_size */
	ARRAY_LEN(rtos_standard_NDS32_N1068_stack_offsets),
	-1,					/* stack_growth_direction */
	32,					/* num_output_registers */
	rtos_generic_stack_align8,	/* stack_alignment */
	rtos_standard_NDS32_N1068_stack_offsets	/* register_offsets */
};
