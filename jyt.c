/*---------------------------------------------------------------------------------------
 * Testprogram:  Port M LED on/off
 * 	             Print "MP-Labor" on console
 *                                                                        Prosch 3/2017
 *---------------------------------------------------------------------------------------
*/

#define LED_0  	{ 0x7F, 0x41, 0x41, 0x41, 0x7F }
#define LED_1   { 0x00, 0x00, 0x00, 0x00, 0x7F }
#define LED_2   { 0x4F, 0x49, 0x49, 0x49, 0x79 }
#define LED_3 	{ 0x49, 0x49, 0x49, 0x49, 0x7F }
#define LED_4  	{ 0x78, 0x08, 0x08, 0x08, 0x7F }
#define LED_5 	{ 0x79, 0x49, 0x49, 0x49, 0x4F }
#define LED_6   { 0x7F, 0x49, 0x49, 0x49, 0x4F }
#define LED_7	{ 0x40, 0x40, 0x40, 0x40, 0x7F }
#define LED_8	{ 0x7F, 0x49, 0x49, 0x49, 0x7F }
#define LED_9	{ 0x79, 0x49, 0x49, 0x49, 0x7F }

#define LED_C     { 0x0F, 0x09, 0x09, 0x09, 0x09 }
#define LED_M     { 0x0F, 0x08, 0x0F, 0x08, 0x0F }


#include "tm4c1294ncpdt.h"
#include "stdio.h"

void warte (unsigned long zeit){				// Wartefunktion
	unsigned long j ;
	for ( j = 0 ; j < zeit ; j++ ) ;
}

///Ultraschallmessung
unsigned long ultraschallMessung()
{
	GPIO_PORTD_AHB_DATA_R = 0x01;                     // Trigger ultraschallsensor
	warte(200);//0
	TIMER0_TAV_R = 0;								//Reset time
	GPIO_PORTD_AHB_DATA_R = 0x00;                     //Start Messung
	/*while((GPIO_PORTD_AHB_DATA_R & 0x02) == 0);        // noch kein Echo
	printf("Hallo");
		TIMER0_CTL_R |= 0x01;                        // Start Timer
	while((GPIO_PORTD_AHB_DATA_R & 0x02) == 2);        // Echo
		TIMER0_ICR_R |= (1 << 2);                                // clear capture event flag
		//TIMER0_CTL_R &= ~0x01;                       // Stop Timer
	timer = TIMER1_TAV_R;                              // Resultat Timer speichern
	strecke = (timer*335*100)/(2*16000000);            // Strcke in cm
	printf("Entfernung:%f cm \n",strecke);             //Ausgabe
	TIMER0_TAV_R = 0x0;                               // Speicher zurücksetzen
	*/
	while((GPIO_PORTD_AHB_DATA_R & 0x02) == 0);		// noch kein Echo
	TIMER0_CTL_R |= 0x01;							// Start Timer
	while((GPIO_PORTD_AHB_DATA_R & 0x02) == 2);		// Echo
	TIMER0_CTL_R &= 0x00;							// Stop Timer
	return TIMER0_TAV_R;							// Resultat Timer speichern
}

void task2()
{
	SYSCTL_RCGCGPIO_R |= (1<<3);				// clocke enable port D
	while ((SYSCTL_PRGPIO_R & (1<<3))==0);            // check port D ready

	// Konfiguration PORT

	GPIO_PORTD_AHB_DEN_R = 0x03;                     // pins PD0-PD1 enable
	GPIO_PORTD_AHB_DIR_R = 0x01;                     // set output direction for Pin 0
	GPIO_PORTD_AHB_DATA_R = 0x01;                  // PD0 set High

	//Konfiguration Timer
	SYSCTL_RCGCTIMER_R |= 0x01;                   // sytemtakt auf Timer 1
	while (!(SYSCTL_PRTIMER_R & 0x01));           // warte auf aktivierung
	TIMER0_CTL_R &= ~0x01;                     //Timer 1 deaktivieren
	TIMER0_CFG_R = 0x00;                         // 32 bits mode
	TIMER0_TAMR_R = 0x18;                       // hochzaehlend
	TIMER0_ICR_R |= 0x001F;						// clear all flags Timer0A

	double timer = 0;
	double strecke = 0;
	while(1){
		timer = ultraschallMessung();
		strecke = (timer*335*100)/(2*16000000);			// Strcke in cm
		printf("Entfernung:%f cm \n",strecke);			//Ausgabe
		warte(1000000);
	}
}


