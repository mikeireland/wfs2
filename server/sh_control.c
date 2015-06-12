/************************************************************************/
/* sh_control.c                                                         */
/*                                                                      */
/* For controling the rotation stage					*/
/************************************************************************/
/*                                                                      */
/*     Center for High Angular Resolution Astronomy                     */
/* Georgia State University, Atlanta GA 30303-3083, U.S.A.              */
/*                                                                      */
/*                                                                      */
/* Telephone: 1-404-651-1882                                            */
/* Fax      : 1-404-651-1389                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu/~theo/theo.html                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Xiao Che							*/
/* Date   : Aug 2012                                                    */
/************************************************************************/

#include "./wfs_server.h"

static int tdc_des = -1;
static struct termios old_options;

/************************************************************************/
/* char_to_dec()                                            		*/
/*									*/
/* lendian is the switch between big and little edian       		*/
/************************************************************************/

long char_to_dec(char *s, int nbyte, bool ledian)
{
	long value=0;
	unsigned temp;
	int i;
	bool negflag;
	
	if(ledian) negflag = *(s+nbyte-1) & '\x80';
	else negflag = *(s) & '\x80';

	if(ledian)
	{
	  for(i=0; i < nbyte; i++)
	  {
	    if (!negflag)
		temp = (*(s+i)) & 0xFF;
	    else
		temp = (~(*(s+i))) & 0xFF;

	    value += temp << (i*8);
	  }
	}
	else
	{
	  for(i=0; i < nbyte; i++)
	  {
	    if (!negflag)
		temp = (*(s+i)) & 0xFF;
	    else
		temp = (~(*(s+i))) & 0xFF;

	    value += temp << ((nbyte-i)*8);
	  }
	}

	if (!negflag)
		return value;
	else
		return -(value+1);

} /* char_to_dec() */

/************************************************************************/
/* dec_to_char()							*/
/*									*/
/* n is expected to be 4 bytes                              		*/
/* lendian is the switch between big and little edian       		*/
/************************************************************************/

char *dec_to_char(long n, bool lendian)
{
	char *buff = (char *)malloc(4);

	if(lendian) n = ((n>>24)&0xff) |
	              ((n<<8)&0xff0000)|
	              ((n>>8)&0xff00)  |
	              ((n<<24)&0xff000000);

	*(buff) = (char)(n>>24) & 0xff;
	*(buff+1) = (char)(n>>16) & 0xff;
	*(buff+2) = (char)(n>>8) & 0xff;
	*(buff+3) = (char)n & 0xff;

	return buff;

} /*dec_to_char() */

/************************************************************************/
/* tdc_open()                                               		*/
/*									*/
/* open connection to the shack hartmann rotatoin stage     		*/
/* and initialize it                                        		*/
/************************************************************************/

int tdc_open(void)
{

	tdc_des = open(TDC, O_RDWR | O_NONBLOCK);

	if (tdc_des == -1)
	{
	    return error(ERROR,"open_port: Unable to open %s ",TDC);
	}

	if (verbose) error(MESSAGE,"TDC roation unit %s open.", TDC);
	
	return NOERROR;

} /*tdc_open()*/

/************************************************************************/
/* tdc_initialize()                                         		*/
/*                                                          		*/
/************************************************************************/

int tdc_initialize(void)
{
	
	struct termios options;

	tcgetattr(tdc_des, &old_options); /*get current port options */
	options = old_options;

	cfsetispeed(&options, TDC_BRATE); /*set baud rate*/
	cfsetospeed(&options, TDC_BRATE);

	options.c_cflag |= (CLOCAL | CREAD);/* Enable the receiver*/
	                                    /* and set local mode*/
	options.c_cflag &= ~PARENB; /* set No parity (8N1)*/
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	usleep(50000);		    /* Let this settle */

	/* disable cononical mode (buffered i/o) */

	options.c_lflag &= (~ICANON);

	tcsetattr(tdc_des, TCSANOW, &options); /* Set the options*/

#warning These things are here for initial test only and we should test more.

	//tdc_get_poscounter();               /*read the current position*/
	//tdc_deact_avmodes();                  /*deactivate the LED*/
	tdc_act_avmodes();                  /* activate the LED*/
	tdc_identify();			    /* LED should flash */
	

	tdc_status.max_vel = DFT_TDC_MAX_VEL;
	tdc_status.accel = DFT_TDC_ACCEL;

	tdc_setvelparams(tdc_status);

	return error(NOERROR,"SH rotation stage connection initialized");

}/* tdc_initialize() */

