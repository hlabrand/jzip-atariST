#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "termcap.h"
#if defined(OSK) || defined(__OSK__)
#define O_RDONLY 0
#else
#define writeln(file, s) if (write(file, s, sizeof(s)-1) != 0) {}
#endif
#if defined(__unix__) || defined(__HAIKU__) || defined(__BEOS__)
#include <unistd.h>
#include <fcntl.h>
#endif
#ifdef __MINT__
#include <unistd.h>
#include <fcntl.h>
#endif

#ifndef NULL
# define NULL ((char *)0)
#endif

#ifndef O_RDONLY
# define O_RDONLY 0
#endif


static int tcap_file;
static int search_level;
static char *tcbufp;


static int open_termcap(void)
{
	int fno;
	
#if defined(OSK) || defined(__OSK__)
	if ((fno = open("/dd/sys/termcap", O_RDONLY)) > 0)
		return fno;
	if ((fno = open("/h0/sys/termcap", O_RDONLY)) > 0)
		return fno;
	return open("/d0/sys/termcap", O_RDONLY);
#endif
#if defined(__unix__) || defined(__HAIKU__) || defined(__BEOS__)
	if ((fno = open("/etc/termcap", O_RDONLY)) > 0)
		return fno;
	return -1;
#endif
#if defined(__TOS__) || defined(atarist) || defined(__MSDOS__) || defined(MSDOS)
	if ((fno = open("c:\\etc\\termcap", O_RDONLY)) > 0)
		return fno;
	return -1;
#endif
}


static int test_entry(const char *buf, const char *type)
{
	const char *cp;
	const char *sp;
	
	if (*buf == '#')
		return 0;
	for (cp = buf; *cp != '\0' && *cp != ':'; cp++)
	{
		sp = type;
		while (*cp != '\0' && *cp++ == *sp++)
			;
		cp--;
		if (*--sp == '\0' && (*cp == '|' || *cp == ':' || *cp == '\0'))
			return 1;
		while (*cp != '\0' && *cp != ':' && *cp != '|')
			cp++;
	}
	return 0;
}


int tgetent(char *tcbuf, const char *term_type)
{
	char *cp;
	char *cp2;
	char mybuf[TCAPSLEN];
	char c;
	int nread;
	int bufpos;
	
	if (tcap_file == 0)
	{
		tcbufp = tcbuf;
		if ((cp = getenv("TERMCAP")) != NULL && *cp != '\0')
		{
			if (*cp == '/' || *cp == '\\' ||
				(isalpha((unsigned char)cp[0]) && cp[1] == ':'))
			{
				tcap_file = open(cp, O_RDONLY);
			} else
			{
				if ((cp2 = getenv("TERM")) == NULL ||
					strcmp(term_type, cp2) == 0)
				{
					strcpy(tcbuf, cp);
					return 1;
				}
				tcap_file = open_termcap();
			}
		}
		if (tcap_file == 0)
			tcap_file = open_termcap();
		if (tcap_file < 0)
			return tcap_file;
	}
	
	lseek(tcap_file, 0l, 0);
	nread = bufpos = 0;
	do
	{
		cp = tcbuf;
		for (;;)
		{
			if (bufpos == nread)
			{
				if ((nread = (int)read(tcap_file, mybuf, sizeof(mybuf))) <= 0)
				{
					if (search_level != 0)
						return 0;
					close(tcap_file);
					tcap_file = 0;
					return 0;
				}
				bufpos = 0;
			}
#if defined(__TOS__) || defined(atarist) || defined(__MSDOS__) || defined(MSDOS)
			if (mybuf[bufpos] == '\r')
			{
				--nread;
				if (bufpos == nread)
					continue;
				memmove(&mybuf[bufpos], &mybuf[bufpos + 1], nread - bufpos);
			}
#endif
			if ((c = mybuf[bufpos++]) == '\n')
			{
				if (cp <= tcbuf || cp[-1] != '\\')
					break;
				cp--;
				continue;
			}
			if (cp >= tcbuf + sizeof(mybuf))
			{
				writeln(2, "Termcap entry too long!\n");
				break;
			}
			*cp++ = c;
		}
		*cp = '\0';
	} while (test_entry(tcbuf, term_type) == 0);
	if (search_level != 0)
		return 1;
	cp = tcbuf;
	for (;;)
	{
		while (*cp != '\0' && *cp++ != ':')
			;
		if (*cp == '\0')
			break;
		if (*cp++ != 't' || *cp++ != 'c' || *cp++ != '=')
			continue;
		cp2 = cp;
		while (*++cp2 != '\0' && *cp2 != ':')
			;
		*cp2 = '\0';
		search_level++;
		if (tgetent(mybuf, cp) == 0)
		{
			return search_level = 0;
		}
		search_level--;
		cp2 = mybuf;
		while (*cp2 != '\0' && *cp2++ != ':')
			;
		if ((cp - tcbuf + strlen(cp2)) >= (sizeof(mybuf)+3))
		{
			writeln(2, "Termcap entry too long!\n");
			return 0;
		}
		strcpy(cp -= 3, cp2);
	}
	close(tcap_file);
	tcap_file = 0;
	return 1;
}


