# Multimedia-Project 2020
Multimedia Project with opengl.  
Spacegame-Style (kinda like Astoroids)  


## Used Libaries  
  stdio.h  
  stdlib.h  
  GL/freeglut.h  
  math.h  
  limits.h  
  time.h
  string.h

 
## structure  
  c++ code  
  not object orientated  
  mainly based on c code  

## requirements  
  freeglut (newer version of glut)  
  installation:  
  ```
  sudo apt-get install freeglut3-dev
  ```
  glut (to old and not supported scince 1998)  

## compiling game  
  make (or)  
  g++ -Wall -g -c rocket_dodge.cpp   
  g++ rocket_dodge.o -o rocketdodge -lGL -lGLU -lglut  

## start game  
  ./rocketdodge  
  options for parameters:  
    -e (easy)  
    -m (medium)  
    -h (hard)  
    -k (korean || very hard)  

## play controlls  
  ENTER = NewGame  
  ESC = Close Application  
  P = Pause/Resume  
  W / ArrowUp = Move upwards  
  S / ArrowDown = Move downwards  
  A / ArrowLeft = Move left  
  D / ArrowRight = Move right  
  LMB = Rotate Ship left  
  RMB = Rotate Ship right  
  MMB = Rotate to original position  

### in game-over screen:  
  change of difficulty is possible  
  press button for change:  
  e = easy  
  m = medium  
  h = hard  
  k = korean || very hard  