///Aufgabe1
void task1()
{
	// Clock Ports aktivieren		Vorlesung GPIO Seite 30
	SYSCTL_RCGCGPIO_R |= (1<<9)|(1<<10);		// enable clock port K + port L

	// Stabilisieren lassen
	while((SYSCTL_PRGPIO_R & ((1<<9)|(1<<10))) == 0); // clock when port D is available, wait for port

	// Ports aktivieren
	GPIO_PORTK_DEN_R = 0xFF; 					// PK(7:0)
	GPIO_PORTL_DEN_R = 0x01; 					// PL(0)


	// Ports Direction
	GPIO_PORTK_DIR_R = 0xFF;						// PK(7:0) output - LEDs
	GPIO_PORTL_DIR_R = 0x00;						// PL(0) input - R/L


	// Deklarationen
	unsigned char RL_alt = 0;					// Altes (Hight/Low)
	unsigned char RL_neu;			           	// Neues (Hight/Low)
	unsigned int Einschaltzeit=160;			// Einschaltzeit der LEDs 1ms
	unsigned int BisEnde=9631;				// Einschaltzeit der LEDs 60ms

	while(1){
		RL_neu = (GPIO_PORTL_DATA_R == 0x00);                   // wenn Flanke
		if(RL_alt != RL_neu){
			GPIO_PORTK_DATA_R = 0xFF; 			// LEDs (7:0) einschalten
			warte(Einschaltzeit);
			GPIO_PORTK_DATA_R = 0x00; 			// LEDs (7:0) ausschalten
			warte(BisEnde-Einschaltzeit*2);
			GPIO_PORTK_DATA_R = 0xFF; 			// LEDs (7:0) einschalten
			warte(Einschaltzeit);
			GPIO_PORTK_DATA_R = 0x00; 			// LEDs (7:0) ausschalten
		}
		RL_alt = RL_neu;
	}
}


int task3(){
	// Clock Ports aktivieren		Vorlesung GPIO Seite 30
	SYSCTL_RCGCGPIO_R |= (1<<9)|(1<<10);		// enable clock port K + port L

	// Stabilisieren lassen
	while((SYSCTL_PRGPIO_R & ((1<<9)|(1<<10))) == 0); // clock when port D is available, wait for port

	// Ports aktivieren
	GPIO_PORTK_DEN_R = 0xFF; 					// PK(7:0)
	GPIO_PORTL_DEN_R = 0x01; 					// PL(0)


	// Ports Direction
	GPIO_PORTK_DIR_R = 0xFF;						// PK(7:0) output - LEDs
	GPIO_PORTL_DIR_R = 0x00;						// PL(0) input - R/L


	// Deklarationen
	unsigned char RL_alt = 0;					// Altes (Hight/Low)
	unsigned char RL_neu;			           	// Neues (Hight/Low)
	unsigned int Einschaltzeit=160;			// Einschaltzeit der LEDs 1ms
	unsigned int BisEnde=95310;				// Einschaltzeit der LEDs 60ms

	//Echo
	SYSCTL_RCGCGPIO_R |= (1<<3);				// clocke enable port D
	while ((SYSCTL_PRGPIO_R & (1<<3))==0);            // check port D ready

	// Konfiguration PORT

	GPIO_PORTD_AHB_DEN_R = 0x03;                     // pins PD0-PD1 enable
	GPIO_PORTD_AHB_DIR_R = 0x01;                     // set output direction for Pin 0
	GPIO_PORTD_AHB_DATA_R = 0x01;                  // PD0 set High

	//Konfiguration Timer
	SYSCTL_RCGCTIMER_R |= 0x01;                   // sytemtakt auf Timer 1
	while (!(SYSCTL_PRTIMER_R & 0x01));           // warte auf aktivierung
	TIMER0_CTL_R &= ~0x01;                     //Timer 1 deaktivieren
	TIMER0_CFG_R = 0x00;                         // 32 bits mode
	TIMER0_TAMR_R = 0x18;                       // hochzaehlend
	TIMER0_ICR_R |= 0x001F;						// clear all flags Timer0A

	//Konfiguration Timer
	SYSCTL_RCGCTIMER_R |= 0x02;                   // sytemtakt auf Timer 1
	while (!(SYSCTL_PRTIMER_R & 0x02));           // warte auf aktivierung
	TIMER1_CTL_R &= ~0x01;                     //Timer 1 deaktivieren
	TIMER1_CFG_R = 0x04;                         // 2x16 bits mode
	TIMER1_TAMR_R = 0x01;                       // hochzaehlend
	TIMER1_TAPR_R = 14-1;						//prescaler
	TIMER1_TAILR_R = (73846-1)/2;						//
	TIMER1_TAR_R = 0xFFFF;


	TIMER1_ICR_R |= 0x001F;						// clear all flags Timer0A

	double timer = 0;
	double strecke = 0;
	while(1){

		RL_neu = (GPIO_PORTL_DATA_R == 0x00);                   // wenn Flanke
		if(RL_alt != RL_neu && !RL_neu){
			if(TIMER1_CTL_R & 0x1 == 0)
			{
				printf("An");
				GPIO_PORTK_DATA_R = 0xFF;
				TIMER1_CTL_R = 0x1;
			}
			//int waitNumber = BisEnde*(strecke/150);
			//int scaler = BisEnde/5;
			//GPIO_PORTK_DATA_R = 0xFF; 			// LEDs (7:0) einschalten
			//warte(Einschaltzeit);
			//GPIO_PORTK_DATA_R = 0x0F; 			// LEDs (7:0) ausschalten
			//warte(waitNumber);

			unsigned long ses = TIMER1_TAV_R;

			if(TIMER1_TAV_R == 0)
			{
				printf("Aus");
				GPIO_PORTK_DATA_R = 0x00; 			// LEDs (7:0) ausschalten
			}
		}
		else if(RL_alt != RL_neu && RL_neu){
			timer = ultraschallMessung();
			strecke = (timer*335*100)/(2*16000000);			// Strcke in cm
		}
		RL_alt = RL_neu;
	}
}

