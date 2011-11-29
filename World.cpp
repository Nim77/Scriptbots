#include "World.h"
#include <stdio.h>
#include <ctime>

#include "settings.h"
#include "helpers.h"
#include "vmath.h"
#include <stdio.h>

using namespace std;

World::World() :
        modcounter(0),
        current_epoch(0),
        idcounter(0),
        FW(conf::WIDTH/conf::CZ),
        FH(conf::HEIGHT/conf::CZ)
{
    addRandomBots(conf::NUMBOTS);

	
    //inititalize food layer	
    srand(time(0));
	double rand1; // store temp random float to save randf() call
	
    for (int x=0;x<FW;x++) {
       for (int y=0;y<FH;y++) {

		   rand1 = randf(0,1);
    	   if(rand1 > .5)
		   {
			   food[x][y] = rand1 * conf::FOODMAX;
		   }
       }
    }
 // Setting temperature of the world
    for(int i=0;i<conf::WIDTH/2;i++)
      {
        for(int j=0;j<conf::HEIGHT/2;j++)
         {
           temp[i][j]=0.2;
         }
      }


	// Decide if world if closed based on settings.h
	CLOSED = conf::CLOSED;
}


void World::update()
{
	int i; // counter var used throughout for counting entire agent amount
	
    modcounter++;

    //Process periodic events --------------------------------------------------------
    //Age goes up!
    if (modcounter%100==0) {
        for (i=0;i<agents.size();i++) {
            agents[i].age+= 1;    //agents age...
        }
    }

    if (modcounter%1000==0)
    	writeReport();

    if (modcounter>=10000) {
        modcounter=0;
        current_epoch++;
    }

	// What kind of food method are we using?
	if(conf::FOOD_MODEL == conf::FOOD_MODEL_GROW)
	{
		//GROW food enviroment model
		if (modcounter%conf::FOODADDFREQ==0) {
			for (int x=0; x<FW; ++x) {
				for (int y=0; y<FH; ++y) {
					//only grow if not dead
					if(food[x][y] > 0) {

						//Grow current square
						growFood(x,y);

						//Grow surrounding squares sometimes and only if well grown
						if(randf(0,food[x][y]) > .1){
							//Spread to surrounding squares
							growFood(x+1,y-1);
							growFood(x+1,y);
							growFood(x+1,y+1);
							growFood(x-1,y-1);
							growFood(x-1,y);
							growFood(x-1,y+1);
							growFood(x,y-1);
							growFood(x,y+1);
						}
					}
				}
			}
		}
    }
	else
	{
		//Add Random food model - default
		if (modcounter%conf::FOODADDFREQ==0) {
			fx=randi(0,FW);
			fy=randi(0,FH);
			food[fx][fy]= conf::FOODMAX;
		}
	}
	
	// general settings loop
    for(i=0;i<agents.size();i++){

		// says that agent was not hit this turn
        agents[i].spiked= false; 

		// process indicator used in drawing
        if(agents[i].indicator > 0)
			agents[i].indicator -= 1;
    }

    //give input to every agent. Sets in[] array
    setInputs();

    //brains tick. computes in[] -> out[]
    brainsTick();

    //read output and process consequences of bots on environment. requires out[]
    processOutputs();

    float healthloss; // amount of health lost
	float dd, discomfort; // temperature preference vars
	int numaround, j; // used for dead agents
	float d, agemult; //used for dead agents
	
    //process bots health
    for (i=0;i<agents.size();i++) {
        healthloss = conf::LOSS_BASE; // base amount of health lost every turn for being alive



		// remove health based on wheel speed
		if(agents[i].boost) { // is using boost			
			healthloss +=
				conf::LOSS_SPEED * conf::BOTSPEED * (abs(agents[i].w1) + abs(agents[i].w2))
				* conf::BOOSTSIZEMULT * agents[i].boost;
		} else { // no boost
			healthloss +=
				conf::LOSS_SPEED * conf::BOTSPEED * (abs(agents[i].w1) + abs(agents[i].w2));
		}
		
		// shouting costs energy.
		healthloss += conf::LOSS_SHOUTING*agents[i].soundmul;

                 // less health loss if bots move with other agents
                if(agents[i].in[9]<=0.1) // found from the values of the sensor is in generally
               { agents[i].health +=0.000001; //giving extra health
	      }

		//process temperature preferences
        //calculate temperature at the agents spot. (based on distance from equator)
       // dd = 2.0*abs(agents[i].pos.x/conf::WIDTH - 0.5);
        //discomfort = abs(dd-agents[i].temperature_preference);
        //discomfort = discomfort*discomfort;
        //if (discomfort<0.08)
	//		discomfort=0;
        //healthloss +=conf::LOSS_TEMP*discomfort;

		//temperature setting of world
                dd = 2.0*abs(agents[i].pos.x/conf::WIDTH - 0.5);
                
              if(agents[i].pos.x>=conf::WIDTH/2 && agents[i].pos.y>=conf::HEIGHT/2)
                {
                  healthloss=0.0005;
                 }
               if(agents[i].pos.x<=conf::WIDTH/2 && agents[i].pos.y>=conf::HEIGHT/2)
                {
                  healthloss=0.0005;
                 } if(agents[i].pos.x>=conf::WIDTH/2 && agents[i].pos.y>=conf::HEIGHT/2)
                {
                  healthloss=0.0005;
                 } if(agents[i].pos.x>=conf::WIDTH/2 && agents[i].pos.y>=conf::HEIGHT/2)
                {
                  healthloss=0.0005;
                 }
		// apply the health changes
        agents[i].health -= healthloss;

		//------------------------------------------------------------------------------------------
		
		//remove dead agents and distribute food

        //if this agent was spiked this round as well (i.e. killed). This will make it so that
        //natural deaths can't be capitalized on. I feel I must do this or otherwise agents
        //will sit on spot and wait for things to die around them. They must do work!
        if (agents[i].health<=0 && agents[i].spiked) { 
        
            //distribute its food. It will be erased soon
            //first figure out how many are around, to distribute this evenly
            numaround=0;
            for (j=0;j<agents.size();j++) {
				//only carnivores get food. not same agent as dying
                if (agents[j].herbivore < .1 && agents[j].health > 0) {
					
                    d= (agents[i].pos-agents[j].pos).length();
                    if (d<conf::FOOD_DISTRIBUTION_RADIUS) {
                        numaround++;
                    }
                }
            }
            
            //young killed agents should give very little resources
            //at age 5, they mature and give full. This can also help prevent
            //agents eating their young right away
            agemult= 1.0;
            if(agents[i].age<5)
				agemult= agents[i].age*0.2;
            
            if (numaround>0) {
                //distribute its food evenly
                for (j=0;j<agents.size();j++) {
					//only carnivores get food. not same agent as dying					
					if (agents[j].herbivore < .1 && agents[j].health > 0) {

						d= (agents[i].pos-agents[j].pos).length();
                        if (d<conf::FOOD_DISTRIBUTION_RADIUS) {
							// add to agent's health
							/*  percent_carnivore = 1-agents[j].herbivore
								coefficient = 5
								numaround = # of other agents within vicinity
								agemult = 1 if agent is older than 4
								health += percent_carnivore ^ 2 * agemult * 5
								          -----------------------------------
									               numaround ^ 1.25
							 */
							agents[j].health +=
								5*(1-agents[j].herbivore)*(1-agents[j].herbivore)/
								pow(numaround,1.25)*agemult;
							
							// make this bot reproduce sooner
                            agents[j].repcounter -= 6*(1-agents[j].herbivore)*
								(1-agents[j].herbivore)/pow(numaround,1.25)*agemult;

                            if (agents[j].health>2)
								agents[j].health=2; //cap it!
							
                            agents[j].initEvent(30,1,1,1); //white means they ate! nice
                        }
                    }
                }
            }else{
            	//if no agents are around to eat it, it becomes regular food
            	food[(int) agents[i].pos.x/conf::CZ][(int) agents[i].pos.y/conf::CZ]
					= conf::FOOD_DEAD*conf::FOODMAX; // since it was dying it is not much food
            }

        }

    }

	// Delete dead agents
    vector<Agent>::iterator iter= agents.begin();

    while (iter != agents.end()) {
        if (iter->health <=0) {
            iter= agents.erase(iter);
        } else {
            ++iter;
        }
    }

    // Handle reproduction
    for (i=0;i<agents.size();i++) {
        if (agents[i].repcounter<0 && agents[i].health>conf::REP_MIN_HEALTH &&
			modcounter%15==0 && randf(0,1)<0.1) {
			//agent is healthy (REP_MIN_HEALTH) and is ready to reproduce.
			//Also inject a bit non-determinism

			//the parent splits it health evenly with all of its babies
            agents[i].health -= agents[i].health / (conf::BABIES + 1);

			//add conf::BABIES new agents to agents[]
            reproduce(i, agents[i].MUTRATE1, agents[i].MUTRATE2);
			
            agents[i].repcounter =
				agents[i].herbivore*randf(conf::REPRATEH-0.1,conf::REPRATEH+0.1) +
				(1-agents[i].herbivore)*randf(conf::REPRATEC-0.1,conf::REPRATEC+0.1);
        }
    }

    //add new agents, if environment isn't closed
    if (!CLOSED) {
        //make sure environment is always populated with at least NUMBOTS bots
        if (agents.size()<conf::NUMBOTS
           ) {
            //add new agent
            addRandomBots(1);
        }
        if (modcounter%100==0) {
            if (randf(0,1)<0.5){
                addRandomBots(1); //every now and then add random bots in
            }else{
                addNewByCrossover(); //or by crossover
			}
        }
    }


}
//Grow food around square
void World::growFood(int x, int y)
{
	//check if food square is inside the world
	if(food[x][y] < conf::FOODMAX && x >= 0 && x < FW && y >= 0 && y <FH)
		food[x][y] += conf::FOODGROWTH;
}

