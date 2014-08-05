/************************************************************************/
/* wfs_data.c								*/
/*                                                                      */
/* Here is where the real WFS calculations will be done. For now, this	*/
/* is a place holder function and uses USB data. IT will eventually	*/
/* get data via CameraLink.						*/
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY SERVER LIB   				*/
/*                 Based on the CHARA User Interface			*/
/*                 Based on the SUSI User Interface			*/
/*		In turn based on the CHIP User interface		*/
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA			*/
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu			                */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar 		                        */
/* Date   : Aug 2012							*/
/************************************************************************/

#include "wfs_server.h"

/************************************************************************/
/* process_data()							*/
/*									*/
/* Place holder function for WFS calculations. For now uses USB derived */
/* data.								*/
/* Since it is called inside the USB thread for now the usb mutex will  */
/* be locked while it is running. 					*/
/************************************************************************/

#warning Look into how the newer version did the timing, we should copy that.
void process_data(long time_stamp, int nx, int ny, unsigned short int *ccd)
{
	float	data[90][90];
	float	div;
	unsigned short int *pi1;
	int	i,j,k,l,m,n;

	/* Is this a 90x90 region? */

	if (nx != 90 || ny != 90) return;

	/* 
	 * First, let's put the data into a nice float array.
	 * I'm still not entirely sure which way the pixels come out
	 * but this order, based on the GTK GUI, at least gives
	 * a reasonable result.
	 */

	for(pi1 = ccd, j=0; j < ny; j++)
	for(i=0; i < nx; i++) data[i][j] = *pi1++;
	
	/* Now, go through each cell and work out the cogs */

	wfs_status.mean_cog_x = 0.0;
	wfs_status.mean_cog_y = 0.0;

	for(k=0; k<5; k++)
	for(l=0; l<5; l++)
	{
	    wfs_status.cog_x[k][l] = 0.0;
	    wfs_status.cog_y[k][l] = 0.0;
	    div = 0.0;

	    m = k * 18;
	    n = l * 18;

	    for(i=0; i<18; i++)
	    for(j=0; j<18; j++)
	    {
		wfs_status.cog_x[k][l] += ((float)(i-8.5) * data[i+m][j+n]);
		wfs_status.cog_y[k][l] += ((float)(j-8.5) * data[i+m][j+n]);
		div += data[i+m][j+n];
	    }

	    if (div == 0.0)
	    {
	        wfs_status.cog_x[k][l] = 0.0;
	        wfs_status.cog_y[k][l] = 0.0;
	    }
	    else
	    {
	        if ((wfs_status.cog_x[k][l] /= div) < -1.0)
			wfs_status.cog_x[k][l] = -1.0;
		else if (wfs_status.cog_x[k][l] > 1.0)
			wfs_status.cog_x[k][l] = 1.0;

	        if ((wfs_status.cog_y[k][l] /= div) < -1.0)
			wfs_status.cog_y[k][l] = -1.0;
		else if (wfs_status.cog_y[k][l] > 1.0)
			wfs_status.cog_y[k][l] = 1.0;
	    }

	    wfs_status.mean_cog_x += wfs_status.cog_x[k][l];
	    wfs_status.mean_cog_y += wfs_status.cog_y[k][l];

	}

	wfs_status.mean_cog_x /= (float)(nx *ny);
	wfs_status.mean_cog_y /= (float)(nx *ny);

} /* process_data() */
