# Spaceship Traveller Project - Readme



## Installation Steps

1. **Download and Unzip**: Download the zip file and unzip it on your computer.
2. **Setup Environment**:
   - For Mac: Use Visual Studio Code.
   - For Windows: Use Visual Studio.
3. **Install Tools**: Install C++ and CMake tools on both platforms.
4. **Open Project**: Open the `Project_CRX-main` folder in your chosen software.
5. **Build Libraries**: Run the  `main.cpp` the first time it will build the necessary libraries (this may take some time).
6. **Launch Game**: Once built, a game window will open, ready for play.

## Gameplay Tips

- **Movement**: Use arrow keys or 'zqsd' to control the spaceship.
- **Shoot**: Use left click to shoot bullets

## Project Structure

This document outlines the directory structure of the "Spaceship Traveller" OpenGL project.
<details>
  <summary>Click here to see the architecture of the main folder </summary>

```bash
Project_CRX-main/   ------------------------ #This is the root directory of the project.
    ├── 3rdParty/   ------------------------ #Contains all the libraries essential for the project. This includes physics engines, window management libraries, and other dependencies.
    │   ├── bullet/
    │   ├── assimp /
    │   ...
    ├── PROJECT_CRX/  ------------------------ #The primary directory for the project's code and resources.
    │   ├── code/     ------------------------ #Stores the main executable code that integrates various components of the project.
    │   │   ├── main.cpp
    │   │   ├── colladainterface.cpp
    │   ├── headers/  ------------------------ #Contains all the header files (.h) defining the classes and functions used in the project. 
    │   │   ├── colladaanimation.h
    │   │   ├── collision.h
    │   │   ...
    │   ├── objects/  ------------------------ #Contains object files (.obj, .dae) necessary for building the different objects.
    │   │   ├── sphere.obj
    │   │   ├── object2.dae
    │   │   ...
    │   ├── source/   ------------------------ #This folder is dedicated to shader files (vertex , fragment, geometry, compute ), used for rendering the game's graphics.
    │   │   ├── fragSrc
    │   │   ├── ComputeShader
    │   │   ...
    │   ├── textures/  ------------------------ #Includes texture files (.png, .jpg) used to render colors and textures for different objects in the game, like spaceships, asteroids, and celestial bodies.
    │       ├── earth.png
    │       ├── moon.jpg
    │       ...

```
</details>


## Librairies dependencies


- **Bullet**: Physics used for simulating realistic physical interactions and collisions. Able to handle complex simulations like rigid body dynamics and collision detection.
- **Assimp**:  It simplifies the process of integrating complex 3D models and animations into the project, including Collada animations.
- **GLAD**: Provides access to the latest OpenGL features for rendering. Manages the function pointers for OpenGL, making it easier to call OpenGL functions.
- **GLFW**: Manages the game window and player interactions.
- **GLM**: Mathematics library designed for graphics software. It provides a lot of  mathematical operations crucial for graphics programming.
- **std**: Standard C++  library with essential coding tools.
- **TinyXML**: It's used for reading, writing, and manipulating XML files. Perfect to load our collada objects.