void World::setInputs()
{
    //P1 R1 G1 B1 FOOD P2 R2 G2 B2 SOUND SMELL HEALTH P3 R3 G3 B3 CLOCK1 CLOCK 2 HEARING     BLOOD_SENSOR   TEMPERATURE_SENSOR  4,9,10,18,
    //0   1  2  3  4   5   6  7 8   9     10     11   12 13 14 15 16       17      18           19                 20

    float PI8=M_PI/8/2; //pi/8/2
    float PI38= 3*PI8; //3pi/8/2
    for (int i=0;i<agents.size();i++) {
        Agent* a= &agents[i];

        //HEALTH
        a->in[11]= cap(a->health/2); //divide by 2 since health is in [0,2]

        //FOOD
        int cx= (int) a->pos.x/conf::CZ;
        int cy= (int) a->pos.y/conf::CZ;
        a->in[4]= food[cx][cy]/conf::FOODMAX;

        //SOUND SMELL EYES
        float p1,r1,g1,b1,p2,r2,g2,b2,p3,r3,g3,b3;
        p1=0;
        r1=0;
        g1=0;
        b1=0;
        p2=0;
        r2=0;
        g2=0;
        b2=0;
        p3=0;
        r3=0;
        g3=0;
        b3=0;
        float soaccum=0;
        float smaccum=0;
        float hearaccum=0;

        //BLOOD ESTIMATOR
        float blood= 0;

        //SMELL SOUND EYES
        for (int j=0;j<agents.size();j++) {
            if (i==j) continue;
            Agent* a2= &agents[j];

			// Do manhattan-distance estimation
            if (	a->pos.x < a2->pos.x - conf::DIST ||
            		a->pos.x > a2->pos.x + conf::DIST ||
            		a->pos.y > a2->pos.y + conf::DIST ||
            		a->pos.y < a2->pos.y - conf::DIST)
            	continue;

			// standard distance formula
            float d= (a->pos-a2->pos).length();

            if (d<conf::DIST) {

                //smell
                smaccum+= 0.3*(conf::DIST-d)/conf::DIST;

                //sound
                soaccum+= 0.4*(conf::DIST-d)/conf::DIST*(max(fabs(a2->w1),fabs(a2->w2)));

                //hearing. Listening to other agents
                hearaccum+= a2->soundmul*(conf::DIST-d)/conf::DIST;

                float ang= (a2->pos- a->pos).get_angle(); //current angle between bots

                //left and right eyes
                float leyeangle= a->angle - PI8;
                float reyeangle= a->angle + PI8;
                float backangle= a->angle + M_PI;
                float forwangle= a->angle;
                if (leyeangle<-M_PI) leyeangle+= 2*M_PI;
                if (reyeangle>M_PI) reyeangle-= 2*M_PI;
                if (backangle>M_PI) backangle-= 2*M_PI;
                float diff1= leyeangle- ang;
                if (fabs(diff1)>M_PI) diff1= 2*M_PI- fabs(diff1);
                diff1= fabs(diff1);
                float diff2= reyeangle- ang;
                if (fabs(diff2)>M_PI) diff2= 2*M_PI- fabs(diff2);
                diff2= fabs(diff2);
                float diff3= backangle- ang;
                if (fabs(diff3)>M_PI) diff3= 2*M_PI- fabs(diff3);
                diff3= fabs(diff3);
                float diff4= forwangle- ang;
                if (fabs(forwangle)>M_PI) diff4= 2*M_PI- fabs(forwangle);
                diff4= fabs(diff4);

                if (diff1<PI38) {
                    //we see this agent with left eye. Accumulate info
                    float mul1= conf::EYE_SENSITIVITY*((PI38-diff1)/PI38)*((conf::DIST-d)/conf::DIST);
                    //float mul1= 100*((conf::DIST-d)/conf::DIST);
                    p1 += mul1*(d/conf::DIST);
                    r1 += mul1*a2->red;
                    g1 += mul1*a2->gre;
                    b1 += mul1*a2->blu;
                }

                if (diff2<PI38) {
                    //we see this agent with left eye. Accumulate info
                    float mul2= conf::EYE_SENSITIVITY*((PI38-diff2)/PI38)*((conf::DIST-d)/conf::DIST);
                    //float mul2= 100*((conf::DIST-d)/conf::DIST);
                    p2 += mul2*(d/conf::DIST);
                    r2 += mul2*a2->red;
                    g2 += mul2*a2->gre;
                    b2 += mul2*a2->blu;
                }

                if (diff3<PI38) {
                    //we see this agent with back eye. Accumulate info
                    float mul3= conf::EYE_SENSITIVITY*((PI38-diff3)/PI38)*((conf::DIST-d)/conf::DIST);
                    //float mul2= 100*((conf::DIST-d)/conf::DIST);
                    p3 += mul3*(d/conf::DIST);
                    r3 += mul3*a2->red;
                    g3 += mul3*a2->gre;
                    b3 += mul3*a2->blu;
                }

                if (diff4<PI38) {
                    float mul4= conf::BLOOD_SENSITIVITY*((PI38-diff4)/PI38)*((conf::DIST-d)/conf::DIST);
                    //if we can see an agent close with both eyes in front of us
                    blood+= mul4*(1-agents[j].health/2); //remember: health is in [0 2]
                    //agents with high life dont bleed. low life makes them bleed more
                }
            }
        }

        //TOUCH (wall)
        if(	a->pos.x < 2 || a->pos.x > conf::WIDTH - 3 ||
        	a->pos.y < 2 || a->pos.y > conf::HEIGHT - 3 )
        	//they are very close to the wall (1 or 2 pixels)
        	touch = 1;
        else
			touch = 0;

        //temperature varies from 0 to 1 across screen.
        //it is 0 at equator (in middle), and 1 on edges. Agents can sense discomfort
        float dd= 2.0*abs(a->pos.x/conf::WIDTH - 0.5);
       // float discomfort= abs(dd - a->temperature_preference);
		
		//initializing the inl vector
		//for(int x=0;x<21;x++)
		//{ a->inl[x]=0;
		//}

        a->in[0]= cap(p1);
        a->in[1]= cap(r1);
        a->in[2]= cap(g1);
        a->in[3]= cap(b1);
        a->in[5]= cap(p2);
        a->in[6]= cap(r2);
        a->in[7]= cap(g2);
        a->in[8]= cap(b2);
        a->in[9]= cap(soaccum);
        a->in[10]= cap(smaccum);
        a->in[12]= cap(p3);
        a->in[13]= cap(r3);
        a->in[14]= cap(g3);
        a->in[15]= cap(b3);
        a->in[16]= abs(sin(modcounter/a->clockf1));
        a->in[17]= abs(sin(modcounter/a->clockf2));
        a->in[18]= cap(hearaccum);
        a->in[19]= cap(blood);
     //   a->in[20]= discomfort;
		a->in[21] = touch;
                

		//cout<<" Values of in[4],[9],[10],[18]"<<a->in[4]<<" "<<a->in[9]<<" "<<a->in[10]<<" "<<a->in[18]<<"\n";

        

 // 4,9/10,18

		//if(a->inl[4]-a->in[4]<=0.001 && a->inl[9]-a->in[9]<=0.001 && a->inl[10]-a->in[10]<=0.001 && a->inl[18]-a->in[18]<=0.001 )
		//{
			//change output to something else...  make a function and call that function 

		//}
		
		// initialise the first time iteration values...set them to zero or may be some random value
		// initalise inl vector
		// check values of the input so that 0.001 is good to check

		//to store the inputs of previous tick
	/*	a1->inl[0]= a->in[0];
		a1->inl[1]= a->in[1];
a1->inl[2]= a->in[2];
a1->inl[3]= a->in[3];*/
                       //a->inl[4]= a->in[4];
//a1->inl[5]= a->in[5];
//a1->inl[6]= a->in[6];
//a1->inl[7]= a->in[7];
//a1->inl[8]= a->in[8];
			//a->inl[9]= a->in[9];
			//a->inl[10]= a->in[10];
//a1->inl[11]= a->in[11];
//a1->inl[12]= a->in[12];
//a1->inl[13]= a->in[13];
//a1->inl[14]= a->in[14];
//		a1->inl[15]= a->in[15];
//		a1->inl[16]= a->in[16];
//		a1->inl[17]= a->in[17];
			//a->inl[18]= a->in[18];
		/*a1->inl[20]= a->in[20];
		a1->inl[21]= a->in[21];*/

 //cout<<"Value of sound sensor"<< a->in[9]<<"\n";

		// Now assign the "plan" from the last outputs as the new inputs
		// PREV_PLAN is input 21-30
		// NEXT_PLAN is output 9-17

a->in[22]=rand() % 1 + 0;


// when there is no input
         if(a->in[4] ==0 && a->in[9] ==0 && a->in[10]==0 && a->in[18]==0)
           {
                 
             if(a->in[22]<=0.4)
                { a->out[0] = rand()%1+0;
                  a->out[1]= rand()%1+0;
                }	
             if(a->in[22]>0.4 && a->in[22] <=0.7)
               {  a->out[0] = rand()%1+0;
                  a->out[1] = rand()%1+0;
               }   
             if(a->in[22]>0.7)
               { a->out[0] = rand()%1+0;
                 a->out[1] = rand()%1+0;
                }
           }
		
//when bot senses another bot with both eyes
if (a->in[19]>=0.5)

{
    a->health+=0.0005;
}

//extra inputs
//for(int i=0;i<=9;i++)
//{a->in[i+13]=a->out[i];
//}

		for(int i = 9; i <= 17; ++i)
		{
			a->in[i+13] = a->out[i];
		}

    }
}

