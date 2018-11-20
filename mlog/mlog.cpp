#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>

#define MAXBUFLOG 512
#define MAXNBLOG 1024

static unsigned int g_logindex = 0 ;
static char g_logstack[MAXNBLOG][MAXBUFLOG];
static unsigned long g_dtstack[MAXNBLOG];
static bool g_turnindex = false ;
static bool firstuse = true ;

struct timeval g_clockprevious , g_clock ;

void mlogpush(const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(g_logstack[g_logindex], format, args);
	va_end(args);
	gettimeofday(&g_clock, NULL);
	if (firstuse )
	{
		g_dtstack[g_logindex] = 0 ;
		firstuse = false ;
	}
	else
		g_dtstack[g_logindex] = 1000000 * (g_clock.tv_sec - g_clockprevious.tv_sec) + ( g_clock.tv_usec - g_clockprevious.tv_usec) ;
	g_clockprevious = g_clock ;
	g_logindex++;
	if (g_logindex >= MAXNBLOG)
	{
		g_logindex = 0 ;
		g_turnindex = true ;
	}
}
void mlogflush(char *mfile)
{
	if (firstuse)
		return ;
	if ((g_logindex == 0) && (g_turnindex == false))
		return ;
	
	FILE * pFile = NULL ; 
	if ( mfile != NULL )
		pFile = fopen(mfile, "w");;
	if (pFile == NULL)
		pFile = stderr ;
	fprintf(pFile, "mlog g_turnindex=%d , g_log_index=%d\n", g_turnindex , g_logindex);
	for(unsigned int i = (g_turnindex?g_logindex:0) ; i !=  ((g_logindex==0)?(MAXNBLOG-1):(g_logindex-1)) ; i++ )
	{
		if (i >= MAXNBLOG)
			i = 0 ;
		fprintf(pFile, "dt=%8ld %s\n", g_dtstack[i] , g_logstack[i]);
	}
	fclose(pFile);
}

