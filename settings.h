#define INPUTSIZE 31 // 22 + 9 = 31
#define OUTPUTSIZE 18 // 9 + 9 = 18
#define BRAINSIZE 150
#define CONNS 4

#ifndef SETTINGS_H
#define SETTINGS_H

namespace conf {

	// WORLD / WINDOW SETTINGS -------------------------------------
    const int WIDTH = 4200;  //width and height of simulation
    const int HEIGHT = 2600;
    const int WWIDTH = 1000;  //window width and height
    const int WHEIGHT = 900;
    
    const int CZ = 50; //cell size in pixels, for food squares. Should divide well into Width Height

	const bool CLOSED = true; // world is closed and no new agents are added
	
	// BOT PROPERTIES ---------------------------------------------
    const int NUMBOTS=100; //initially, and minimally
    const float BOTRADIUS=10; //for drawing
    const float BOTSPEED= 0.1;
    const float SPIKESPEED= 0.01; //how quickly can attack spike go up?
    const float SPIKEMULT= 2; //essentially the strength of every spike impact
    const float BOOSTSIZEMULT=2; //how much boost do agents get? when boost neuron is on
    const float DIST= 150;		//how far can the eyes see on each bot?
    const float EYE_SENSITIVITY= 2; //how sensitive are the eyes?
    const float BLOOD_SENSITIVITY= 2; //how sensitive are blood sensors?
    const float METAMUTRATE1= 0.002; //what is the change in MUTRATE1 and 2 on reproduction? lol
    const float METAMUTRATE2= 0.05;

	// REPRODUCTION ----------------------------------------------
    const int BABIES=2; //number of babies per agent when they reproduce
    const float REPRATEH=7; //reproduction rate for herbivors
    const float REPRATEC=7; //reproduction rate for carnivors
	const float REP_MIN_HEALTH=.75; // health level required of agent before it can reproduce

	// HEALTH DEDUCTIONS
	const float LOSS_BASE     = 0.00003;
	const float LOSS_SHOUTING = 0.00005;
	const float LOSS_SPEED    = 0.00005;
	const float LOSS_TEMP     = 0.005;
		
	// FOOD SETTINGS -----------------------------------------------
	const int FOOD_MODEL_RAND = 1; // Food Model Options
	const int FOOD_MODEL_GROW = 2; // Food Model Options

	const int   FOOD_MODEL = FOOD_MODEL_RAND; //what kind of food appearance is to be used
    const float FOODGROWTH= 0.0005; //how quickly does food grow on a square. only used with FOOD_MODEL_GROW
    const float FOODINTAKE= 0.00225; //how much does every agent consume?
    const float FOODWASTE= 0.001; //how much food disapears if agent eats?
    const float FOODMAX= 0.5; //how much food per cell can there be at max?
    const int   FOODADDFREQ= 15; //how often does random square get to full food?
	const float FOOD_DEAD = .1; // what percent of FOOD MAX does a dead agent create that is not eaten by carnivores?
	const float FOOD_MEAT_VALUE = 2.0; // percentage that health is transferred to another agent when eaten
		
    const float FOODTRANSFER= 0.001; //how much is transfered between two agents trading food? per iteration
    const float FOOD_SHARING_DISTANCE= 40; //how far away is food shared between bots?

    const float FOOD_DISTRIBUTION_RADIUS=100; //when bot is killed, how far is its body distributed?

}

#endif
