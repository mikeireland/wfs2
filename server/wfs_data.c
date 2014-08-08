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
/* Since it is called inside the USB thread for now the usb mutex will  */
/* be locked while it is running. 					*/
/************************************************************************/

void process_data(long time_stamp, int nx, int ny, unsigned short int *ccd)
{
	/* OK, we have processed one more frame. */

	number_of_processed_frames++;

} /* process_data() */
