# Spaceship Traveller Project - Readme

## Project Structure

This document outlines the directory structure of the "Spaceship Traveller" OpenGL project.
```bash
SpaceshipTraveller/
    ├── 3rdParty/
    │   ├── bullet/
    │   ├── assimp /
    │   ...
    ├── PROJECT_CRX/
    │   ├── code/
    │   │   ├── main.cpp
    │   ├── headers/
    │   │   ├── colladaanimation.h
    │   │   ├── collision.h
    │   │   ...
    │   ├── objects/
    │   │   ├── sphere.obj
    │   │   ├── object2.dae
    │   │   ...
    │   ├── source/
    │   │   ├── fragSrc
    │   │   ├── ComputeShader
    │   │   ...
    │   ├── textures/
    │       ├── earth.png
    │       ├── moon.jpg
    │       ...

```

### Main Directory
This is the root directory of the project.

#### `3rdParty`
Contains all the third-party libraries essential for the project. This includes physics engines, window management libraries, and other dependencies.

#### `PROJECT_CRX`
The primary directory for the project's code and resources.

##### `Main Code`
Stores the main executable code that integrates various components of the project.

##### `Headers`
Contains all the header files (.h) defining the classes and functions used in the project.

##### `Objects`
Holds object files (.obj) necessary for building the final executable.

##### `Source`
This folder is dedicated to shader files (.vert, .frag, .geom, etc.), crucial for rendering the game's graphics.

##### `Textures`
Includes texture files (.png, .jpg, etc.) used to render realistic appearances for objects in the game, like spaceships, asteroids, and celestial bodies.


## Librairies dependencies
