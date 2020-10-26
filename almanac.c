#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>
//=====================Solar===================
int d_o_y()
{
	time_t t;
	t=time(NULL); // get current time.
	struct tm *ptm;
	ptm=localtime(&t);
	int yday = ptm->tm_yday;
	return(yday);
}
double get_offset()
{
    time_t rawtime = time(NULL);
    struct tm *ptm = gmtime(&rawtime);
    time_t gmt = mktime(ptm);
    ptm = localtime(&rawtime);
    time_t offset = rawtime - gmt + (ptm->tm_isdst ? 3600 : 0);
	offset /=60;
    return((int)offset);
}
void min_2_hr(double mins, char *str)
{
	int hr,mn,sc;
	hr=(int)mins/60;
	mins /=60;
	mins = mins - hr;
	mins *=60;
	mn=(int)mins;
	mins = mins - mn;
	mins*=60;
	sc=(int)mins;
	sprintf(str,"%2d:%2d:%2d",hr,mn,sc);
} 
//code from libhdate. By Yaacov Zamir <kzamir@walla.co.il>
void get_utc_sun_time_deg (double latitude, double longitude, double Sun_angle_deg, double *sunrise, double *sunset)
{
	double gama; /* location of sun in yearly cycle in radians */
	double eqtime; /* diffference betwen sun noon and clock noon */
	double decl; /* sun declanation */
	double ha; /* solar hour engle */
	double sunrise_angle = M_PI * Sun_angle_deg/ 180.0; /* sun angle at sunrise/set */
	
	int day_of_year;
	
	/* get the day of year */
	//day_of_year = hdate_get_day_of_year (day, month, year);
	day_of_year = d_o_y();
	/* get radians of sun orbit around erth =) */
	gama = 2.0 * M_PI * ((double)(day_of_year - 1) / 365.0);
	
	/* get the diff betwen suns clock and wall clock in minutes */
	eqtime = 229.18 * (0.000075 + 0.001868 * cos (gama)
		- 0.032077 * sin (gama) - 0.014615 * cos (2.0 * gama)
		- 0.040849 * sin (2.0 * gama));
	
	/* calculate suns declanation at the equater in radians */
	decl = 0.006918 - 0.399912 * cos (gama) + 0.070257 * sin (gama)
		- 0.006758 * cos (2.0 * gama) + 0.000907 * sin (2.0 * gama)
		- 0.002697 * cos (3.0 * gama) + 0.00148 * sin (3.0 * gama);
	
	/* we use radians, ratio is 2pi/360 */
	latitude = M_PI * latitude / 180.0;
	
	/* the sun real time diff from noon at sunset/rise in radians */
	errno = 0;
	ha = acos (cos (sunrise_angle) / (cos (latitude) * cos (decl)) - tan (latitude) * tan (decl));
	
	/* check for too high altitudes and return negative values */
	if (errno == EDOM)
	{
		*sunrise = -720;
		*sunset = -720;
		
		return;
	}
	
	/* we use minutes, ratio is 1440min/2pi */
	ha = 720.0 * ha / M_PI;
	
	/* get sunset/rise times in utc wall clock in minutes from 00:00 time */
	*sunrise = /*(int)*/(720.0 - 4.0 * longitude - ha - eqtime);
	*sunset = /*(int)*/(720.0 - 4.0 * longitude + ha - eqtime);
		
	return;
}
//========================Luna====================
typedef struct Luna
{
	double age; // age since newmoon
	double distance; //moon distance in earth radii
	double lat_m;	// moon latitude about eccliptic
	double lon_m;	// moon longitude on eccliptic
	char   Phase[20];
}Luna;
// days in month
uint16_t dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};
// range 0 to 1
double normalize(double v)
{
	v =  v - floor(v);
	if(v<0)
		v = v+1;
	return(v);
}

// round 2 places
double round2(double x)
{
	return (round( 100*x )/100.0 );
}

int is_leapyear( uint16_t y)
{
	float p,q,r;
	p = floor(y - 4*floor( y/4 ) );
	q = floor(y - 100*floor(y/100));
	r = floor(y - 400*floor(y/400));
	if(p == 0) 			// might be a leap year
	{
		if(q == 0 && r != 0)
		{
			return (0);
		}
		else
			return (1);
	}
	return (0);
}
// check if the day of mont is correct
int is_day_of_month( int y, int m, int d ) 
{
	if( m != 2 ) 
	{
		if( 1 <= d && d <= dom[m-1] ) 
		{
			return 1;
        } 
        else 
        {
			return 0;
        }
    }
    uint16_t feb = dom[1];
    if(is_leapyear( y )) 
		feb += 1;                // is leap year
    if(1 <=d && d<=feb)
		return 1;
    return 0;
}

