# HeightField

###0.Running the Program
**For Mac:**
> cd hw1-starterCode
> make
> ./hw1 heightmap/spiral.jpg (or other images)

**For Windows (VS 2017):**
> Run the .sln
> Select Project > hw1 Properties > Debugging > Command Arguments

> F5 to Run

###1.Image Import 
This program supports:
- Grayscale (1 Channel) image input
  ```<spiral.jpg, Heightmap.jpg>```
  ```<SantaMonicaMountains-128.jpg, SantaMonicaMountains-256.jpg, SantaMonicaMountains-512.jpg, SantaMonicaMountains-768.jpg>```
  ```<OhioPyle-128.jpg, OhioPyle-256.jpg, OhioPyle-512.jpg, OhioPyle-768.jpg>```
  ```<GrandTeton-128.jpg, GrandTeton-256.jpg, GrandTeton-512.jpg, GrandTeton-768.jpg>```

- RGB (3 Channels) image input (Extra Credit)  

  ```	<color128.jpg, color256.jpg, color512.jpg, color768.jpg>```
  **NOTE: ORIGINAL COLOR IMAGE WAS 256 * 256, SO MAY LOOK PIXEL-LIKE WHEN ENLARGED TO 512 and 768.**
  **NOTE: Corresponding size of color image will be used for mode 5.**

###2.Mode Selection
Keyboard ``` 1``` -> ```Points``` Mode
Keyboard ``` 2``` -> ```Wireframe``` Mode
Keyboard ```3``` -> ```Triangle``` Mode
Keyboard ```4``` -> ```Wireframe Overlay Triangles``` (Extra Credit)
Keyboard ```5``` -> ```Color Vertices``` Based on Color Values Taken from Another Image of Equal Size  (Extra Credit)
Keyboard ```6```-> Change to ```Another Shader``` (Extra Credit)
Keyboard ```a or d```-> **Translate (Move)** on X Axis
Keyboard ```w or s``` -> **Translate (Move)** on Y Axis
Keyboard ```q or e``` -> **Translate (Move)** on Z Axis
Keyboard ```SHIFT + a or d``` -> **Scale** on X Axis
Keyboard ```SHIFT + w or s``` -> **Scale** on Y Axis
Keyboard ```SHIFT + q or e``` -> **Scale** on Z Axis
Keyboard ```CTRL + a or d``` -> **Rotate** on X Axis
Keyboard ```CTRL + w or s``` -> **Rotate** on Y Axis
Keyboard ```CTRL + q or e``` -> **Rotate** on Z Axis
Keyboard ```x``` -> Take a Screenshot and Save to "screenshot.jpg"
Keyboard ```esc``` -> Exit the Program

###3.Translate, Scale and Rotate
If``` CTRL``` is Pressed -> **Translate** Mode
- Control x, y Translation via the Left Mouse Button
- Control z Translation via the Middle Mouse Button

If ```SHIFT``` is Pressed -> **Scale** Mode
- Control x, y Scaling via the Left Mouse Button
- Control z Scaling via the Middle Mouse Button

If Neither is Pressed -> **Rotate** Mode
- Control x, y Rotation via the Left Mouse Button
- control z Rotation via the Middle Mouse Button

###4.Modified Files
- hw1.cpp
- basicPipelineProgram.cpp 
  (Changed Init to build different shaders)
  (Now uses program handle in hw1.cpp instead of SetModelViewMatrix, SetProjectionMatrix, and SetShaderVariableHandles)
- basicPipelineProgram.h
- basic.fragmentShaderColor.glsl

###5.Draw
- Points, Wireframe uses glDrawArrays
- Triangle uses glDrawElements (Extra Credit)

###6.Shaders
  There are two fragment shaders used in the program.
  To switch shaders to use, press 6.

###7.Animation
  Animation is commented out.It is located in the IdleFunc, line 716 to line 800. 
  When animating, it will save 300 screenshots to animation folder.