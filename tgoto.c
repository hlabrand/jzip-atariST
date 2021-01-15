#include <string.h>
#include "termcap.h"

#ifndef NULL
# define NULL ((char *)0)
#endif



char *BC;
char *UP;


char *tgoto(const char *motion_string, int column, int line)
{
	const char *strp;
	char *bufp;
	short c;
	int param;
	int reversed;
	int overflow;
	static char strbuf[64];
	
	bufp = strbuf;
	param = line;
	reversed = 0;
	overflow = 0;
	strp = motion_string;
	if (strp == NULL)
		return strcpy(strbuf, "OOPS");
	while ((c = *strp++) != '\0')
	{
		if (c == '%')
		{
			switch (c = *strp++)
			{
			case 'd':
				if (param < 10) goto int1;
				if (param < 100) goto int2;
				*bufp++ = (param / 100) | '0';
				param %= 100;
		int2:	*bufp++ = (param / 10) | '0';
		int1:	*bufp++ = (param % 10) | '0';
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;
			case '3':
				*bufp++ = (param / 100) | '0';
				param %= 100;
				*bufp++ = (param / 10) | '0';
				*bufp++ = (param % 10) | '0';
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;
			case '2':
				*bufp++ = (param / 10) | '0';
				*bufp++ = (param % 10) | '0';
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;
			case 'r':
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;
			case '+':
				param += *strp++;
				if (param == 0 || param == '\n')
				{
					param++;
					overflow |= reversed + 1;
				}
				*bufp++ = param;
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;				
			case '.':
				if (param == 0 || param == '\n')
				{
					param++;
					overflow |= reversed + 1;
				}
				*bufp++ = param;
				reversed = 1 - reversed;
				param = reversed ? column : line;
				break;
			case '>':
				if (param > *strp++)
					param += *strp++;
				else
					strp++;
				break;
			case 'i':
				column++;
				line++;
				param++;
				break;
			case '%':
				*bufp++ = c;
				break;
			case 'B':
				param = (16 * (param / 10)) + (param % 10);
				break;
			case 'D':
				param = param - 2 * (param % 16);
				break;
			default:
				return "OOPS";
			}
		} else
		{
			*bufp++ = c;
		}
	}
	if (overflow != 0)
	{
		if (overflow & 1)
		{
			strcpy(bufp, UP);
			bufp += strlen(UP);
		}
		if (overflow & 2)
		{
			if (BC != NULL && *BC != '\0')
			{
				strcpy(bufp, BC);
				bufp += strlen(BC);
			} else
			{
				*bufp++ = '\b';
			}
		}
	}
	*bufp = '\0';
	return strbuf;
}