Luna moon_position( uint16_t yr, uint16_t mn, uint16_t day)
{
	Luna l;
	//find julian day  of yr mh and day 
	double YY = yr - floor( ( 12 - mn ) / 10 );       
    double MM = mn + 9; 
    if( MM >= 12 ) 
		MM = MM - 12;
        
    double K1 = floor( 365.25 * ( YY + 4712 ) );
    double K2 = floor( 30.6 * MM + 0.5 );
    double K3 = floor( floor( ( YY / 100 ) + 49 ) * 0.75 ) - 38;
        
    double JD = K1 + K2 + day + 59;                  // for dates in Julian calendar
    if( JD > 2299160 ) 
		JD = JD - K3;        // for Gregorian calendar
    double IP = normalize((JD - 2451550.1 ) / 29.530588853 );
    l.age = IP*29.53; // moon's age
    if(l.age <  1.84566) 
		strcpy(l.Phase,"A new moon");
	else if(l.age < 5.53699) 
		strcpy(l.Phase,"An evening crescent");
	else if( l.age <  9.22831 ) 
		strcpy(l.Phase , "A first quarter");
    else if( l.age < 12.91963 )
		strcpy(l.Phase , "A waxing gibbous");
    else if( l.age < 16.61096 ) 
		strcpy(l.Phase , "A full moon");
	else if( l.age < 20.30228 ) 
		strcpy(l.Phase ,"A waning gibbous");
    else if( l.age < 23.99361 )
		strcpy(l.Phase, "A Last quarter");
    else if( l.age < 27.68493 ) 
		strcpy(l.Phase, "A Morning crescent");
    else
		strcpy(l.Phase, "A new moon");   
	
	IP = IP*2*M_PI;   // Convert phase to radians
    // calculate moon's distance
    double DP = 2*M_PI*normalize((JD - 2451562.2 ) / 27.55454988 );
    l.distance = 60.4 - 3.3*cos( DP ) - 0.6*cos( 2*IP - DP ) - 0.5*cos( 2*IP );

        // calculate moon's ecliptic latitude
    double NP = 2*M_PI*normalize(( JD - 2451565.2 ) / 27.212220817 );
    l.lat_m = 5.1*sin( NP );

    // calculate moon's ecliptic longitude
    double RP = normalize( ( JD - 2451555.8 ) / 27.321582241 );
    l.lon_m = 360*RP + 6.3*sin( DP ) + 1.3*sin( 2*IP - DP ) + 0.7*sin( 2*IP );
    // correcting if longitude is not greater than 360!
    if ( l.lon_m > 360 ) 
		l.lon_m = l.lon_m - 360;
	return (l); 
}
//=================Output=================
int main(int argc, char *argv[])
{
	printf("Sunrise Sunset time, Moon Age, Phase, calculator for a location.\n");
	printf("The values are to be used for hobby use only\n\n");
	double sunrise, sunset;
	if(argc < 3)
	{
		printf("Usage: %s <latitude> <longitude>\n", argv[0]);
		printf("Latitude and Longitudes are in degrees and fractions\n");
		printf("North latitudes are (+) south are (-)\n");
		printf("East longitudes are (+) and west are (-)\n");
		printf("Example: 85 degree 52 minutes East longitude is 85.8666\n");
		exit(1);
	}
	double latitude = atof(argv[1]);
	double longitude = atof(argv[2]);
	get_utc_sun_time_deg(latitude,longitude,90.833,&sunrise,&sunset);
	double offset = get_offset() ;//longitude *4;
	//double offset = longitude *4;
	sunrise += offset; // UTC to IST offset
	//sunrise /= 60; //hours IST
	sunset += offset;
	//sunset /= 60; // hours IST
	double noon = (sunrise+sunset)/2;
	
	//printf("sunrise = %f noon = %f sunset = %f\n",sunrise,noon,sunset);
	char srstr[10],ststr[10],nnstr[10];
	min_2_hr(sunrise,srstr);
	min_2_hr(noon,nnstr);
	min_2_hr(sunset,ststr);
	printf("Sunrise %s Noon %s Sunset %s\n",srstr,nnstr,ststr);
//===========Lunar details==================
	Luna lun;
	time_t t;
	t=time(NULL); // get current time.
	struct tm *ptm;
	ptm=localtime(&t);
	lun = moon_position(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
	printf("Moon age = %f Phase = %s Lat= %f Lon = %f\n",lun.age,lun.Phase,lun.lat_m,lun.lon_m);
	return(0);
}
