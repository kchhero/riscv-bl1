/*
 * Copyright (C) 2018  Nexell Co., Ltd.
 * Author: deoks <truevirtue@nexell.co.kr>
 *
 * Nexell informs that this code and information is provided "as Is" base
 * and without warranty of any kind, either expressed or implied, including,
 * but not limited to the implied warranties of merchantabulity and/or
 * fitness for a aparticular purpose.
 *
 * This program is copyrighted by Nexell and does not allow modification or
 * distribution. In addition to the author (person in charge), the modifier
 * is responsible for the modification.
 */
#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Function Define */
 void serial_set_baudrate (int channel, int uclk, int baud_rate);
 int serial_init(unsigned int channel);

char serial_getc(void);
void serial_putc(char ch);

 int serial_is_uart_tx_done(void);
 int serial_is_tx_empty(void);
 int serial_is_busy(void);

#endif	//#ifndef __SERIAL_H__
