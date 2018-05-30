/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//#include <sysheader.h>
#include <memtester.h>
#include <nx_qemu_sim_printf.h>

#define MPTRS unsigned int

void simple_memtest(void)
{
	unsigned int* start, *end, *ptr;
	unsigned int  size;

	start = ((unsigned int *)0x80200000);
        end   = ((unsigned int *)0x80202000);
	/* end   = ((unsigned int *)(0x80000000 */
        /*                           + (g_ddr3_info.sdram_size * 1024 * 1024 - 1)));	//MB -> Byte */
	ptr = start;
	size = (unsigned int)(end - start) * sizeof(unsigned int);

	_dprintf("############## Simple Memory Test Start!! ###############\r\n");
	_dprintf("Start: 0x%x, End: 0x%x, Size: 0x%x \r\n",
		start, end, size);

	/* step xx. data write */
	_dprintf("Read/Write : \n");
	_dprintf("Write  \n");
	while (ptr < end) {
		*ptr = (unsigned int)((MPTRS)ptr);
#if 0
		if (((unsigned int)((MPTRS)ptr) & 0x3FFFFFL) == 0)
			_dprintf("0x%x:\r\n", ptr);
#endif
#if 0
		if (((unsigned int)((MPTRS)ptr) % PROGRESSOFTEN) == 0) {
			_dprintf("\b");
			_dprintf("%c", progress[++j%  PROGRESSLEN]);
		}
#endif
		ptr++;
	}
	_dprintf("\b\b\b\b\b\b\b\n");

	_dprintf("Compare  \n");
	ptr = start;
	while (ptr < end) {
#if 0
		if (*ptr != (unsigned int)((MPTRS)ptr))
			_dprintf("0x%08X: %16X\r\n", (unsigned int)((MPTRS)ptr), *ptr);
#else
		unsigned int data0 = *ptr, data1 = (unsigned int)((MPTRS)ptr);
		unsigned int i = 0;

		for (i = 0; i < 32; i++) {
			data0 &= 1UL << i;
			data1 &= 1UL << i;

			if (data0 != data1) {
//				_dprintf("[%dbit] 0x%x: %x\r\n", i,
//				(unsigned int)((MPTRS)ptr), *ptr);
				_dprintf("--------------------------------------"
						"\r\n");
				_dprintf("[%dbit] 0x%x: %x(0x%x: %x)\r\n",
						i, data1, data0, (unsigned int)((MPTRS)ptr),
						*ptr);
				_dprintf("--------------------------------------"
						"\r\n");
//				mask_bit |= 1UL << i;
			}
#if 0
			if ( (mask_bit != 0) && (i == 31) ) {
				_dprintf("[%Xbit] 0x%x: %x(0x%x: %x)\r\n",
						mask_bit, data1, data0, (unsigned int)((MPTRS)ptr), *ptr);
			}
#endif
		}

		ptr++;
#endif

#if 0
		if ((((MPTRS)ptr) & 0xFFFFFL) == 0)
			_dprintf("0x%x:\r\n", ptr);
#endif
	}
	_dprintf("\b\b\b\b\b\b\b\b\b\n");
	_dprintf("Done!   \r\n");

	/* step xx. bit shift test */
	_dprintf("Bit Shift  : \n");
	_dprintf("Write  \n");
	ptr = start;
	while (ptr < end) {
		*ptr = (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	_dprintf("\b\b\b\b\b\b\b\n");

	_dprintf("Compare  \n");
	ptr = start;
	while (ptr < end) {
		if (*ptr != (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			_dprintf("0x%x : 0x%x\r\n", ptr, *ptr);
		ptr++;
	}
	_dprintf("\b\b\b\b\b\b\b\b\b\n");
	_dprintf("Done!   \r\n");

	/* step xx. reserve bit test */
	_dprintf("Reverse Bit: \n");
	_dprintf("Write  \n");
	ptr = start;
	while (ptr < end) {
		*ptr = ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	_dprintf("\b\b\b\b\b\b\b\n");

	_dprintf("Compare  \n");
	ptr = start;
	while (ptr < end) {
		if (*ptr != ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			_dprintf("0x%x : 0x%x\r\n", ptr, *ptr);
		ptr++;
	}
	_dprintf("\b\b\b\b\b\b\b\b\b\n");
	_dprintf("Done!   \r\n");

	_dprintf("############## Simple Memory Test Done!!! ###############\r\n");
}