void World::processOutputs()
{
    //assign meaning
    //LEFT RIGHT R G B SPIKE BOOST SOUND_MULTIPLIER GIVING
    // 0    1    2 3 4   5     6         7             8

    for (int i=0;i<agents.size();i++) {
        Agent* a= &agents[i];

        a->red= a->out[2];
        a->gre= a->out[3];
        a->blu= a->out[4];
        a->w1= a->out[0]; //-(2*a->out[0]-1);
        a->w2= a->out[1]; //-(2*a->out[1]-1);
        a->boost= a->out[6]>0.5;
        a->soundmul= a->out[7];
        a->give= a->out[8];

        //spike length should slowly tend towards out[5]
        float g= a->out[5];
        if (a->spikeLength<g)
            a->spikeLength+=conf::SPIKESPEED;
        else if (a->spikeLength>g)
            a->spikeLength= g; //its easy to retract spike, just hard to put it up. 
    }

    //move bots
    //#pragma omp parallel for
    for (int i=0;i<agents.size();i++) {
        Agent* a= &agents[i];

        Vector2f v(conf::BOTRADIUS/2, 0);
        v.rotate(a->angle + M_PI/2);

        Vector2f w1p= a->pos+ v; //wheel positions
        Vector2f w2p= a->pos- v;

        float BW1= conf::BOTSPEED*a->w1; // bot speed * wheel speed
        float BW2= conf::BOTSPEED*a->w2;

        if (a->boost) {
            BW1=BW1*conf::BOOSTSIZEMULT;
            BW2=BW2*conf::BOOSTSIZEMULT;
        }

        //move bots
        Vector2f vv= w2p- a->pos;
        vv.rotate(-BW1);
        a->pos= w2p-vv;
        a->angle -= BW1;
        if (a->angle<-M_PI)
			a->angle= M_PI - (-M_PI-a->angle);
        vv= a->pos - w1p;
        vv.rotate(BW2);
        a->pos= w1p+vv;
        a->angle += BW2;
        if (a->angle>M_PI)
			a->angle= -M_PI + (a->angle-M_PI);

        //wrap around the map
        /*if (a->pos.x<0) a->pos.x= conf::WIDTH+a->pos.x;
        if (a->pos.x>=conf::WIDTH) a->pos.x= a->pos.x-conf::WIDTH;
        if (a->pos.y<0) a->pos.y= conf::HEIGHT+a->pos.y;
        if (a->pos.y>=conf::HEIGHT) a->pos.y= a->pos.y-conf::HEIGHT;*/

        //have peetree dish borders
        if (a->pos.x<0)
			a->pos.x = 0;
		if (a->pos.x>=conf::WIDTH)
			a->pos.x= conf::WIDTH - 1;
		if (a->pos.y<0)
			a->pos.y= 0;
		if (a->pos.y>=conf::HEIGHT)
			a->pos.y= conf::HEIGHT - 1;
    }

    //process food intake for herbivors
    for (int i=0;i<agents.size();i++) {

        int cx= (int) agents[i].pos.x/conf::CZ;
        int cy= (int) agents[i].pos.y/conf::CZ;
        float f= food[cx][cy];
        if (f>0 && agents[i].health<2) {
            //agent eats the food
            float itk=min(f,conf::FOODINTAKE);
            float speedmul= (1-(abs(agents[i].w1)+abs(agents[i].w2))/2)*0.6 + 0.4;
            itk= itk*agents[i].herbivore*speedmul; //herbivores gain more from ground food
            agents[i].health+= itk;
            agents[i].repcounter -= 3*itk;
            food[cx][cy]-= min(f,conf::FOODWASTE);
        }
    }

    //process giving and receiving of food
    for (int i=0;i<agents.size();i++) {
        agents[i].dfood=0;
    }
    for (int i=0;i<agents.size();i++) {
        if (agents[i].give>0.5) {
            for (int j=0;j<agents.size();j++) {
                float d= (agents[i].pos-agents[j].pos).length();
                if (d<conf::FOOD_SHARING_DISTANCE) {
                    //initiate transfer
                    if (agents[j].health<2) agents[j].health += conf::FOODTRANSFER;
                    agents[i].health -= conf::FOODTRANSFER;
                    agents[j].dfood += conf::FOODTRANSFER; //only for drawing
                    agents[i].dfood -= conf::FOODTRANSFER;
                }
            }
        }
    }

    //process spike dynamics for carnivors
    if (modcounter%2==0) { //we dont need to do this TOO often. can save efficiency here since this is n^2 op in #agents
        for (int i=0;i<agents.size();i++) {

            //NOTE: herbivore cant attack. TODO: hmmmmm
            //fot now ok: I want herbivores to run away from carnivores, not kill them back
            if(agents[i].herbivore>0.8 || agents[i].spikeLength<0.2 || agents[i].w1<0.5 || agents[i].w2<0.5) continue; 
            
            for (int j=0;j<agents.size();j++) {
                
                if (i==j) continue;
                float d= (agents[i].pos-agents[j].pos).length();

                if (d<2*conf::BOTRADIUS) {
                    //these two are in collision and agent i has extended spike and is going decent fast!
                    Vector2f v(1,0);
                    v.rotate(agents[i].angle);
                    float diff= v.angle_between(agents[j].pos-agents[i].pos);
                    if (fabs(diff)<M_PI/8) {
                        //bot i is also properly aligned!!! that's a hit
                        float mult=1;
                        if (agents[i].boost) mult= conf::BOOSTSIZEMULT;
                        float DMG= conf::SPIKEMULT*agents[i].spikeLength*max(fabs(agents[i].w1),fabs(agents[i].w2))*conf::BOOSTSIZEMULT;

                        agents[j].health-= DMG;

                        if (agents[i].health>2) agents[i].health=2; //cap health at 2
                        agents[i].spikeLength= 0; //retract spike back down

                        agents[i].initEvent(40*DMG,1,1,0); //yellow event means bot has spiked other bot. nice!

                        Vector2f v2(1,0);
                        v2.rotate(agents[j].angle);
                        float adiff= v.angle_between(v2);
                        if (fabs(adiff)<M_PI/2) {
                            //this was attack from the back. Retract spike of the other agent (startle!)
                            //this is done so that the other agent cant right away "by accident" attack this agent
                            agents[j].spikeLength= 0;
                        }
                        
                        agents[j].spiked= true; //set a flag saying that this agent was hit this turn
                    }
                }
            }
        }
    }
}