/************************************************************************/
/* tdc_close()                                               		*/
/*                                                          		*/
/* close the connection to the shack hartmann rotatoin stage 		*/
/************************************************************************/

int tdc_close(void)
{
	   
	close(tdc_des); /*close the port */                                        
	return error(MESSAGE,"SH rotation stage connection closed");

} /* tdc_close*/

/************************************************************************/
/* tdc_get_response()							*/
/*									*/
/* Tries to get a response. Will return number of bytes received	*/
/* or -1 on time out.							*/
/************************************************************************/

int tdc_get_response(int num_bytes, char first_return_byte, char *buffer)
{
	int	i = 0;
	time_t	start;

	start = time(NULL);

        while(i < num_bytes)
        {
          while(read(tdc_des,buffer+i,1) == 0)
          {
               if (time(NULL) > start+2) return -1;
          }

	  if (verbose) error(MESSAGE, "Got byte 0x%x", (int)buffer[i]);
          if (*buffer == first_return_byte) i++;
        }

	return i;

} /* tdc_get_response() */

/************************************************************************/
/* tdc_identify()							*/
/*									*/
/* the "active" light on the front surface of the controller		*/
/* will flash five times                                    		*/
/************************************************************************/

int tdc_identify(void)
{
	char s[6] ="\x23\x02\x00\x00\x50\x01";

	if(!tdc_status.led_act)
	{
	  return error(MESSAGE,"Warning, LED is  not active");
	}

	write(tdc_des,s, 6); /*send data to port*/

	return NOERROR;
	
} /* tdc_identify() */

/************************************************************************/
/* tdc_home(void)                                           		*/
/*                                                          		*/
/* rotate to home position                                  		*/
/************************************************************************/

int tdc_home(void)
{
	char s[6]="\x43\x04\x01\x00\x50\x01";
	char r[6];
	int num;

	num=write(tdc_des,s, 6); /*send data to port*/

	if(num==6) error(MESSAGE,"Homing the SH rotationg stage");

	if ((num = tdc_get_response(6, 0x44, r)) < 0)
		return error(ERROR,"Timed out in tdc_home().");
	if (num != 6)
		return error(ERROR,"Received too little data in tdc_home().");

	if(r[0] == '\x44' && r[1] == '\x04')
	{
	  error(MESSAGE,"SH rotation stage homed");
	  tdc_status.position = 0;
	  return NOERROR;
	}
	
	return error(ERROR,"SH rotation stage home unsuccessful");

} /* tdc_home() */
	
/************************************************************************/
/* tdc_setvelparams()							*/
/*							    		*/
/* to set the  maximum rotation velocities                  		*/
/* and acceleration                                         		*/
/* minimum velocity is fixed to be 0                        		*/
/************************************************************************/

int tdc_setvelparams(struct s_wfs_tdc_status status)
{

	char *hmaxvel;
	char *haccel;

	/* the 5th byte is 0x50|0x80 */

	char s[20]="\x13\x04\x0E\x00\xD0\x01\x01\x00\x00\x00\x00\x00";
	int num;
	float maxvel = status.max_vel* 134218;           /* scaling factor*/
	float accel = status.accel * 13.744;

	hmaxvel = dec_to_char((long)maxvel,TRUE);
	haccel = dec_to_char((long)accel,TRUE);

	memcpy(s+12,haccel,4);
	memcpy(s+16,hmaxvel,4);

	free(hmaxvel);
	free(haccel);

	num=write(tdc_des,s, 20); /*send data to port*/

	if (num==20)
	{
	  tdc_status.max_vel = status.max_vel;
	  tdc_status.accel = status.accel;
	  return error(MESSAGE,"new SH rotation stage velparams written");
	}
	return error(ERROR, "new SH rotation stage velparams written failed");

}/* tdc_setvelparams() */

/************************************************************************/
/* tdc_getvelparams()                                       		*/
/*						 	    		*/
/* to get the  maximum rotation velocities                  		*/
/* and acceleration                         				*/
/************************************************************************/

