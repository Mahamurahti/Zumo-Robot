# PSoC Creator project for Metropolia
This source code was used in the robot competition 13.12.2019 in Metropolia UAS.
See "main.cs" for all the different codes used for maze solving, line following and sumo wrestling.

## Maze solving
In maze solving the robot has to follow lines in a grid and avoid obstacles as well as get to the  victory line. All this had to be done without bumping into the obstacles and staying on the line. Our logic for maze solving was the following: in every intersection the robot will turn left, if it can't turn left it will go straight and if it can't turn left or go straight it will turn right.

## Line following
In line following we just had to configure the robots turning speeds so that the robot survives the hard turns. A lot of configuration went into this.

## Sumo wrestling
In sumo wrestling our strategy was to just to stay in the circle and if an enemy was detected, speed up in an attempt to throw the other robot off the ring.

Code contributors:
* Eric Keränen
* Maksim Ilmast
* Jonne Kirvesoja
