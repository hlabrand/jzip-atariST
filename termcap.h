#ifndef __TERMCAP_H__
#define __TERMCAP_H__ 1

#define TCAPSLEN 4096

extern char *BC;
extern char *UP;
extern char PC_;
extern short ospeed;


int tgetent(char *, const char *);
int tgetflag(const char *);
char *tgetstr(const char *, char **);
int tgetnum(const char *);
char *tgoto(const char *, int, int);
void tputs(const char *, int, void (*)(int));


#endif /* __TERMCAP_H__ */