int task4(){
	// Clock Ports aktivieren		Vorlesung GPIO Seite 30
	SYSCTL_RCGCGPIO_R |= (1<<9)|(1<<10);		// enable clock port K + port L

	// Stabilisieren lassen
	while((SYSCTL_PRGPIO_R & ((1<<9)|(1<<10))) == 0); // clock when port D is available, wait for port

	// Ports aktivieren
	GPIO_PORTK_DEN_R = 0xFF; 					// PK(7:0)
	GPIO_PORTL_DEN_R = 0x01; 					// PL(0)


	// Ports Direction
	GPIO_PORTK_DIR_R = 0xFF;						// PK(7:0) output - LEDs
	GPIO_PORTL_DIR_R = 0x00;						// PL(0) input - R/L

	unsigned char RL_alt = 0;					// Altes (Hight/Low)
	unsigned char RL_neu;			           	// Neues (Hight/Low)
	unsigned int BisEnde = 95310;
	unsigned int matrixXDots = 80;
	unsigned int matrixXTime = BisEnde/matrixXDots;

	//unsigned char arrD2[5] = LED_0;
	//unsigned char arrD1[5] = LED_0;
	unsigned char arrD[10][5] = {LED_0,LED_1,LED_2,LED_3,LED_4,LED_5,LED_6,LED_7,LED_8,LED_9};
	unsigned char arrC[5] = LED_C;
	unsigned char arrM[5] = LED_M;

	//Echo
	SYSCTL_RCGCGPIO_R |= (1<<3);				// clocke enable port D
	while ((SYSCTL_PRGPIO_R & (1<<3))==0);            // check port D ready

	// Konfiguration PORT

	GPIO_PORTD_AHB_DEN_R = 0x03;                     // pins PD0-PD1 enable
	GPIO_PORTD_AHB_DIR_R = 0x01;                     // set output direction for Pin 0
	GPIO_PORTD_AHB_DATA_R = 0x01;                  // PD0 set High

	//Konfiguration Timer
	SYSCTL_RCGCTIMER_R |= 0x01;                   // sytemtakt auf Timer 1
	while (!(SYSCTL_PRTIMER_R & 0x01));           // warte auf aktivierung
	TIMER0_CTL_R &= ~0x01;                     //Timer 1 deaktivieren
	TIMER0_CFG_R = 0x00;                         // 32 bits mode
	TIMER0_TAMR_R = 0x18;                       // hochzaehlend
	TIMER0_ICR_R |= 0x001F;						// clear all flags Timer0A

	double timer = 0;
	int strecke = 0;

	while(1){
		RL_neu = (GPIO_PORTL_DATA_R == 0x00);               // wenn Flanke
		if(RL_alt != RL_neu && !RL_neu){					// und ganz links

			int d2 = strecke/100;
			int d1 = (strecke-d2*100)/10;
			int d0 = strecke-d2*100-d1*10;

			int x=0;
			for(x=0;x<matrixXDots;x++)
			{
				if(x > 27 && x < 33)
				{
					GPIO_PORTK_DATA_R = arrD[d2][x-28];
					warte(matrixXTime);
					GPIO_PORTK_DATA_R = 0x00;
				}
				else if(x > 33 && x < 39)
				{
					GPIO_PORTK_DATA_R = arrD[d1][x-34];
					warte(matrixXTime);
					GPIO_PORTK_DATA_R = 0x00;
				}
				else if(x > 39 && x < 45)
				{
					GPIO_PORTK_DATA_R = arrD[d0][x-40];
					warte(matrixXTime);
					GPIO_PORTK_DATA_R = 0x00;
				}
				else if(x > 45 && x < 51)
				{
					GPIO_PORTK_DATA_R = arrC[x-46];
					warte(matrixXTime);
					GPIO_PORTK_DATA_R = 0x00;
				}
				else if(x > 51 && x < 57)
				{
					GPIO_PORTK_DATA_R = arrM[x-52];
					warte(matrixXTime);
					GPIO_PORTK_DATA_R = 0x00;
				}
				else
				{
					warte(matrixXTime);
				}
			}
		}
		else if(RL_alt != RL_neu && RL_neu){
			timer = ultraschallMessung();
			strecke = (timer*335*100)/(2*16000000);			// Strcke in cm
		}
		RL_alt = RL_neu;
	}
}

int main(void){
	task4();
}