int tgetflag(const char *id)
{
	const char *cp;
	
	cp = tcbufp;
	for (;;)
	{
		while (*cp != '\0' && *cp++ != ':')
			;
		if (*cp == '\0')
			break;
		if (*cp++ != id[0])
			continue;
		if (*cp == '\0')
			continue;
		if (*cp++ != id[1])
			continue;
		if (*cp == '\0' || *cp == ':')
			return 1;
		if (*cp == '@')
			break;
	}
	return 0;
}


static char *_tgetstr(const char *str, char **strptr)
{
	char *dst;
	short c;
	int i;
	
	dst = *strptr;
	while ((c = *str++) != '\0' && c != ':')
	{
		switch (c)
		{
		case '\\':
			switch (c = *str++)
			{
				case 'E': c = 0x1b; break;
				case 'n': c = 0x0a; break;
				case 'r': c = 0x0d; break;
				case 't': c = 0x09; break;
				case 'b': c = 0x08; break;
				case 'f': c = 0x0c; break;
				case '\\':
				case '^': break;
				default:
					if (c >= '0' && c <= '7')
					{
						c -= '0';
						i = 2;
						do
						{
							c *= 8;
							c |= *str++ - '0';
						} while (--i != 0 && *str >= '0' && *str <= '7');
					}
					break;
			}
			break;
		case '^':
			c = *str++ & 0x1f;
			break;
		}
		*dst++ = (char)c;
	}
	*dst++ = '\0';
	str = *strptr;
	*strptr = dst;
	/* discards "const", but API requires it */
	return (char *)str;
}


char *tgetstr(const char *id, char **strptr)
{
	const char *cp;
	
	cp = tcbufp;
	for (;;)
	{
		while (*cp != '\0' && *cp++ != ':')
			;
		if (*cp == '\0')
			return NULL;
		if (*cp++ != id[0])
			continue;
		if (*cp == '\0')
			continue;
		if (*cp++ != id[1])
			continue;
		if (*cp == '@')
			return NULL;
		if (*cp == '=')
			break;
	}
	return _tgetstr(++cp, strptr);
}


int tgetnum(const char *id)
{
	const char *cp;
	int val;
	int base;
	
	cp = tcbufp;
	for (;;)
	{
		while (*cp != '\0' && *cp++ != ':')
			;
		if (*cp == '\0')
			return -1;
		if (*cp++ != id[0])
			continue;
		if (*cp == '\0')
			continue;
		if (*cp++ != id[1])
			continue;
		if (*cp == '@')
			return -1;
		if (*cp == '#')
			break;
	}
	base = 10;
	if (*++cp == '0')
		base = 8;
	val = 0;
	while (*cp >= '0' && *cp <= '9')	/* !!! '8' & '9' in octal ??? */
	{
		val *= base;
		val += *cp++ - '0';
	}
	return val;
}