void World::brainsTick()
{
    #pragma omp parallel for
    for (int i=0;i<agents.size();i++) {
        agents[i].tick();
    }
}

void World::addRandomBots(int num)
{
    for (int i=0;i<num;i++) {
        Agent a;
        a.id= idcounter;
        idcounter++;
        agents.push_back(a);
    }
}
void World::addCarnivore()
{
    Agent a;
    a.id= idcounter;
    idcounter++;
    a.herbivore= randf(0, 0.1);
    agents.push_back(a);
}

void World::addNewByCrossover()
{

    //find two success cases
    int i1= randi(0, agents.size());
    int i2= randi(0, agents.size());
    for (int i=0;i<agents.size();i++) {
        if (agents[i].age > agents[i1].age && randf(0,1)<0.1) {
            i1= i;
        }
        if (agents[i].age > agents[i2].age && randf(0,1)<0.1 && i!=i1) {
            i2= i;
        }
    }

    Agent* a1= &agents[i1];
    Agent* a2= &agents[i2];


    //cross brains
    Agent anew = a1->crossover(*a2);


    //maybe do mutation here? I dont know. So far its only crossover
    anew.id= idcounter;
    idcounter++;
    agents.push_back(anew);
}

void World::reproduce(int ai, float MR, float MR2)
{
    if (randf(0,1)<0.04) MR= MR*randf(1, 10);
    if (randf(0,1)<0.04) MR2= MR2*randf(1, 10);

    agents[ai].initEvent(30,0,0.8,0); //green event means agent reproduced.
    for (int i=0;i<conf::BABIES;i++) {

        Agent a2 = agents[ai].reproduce(MR,MR2);
        a2.id= idcounter;
        idcounter++;
        agents.push_back(a2);

        //TODO fix recording
        //record this
        //FILE* fp = fopen("log.txt", "a");
        //fprintf(fp, "%i %i %i\n", 1, this->id, a2.id); //1 marks the event: child is born
        //fclose(fp);
    }
}

