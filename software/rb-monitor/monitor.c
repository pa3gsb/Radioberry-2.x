/* 
* Copyright (C)
* 2018 - Johan Maas, PA3GSB
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
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
*	2018 Johan PA3GSB
*
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>
#include <unistd.h>

#define	RX1_PIN	6

static volatile int globalCounter = 0 ;

time_t rawtime;
struct tm *info;


/*
 * ISR: Overrun logging.
 */
void overrunInterrupt (void)
{
  ++globalCounter ;
}

int main (void)
{
  int myCounter = 0 ;
  
  	fprintf(stderr,"\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr,	"====================================================================\n");
	fprintf(stderr, "                      Radioberry Monitor\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "                    Prints radioberry status 24-5-2018 \n");
	fprintf(stderr,	"\n");
	fprintf(stderr,	"\n");
	fprintf(stderr, "====================================================================\n");
	fprintf(stderr, "====================================================================\n");

  if (wiringPiSetup () < 0)
  {
    fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiISR (RX1_PIN, INT_EDGE_RISING, &overrunInterrupt) < 0)
  {
    fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
    return 1 ;
  }

  long save_min = 0;
  time(&rawtime);
  info = gmtime(&rawtime );
  save_min = info->tm_min;
  printf ("Radioberry monitor started ... \n");
  for (;;)
  {
	fflush (stdout) ;
	usleep(100000);
	time(&rawtime);
	info = gmtime(&rawtime );
	if (info->tm_min != save_min) {
		save_min = info->tm_min;
		printf ("During 1 minute %5d overruns are detected \n", globalCounter);
		globalCounter = 0;
	}
  }

  return 0 ;
}