int tdc_getvelparams(void)
{
	char s[6] ="\x14\x04\x01\x00\x50\x01";
	char r[20];
	int num=0;

	num=write(tdc_des,s, 6); /*send data to port */
	if(num == 6) error(MESSAGE,"requesting velparams");

	if ((num = tdc_get_response(20, 0x15, r)) < 0)
	    return error(ERROR,"Timed out in tdc_getvelparams().");
	if (num != 20)
	    return error(ERROR,
			"Received too little data in tdc_getvelparams().");

	if(*r == '\x15' && *(r+1) == '\x04')
	{
	  /* maximum velocity */

	  tdc_status.max_vel = char_to_dec(&r[16], 4, TRUE) / 134218.;

	  /*acceleration */

	  tdc_status.accel = char_to_dec(&r[12], 4, TRUE) / 13.744;

	  return error(MESSAGE,"SH rotation stage params have been received");
	}
	return error(ERROR,"SH rotation stage velparams receiving failed");
	
} /* tdc_getvelparams() */

/************************************************************************/
/* tdc_move_completed()                                     		*/
/*							    		*/
/* return if the absolute or relative movement is completed 		*/
/************************************************************************/

int tdc_move_completed(void)
{
	char r[20];
	int num=0;
	
	if ((num = tdc_get_response(20, 0x64, r)) < 0)
		return error(ERROR,"Timed out in tdc_move_comeplte().");
	if (num != 20)
		return error(ERROR,
			"Received too little data in tdc_move_comeplte().");

	if(r[0] == '\x64' && r[1] == '\x04')
	{
	  tdc_get_poscounter(); 
	  return error(MESSAGE,"SH rotation stage movement finished");
	}
	return error(ERROR,"SH rotation stage movement failed");

}/* tdc_move_completed() */

/************************************************************************/
/* tdc_rel_move()                                           		*/
/*							    		*/
/* relative movement                                        		*/
/* step in unit of degree, positive step is clockwise       		*/
/************************************************************************/

int tdc_rel_move(float step)
{
	char *hstep;
	char s[12] = "\x48\x04\x06\x00\xD0\x01\x01\x00";
	int num;

	step *=1919.64;           /*scaling factor*/
	hstep = dec_to_char((long)step,TRUE);

	memcpy(&s[8],hstep,4);

	free(hstep);

	num=write(tdc_des,s,12);

	if(num == 12)
	  error(MESSAGE,"Relative movement has been sent to SH rotation stage");

	return tdc_move_completed();

}/* tdc_rel_move() */

/************************************************************************/
/* tdc_abs_move()                                           		*/
/*							    		*/
/* absolute movement                                        		*/
/* step in unit of degree, positive step is clockwise       		*/
/************************************************************************/

int tdc_abs_move(float step)
{
	char *hstep;
	char s[12] = "\x53\x04\x06\x00\xD0\x01\x01\x00";
	int num;
	
	/* scaling factor */

	step *=1919.64;     
	hstep = dec_to_char((long)step,TRUE);

	memcpy(&s[8],hstep,4);

	free(hstep);

	num=write(tdc_des,s,12);

	if(num == 12)
	  error(MESSAGE,"Absolute movement has been sent to SH rotation stage");

	return tdc_move_completed();

}/* tdc_abs_move*/

/************************************************************************/
/* tdc_move_stopped()                                       		*/
/*							    		*/
/* stop any kind of movement                                		*/
/************************************************************************/

int tdc_move_stopped(void)
{
	char r[20];
	int num=0;

	if ((num = tdc_get_response(20, 0x66, r)) < 0)
		return error(ERROR,"Timed out in tdc_move_stopped().");
	if (num != 20)
		return error(ERROR,
			"Received too little data in tdc_move_stopped().");

	if(r[0] == '\x66' && r[1] == '\x04')
	{
	  tdc_get_poscounter();
	  return error(MESSAGE,"SH rotation stage has been forced to stop");
	}
	return error(ERROR,"SH rotation stage stopping failed");

}/* tdc_move_stopped() */

/************************************************************************/
/* tdc_move_stop()                                          		*/
/*							    		*/
/* stop any kind of movement                                		*/
/************************************************************************/

int tdc_move_stop(void)
{
	char s[6] = "\x65\x04\x01\x01\x50\x01";

	write(tdc_des,s,6);

	return tdc_move_stopped();

}/* tdc_move_stop() */

