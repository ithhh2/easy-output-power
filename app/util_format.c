#include "util_format.h"

static void write_digit(char *buf, uint8_t pos, char value)
{
	buf[pos] = value;
}

void format_voltage(char *buf, uint16_t v_x100)
{
	uint16_t whole = v_x100 / 100U;
	uint16_t frac = v_x100 % 100U;

	if (whole >= 10U)
	{
		write_digit(buf, 0, (char)('0' + (whole / 10U)));
		write_digit(buf, 1, (char)('0' + (whole % 10U)));
		write_digit(buf, 2, '.');
		write_digit(buf, 3, (char)('0' + (frac / 10U)));
		write_digit(buf, 4, (char)('0' + (frac % 10U)));
		write_digit(buf, 5, 'V');
		buf[6] = '\0';
		return;
	}

	write_digit(buf, 0, ' ');
	write_digit(buf, 1, (char)('0' + whole));
	write_digit(buf, 2, '.');
	write_digit(buf, 3, (char)('0' + (frac / 10U)));
	write_digit(buf, 4, (char)('0' + (frac % 10U)));
	write_digit(buf, 5, 'V');
	buf[6] = '\0';
}

void format_current_ma(char *buf, uint16_t ma)
{
	if (ma > 9999U)
	{
		ma = 9999U;
	}

	write_digit(buf, 4, 'm');
	write_digit(buf, 5, 'A');
	buf[6] = '\0';

	if (ma >= 1000U)
	{
		write_digit(buf, 0, (char)('0' + ((ma / 1000U) % 10U)));
		write_digit(buf, 1, (char)('0' + ((ma / 100U) % 10U)));
		write_digit(buf, 2, (char)('0' + ((ma / 10U) % 10U)));
		write_digit(buf, 3, (char)('0' + (ma % 10U)));
		return;
	}

	write_digit(buf, 3, (char)('0' + (ma % 10U)));

	if (ma >= 100U)
	{
		write_digit(buf, 0, ' ');
		write_digit(buf, 1, (char)('0' + ((ma / 100U) % 10U)));
		write_digit(buf, 2, (char)('0' + ((ma / 10U) % 10U)));
		return;
	}

	write_digit(buf, 2, (char)('0' + ((ma / 10U) % 10U)));

	if (ma >= 10U)
	{
		write_digit(buf, 0, ' ');
		write_digit(buf, 1, ' ');
		return;
	}

	write_digit(buf, 0, ' ');
	write_digit(buf, 1, ' ');
}
