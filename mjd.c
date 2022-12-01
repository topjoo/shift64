
/***********************************************************************************
 * @file    : mjd.c
 * @brief   : Modified Julian Date Conversion
 *           
 *           
 * @version : 2.0
 * @date    : 2014/06/30
 * @author  : Yoon-Hwan Joo (tp.joo@daum.net or toppy_joo@naver.com) 
 ************************************************************************************/

#include <stdio.h>

#if MODIFIED_JULIAN_DATE /* 2014.07.04 */
////////////////////////////////////////////
/// Modified Julian Date
/// 2013.09.14 by top.joo
////////////////////////////////////////////

/* -------------------------------------------------------- */
/* --  2013.09.14 : KBS EWS (Emergency Warning Service) --- */
/* -------------------------------------------------------- */
/* --- Modified Julian Date ------ */
typedef struct {
	int  m_year;
	int  m_month;
	int  m_day;
	int  m_hour;
	int  m_mins;
	int  m_secs;
	int  m_millis;
} mjd_timestamp;

#define TDMB_REFER_MJD_VALUE  			(15020)  /* Reference TDMB : 1900.1.1 (15020.0) */


/// ---------- Modified JD Examles ----------------------
/// *** -> 50000.000000  ---> [ 1995, 10, 10, 0, 0, 0 0 ] 
/// *** -> 56551.338982  ---> [ 2013, 9, 16, 8, 8, 8 6 ]  
/// *** -> 00000.000000  ---> [ 1858, 11, 17, 0, 0, 0 0 ] 
/// *** -> 40000.000000  ---> [ 1968, 5, 24, 0, 0, 0 0 ]
/// *** -> 47892.000000  ---> [ 1990, 1, 1, 0, 0, 0 0 ]  
/// *** -> 48000.000000  ---> [ 1990, 4, 19, 0, 0, 0 0 ]  
/// *** -> 15020.000000  ---> [ 1900, 1, 1, 0, 0, 0 0 ]  
/// *** -> 15021.000000  ---> [ 1900, 1, 2, 0, 0, 0 0 ]  
/// --------------------------------------------------

#define MJD_PARAM 			(2400000.5)

/// New function at 2013.09.14
/// ---- 2013.09.14
/// ---- Modified Julian Date ------
double Convert2MJD(mjd_timestamp YYMMDD_Time) 
{
	int y = YYMMDD_Time.m_year;
	int m = YYMMDD_Time.m_month;
	double d = (double) YYMMDD_Time.m_day;

	d = d + ((YYMMDD_Time.m_hour / 24.0) +
	         (YYMMDD_Time.m_mins / 1440.0) +
	         (YYMMDD_Time.m_secs / 86400.0) +
	         (YYMMDD_Time.m_millis / 86400000.0));
	
	if (m == 1 || m == 2) {
		y--;
		m = m + 12;
	}

	double a = floor(y / 100.0);
	double b = 2 - a + floor(a / 4.0);

	return (floor(365.25 * (y + 4716.0)) +
			floor(30.6001 * (m + 1)) + d + b - 1524.5) 
			- 2400000.5;  // for Julian Day omit the 2400000.5 term
}



/// ----------------------------------------------------------------------
/// Converts an Modified Julian Day Number (double) to an integer array representing
/// a timestamp (year,month,day,hours,mins,secs,millis).
/// ----------------------------------------------------------------------
mjd_timestamp Convert2Timestamp(double mjd) 
{
	mjd_timestamp ymd_hms;
	int a, b, c, d, e, z;

	/* if a JDN is passed as argument, omit the 2400000.5 term */
	double jd = mjd + 2400000.5 + 0.5;  
	double f, x;

	z = (int)floor(jd);
	f = jd - z;

	if (z >= 2299161) 
	{
		int alpha = (int)floor((z - 1867216.25) / 36524.25);
		a = z + 1 + alpha - (int)floor(alpha / 4.0);
	} 
	else 
	{
		a = z;
	}

	b = a + 1524;
	c = (int) floor((b - 122.1) / 365.25);
	d = (int) floor(365.25 * c);
	e = (int) floor((b - d) / 30.6001);

	/// Day --
	ymd_hms.m_day   = b - d - (int) floor(30.6001 * e);

	/// Month --
	ymd_hms.m_month = (e < 14) ? (e - 1) : (e - 13);

	/// Year --
	ymd_hms.m_year  = (ymd_hms.m_month > 2) ? (c - 4716) : (c - 4715);


	/// Hour --
	f = f * 24.0;
	x = floor(f);
	ymd_hms.m_hour = (int)x;
	f = f - x;

	/// Minutes --
	f = f * 60.0;
	x = floor(f);
	ymd_hms.m_mins = (int)x;
	f = f - x;

	/// seconds --
	f = f * 60.0;
	x = floor(f);
	ymd_hms.m_secs = (int)x;
	f = f - x;

	/// milli-sec ---
	f = f * 1000.0;
	x = floor(f);
	ymd_hms.m_millis= (int)x;
	f = f - x;

    return ymd_hms;
}


#define RADIX 			10 /// Decial,  2:binary

#endif /// MODIFIED_JULIAN_DATE