/************************************************************************/
/* tdc_act_avmodes()                                        		*/
/*							    		*/
/* enable the LED flashing to indicate certain driver states		*/
/* this is the default                                      		*/
/************************************************************************/

int tdc_act_avmodes(void)
{
	char s[10] = "\xB3\x04\x04\x00\xD0\x01\x01\x00\x0B\x00";
	int num=write(tdc_des,s,10);
	
	if(num == 10)
	{
	  tdc_status.led_act = 1;
	  return error(MESSAGE,"SH rotation stage all AVmodes are activated");
	}

	return error(ERROR,"SH rotation stage all AVmodes activation failed");

}/* tdc_act_avmodes() */

/************************************************************************/
/* tdc_deact_avmodes()                                       		*/
/*							     		*/
/* disable the LED flashing to indicate certain driver states		*/
/* this should be set when installed in telescopes           		*/
/************************************************************************/

int tdc_deact_avmodes(void)
{
	char s[10] = "\xB3\x04\x04\x00\xD0\x01\x01\x00\x00\x00";
	int num = write(tdc_des,s,10);

	if(num == 10)
	{
	  tdc_status.led_act = 0;
	  return error(MESSAGE,"SH rotation stage all AVmodes are deactivated");
	}

	return error(ERROR,"SH rotation stage all AVmodes deactivation failed");

} /* tdc_deact_avmodes() */

/************************************************************************/
/* tdc_set_poscounter()                                     		*/
/*                                                          		*/
/* set the current position, unit : degree                  		*/
/************************************************************************/

int tdc_set_poscounter(float pos)
{
	char s[12] = "\x10\x04\x06\x00\xD0\x01\x01\x00";
	char *hpos;
	int num;
	int temp;

	pos *= 1919.64;
	hpos = dec_to_char((long)pos,TRUE);

	memcpy(&s[8],hpos,4);
	free(hpos);

	num=write(tdc_des,s,12);
	
	if(num == 12)
	{
	  temp = (pos/1919.64)/360;
	  tdc_status.position = (pos/1919.64) - 360 * temp ;
	  return error(MESSAGE,
		"SH rotation stage new position set: %ld",pos/1919.64);
	}
	
	return error(MESSAGE,"SH rotation stage new position setting failed");

} /* tdc_set_poscounter() */

/************************************************************************/
/* tdc_get_poscounter()  						*/
/*                                                          		*/
/* get the current position, unit : degree                  		*/
/************************************************************************/

int tdc_get_poscounter(void)
{
	char s[6] = "\x11\x04\x01\x00\x50\x01";
	char r[12];
	int i = 0,num,temp;
	bool startcount = FALSE;
	long pos;
	time_t start = time(NULL);

	num=write(tdc_des,s,6);

	if(num == 6)
	  error(MESSAGE,"SH rotation stage requesting the current position");
	else
	{
	  error(ERROR,"Failed to send command to SH rotation stage.");
	  tdc_status.position = 0.0;
	}

	/* We expect 12 characters to come back */

	while(i < 12)
	{
	  if (time(NULL) > start+2) 
		return error(ERROR,"Timed out in tdc_get_poscounter().");

	  if (!startcount)
	  {
	    while(read(tdc_des,r,1) == 0)
	    {
	        if (time(NULL) > start+2) 
		    return error(ERROR,"Timed out in tdc_get_poscounter().");
	    }
	    if (*r == '\x12')
	    {
		startcount = TRUE;
		i++;
	    }
	    printf("I = %d\n", i);
	  }
	  else
	  {
	    while(read(tdc_des,r+i,1) == 0)
	    {
	        if (time(NULL) > start+2) 
		    return error(ERROR,"Timed out in tdc_get_poscounter().");
	    }
	    i++;
	    printf("I = %d\n", i);
	  }
	}

	if(r[0] == '\x12' && r[1] == '\x04')
	{
	  pos = char_to_dec(&r[8],4,TRUE);
	  temp = (((float)pos)/1919.64) / 360;
	  tdc_status.position = (((float)pos)/1919.64) - 360*temp;
	  return error(MESSAGE,"got SH rotation stage position");
	}

	tdc_status.position = 0.0;
	return error(MESSAGE,"getting SH rotation stage position failed");

} /* tdc_get_poscounter() */
