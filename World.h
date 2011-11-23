#ifndef WORLD_H
#define WORLD_H

#include "View.h"
#include "Agent.h"
#include "settings.h"
#include <vector>
class World
{
public:
    World();
    ~World();
    
    void update();
    void reset();
    
    void draw(View* view, bool drawfood);
    
    bool isClosed() const;
    void setClosed(bool close);
    
    /**
     * Returns the number of herbivores and 
     * carnivores in the world.
     * first : num herbs
     * second : num carns
     */
    std::pair<int,int> numHerbCarnivores() const;
    
    int numAgents() const;
    int epoch() const;
    
    //mouse interaction
    void processMouse(int button, int state, int x, int y);

    void addNewByCrossover();
    void addRandomBots(int num);
    void addCarnivore();
    
private:
    void setInputs();
    void processOutputs();
    void brainsTick();  //takes in[] to out[] for every agent
    
    void growFood(int x, int y);

    void writeReport();
    
    void reproduce(int ai, float MR, float MR2);
    
    int modcounter;
    int current_epoch;
    int idcounter;
    
    std::vector<Agent> agents;
    
    // food
    int FW;
    int FH;
    int fx;
    int fy;
    float food[conf::WIDTH/conf::CZ][conf::HEIGHT/conf::CZ];
    bool CLOSED; //if environment is closed, then no random bots are added per time interval
    // temp
    float temp[conf::WIDTH][conf::HEIGHT];
    bool touch;
};

#endif // WORLD_H