void World::writeReport()
{
    //TODO fix reporting
    //save all kinds of nice data stuff
//     int numherb=0;
//     int numcarn=0;
//     int topcarn=0;
//     int topherb=0;
//     for(int i=0;i<agents.size();i++){
//         if(agents[i].herbivore>0.5) numherb++;
//         else numcarn++;
// 
//         if(agents[i].herbivore>0.5 && agents[i].gencount>topherb) topherb= agents[i].gencount;
//         if(agents[i].herbivore<0.5 && agents[i].gencount>topcarn) topcarn= agents[i].gencount;
//     }
// 
//     FILE* fp = fopen("report.txt", "a");
//     fprintf(fp, "%i %i %i %i\n", numherb, numcarn, topcarn, topherb);
//     fclose(fp);
}


void World::reset()
{
    agents.clear();
    addRandomBots(conf::NUMBOTS);
}

void World::setClosed(bool close)
{
    CLOSED = close;
}

bool World::isClosed() const
{
    return CLOSED;
}


void World::processMouse(int button, int state, int x, int y)
{
     if (state==0) {        
         float mind=1e10;
         float mini=-1;
         float d;

         for (int i=0;i<agents.size();i++) {
             d= pow(x-agents[i].pos.x,2)+pow(y-agents[i].pos.y,2);
                 if (d<mind) {
                     mind=d;
                     mini=i;
                 }
             }
         //toggle selection of this agent
         for (int i=0;i<agents.size();i++) agents[i].selectflag=false;
         agents[mini].selectflag= true;
         agents[mini].printSelf();
     }
}
     
void World::draw(View* view, bool drawfood)
{
    if(drawfood) {
        for(int i=0;i<FW;i++) {
            for(int j=0;j<FH;j++) {
                float f= 0.5*food[i][j]/conf::FOODMAX;
                view->drawFood(i,j,f);
            }
        }
    }
    vector<Agent>::const_iterator it;
    for ( it = agents.begin(); it != agents.end(); ++it) {
        view->drawAgent(*it);
    }
}

std::pair< int,int > World::numHerbCarnivores() const
{
    int numherb=0;
    int numcarn=0;
    for (int i=0;i<agents.size();i++) {
        if (agents[i].herbivore>0.5) numherb++;
        else numcarn++;
    }
    
    return std::pair<int,int>(numherb,numcarn);
}

int World::numAgents() const
{
	if(agents.size() == 0)
	{
		cout << "Population is extinct at epoch " << current_epoch  << endl;
		exit(1);
	}
    return agents.size();
}

int World::epoch() const
{
    return current_epoch;
}

