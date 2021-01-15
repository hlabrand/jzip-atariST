#include "termcap.h"

#define DELAYS 1

char PC_;


#ifndef NULL
# define NULL ((char *)0)
#endif


#if !defined(OSK) && !defined(__OSK__)
short ospeed = 0;
#endif



#if DELAYS
static short time_tab[] = {
	2000,
	1333,
	909,
	743,
	666,
	333,
	166,
	83,
	55,
	50,
	42,
	28,
	20,
	14,
	10,
	5,
	2
};
#endif


void tputs(const char *str, int lines_affected, void (*outfunc)(int))
{
#if DELAYS
	register short time_delay = 0;
#endif
	
	if (str == NULL)
		return;
	if (*str >= '0' && *str <= '9')
	{
		do
		{
#if DELAYS
			time_delay = time_delay * 10 + *str - '0';
#endif
			str++;
		} while (*str >= '0' && *str <= '9');
	}
#if DELAYS
	time_delay *= 10;
#endif
	if (*str == '.' && *++str >= '0' && *str <= '9')
	{
#if DELAYS
		time_delay += *str - '0';
#endif
		str++;
	}
	if (*str == '*')
	{
#if DELAYS
		time_delay *= lines_affected;
#endif
		str++;
	}
	while (*str != '\0')
	{
		(*outfunc)(*str++);
	}
#if DELAYS
	if (time_delay != 0 && ospeed >= 0 && ospeed < 17)
	{
		register short i;
		
		time_delay += time_tab[ospeed] / 2;
		for (i = time_delay / time_tab[ospeed]; i > 0; i--)
		{
			(*outfunc)(PC_);
		}
	}
#else
	if (lines_affected);
#endif
}
