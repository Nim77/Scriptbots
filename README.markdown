SCRIPTBOTS
==========
* Author: Andrej Karpathy
* License: Do whatever you want with this code

Project website: 
(https://sites.google.com/site/scriptbotsevo/home)

Mailing List / Forum:
(http://groups.google.com/group/scriptbots/topics)

BUILDING
---------

To compile scriptbots you will need:

* CMake >= 2.8 (http://www.cmake.org/cmake/resources/software.html)
* OpenGL and GLUT (http://www.opengl.org/resources/libraries/glut/)
* * Linux: freeglut (http://freeglut.sourceforge.net/) 

It will use OpenMP to speed up everything, in case you have multicore cpu.

**For Ubuntu/Debian**

Install all the dependencies with:

    $ sudo apt-get install cmake build-essential libopenmpi-dev freeglut3-dev libxi-dev libxmu-dev

To build ScriptBots on Linux:

    $ cd path/to/source
    $ mkdir build
    $ cd build
    $ cmake ../ # this is the equiv of ./configure
    $ make

To execute ScriptBots simply type the following in the build directory:

    $ ./scriptbots

If you are running Linux through VirualBox you will need to run this command:
    $ LIBGL_ALWAYS_INDRECT=1 ./scriptbots

**For Windows**

Follow basically the same steps, but after running cmake open up the VS solution (.sln) file it generates and compile the project from VS.


USAGE
------
Follow the above instructions to compile then run the program.

**Keyboard Shortcuts:**

* p = pause
* d = toggle drawing (for faster computation)
* f = draw food too
* q = add predators
* + = faster
* - = slower

**Mouse Usage:**

Pan around by holding down right mouse button, and zoom by holding down middle button. Click to see brain details.

**Bot Status Indicator Colors:**

* WHITE: bot just ate part of another agent
* YELLOW: bot just spiked another bot
* GREEN: bot just reproduced

RECORDING
---------
On Linux:

	$ sudo apt-get install xvidcap avidemux audacity ffmpeg2theora mplayer
   	$ xvidcap


QUESTIONS COMMENTS 
------------------
Best posted at the google group, available on project site
or contact me at andrej.karpathy@gmail.com

Contributors:

* Casey Link <unnamedrambler@gmail.com>
* Dave Coleman <davetcoleman@gmail.com>
* Nimisha Morkonda <mgnimisha@gmail.com>

BRAIN MAPPING
------------

P1 R1 G1 B1 FOOD P2 R2 G2 B2 SOUND SMELL HEALTH P3 R3 G3 B3 CLOCK1 CLOCK 2 HEARING  BLOOD  TEMP   TOUCH  PREV_PLAN
0   1  2  3  4   5   6  7 8   9     10     11   12 13 14 15 16       17      18      19     20     21     22-31


LEFT RIGHT R G B SPIKE BOOST SOUND_MULTIPLIER GIVING  NEXT_PLAN
  0   1    2 3 4   5     6         7             8      9-17

