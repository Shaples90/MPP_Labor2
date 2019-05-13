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


int main(void){
	task1();
}