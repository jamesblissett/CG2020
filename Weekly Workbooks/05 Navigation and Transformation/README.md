## Navigation and Transformation
### <a href='https://web.microsoftstream.com/channel/b84051cb-1dba-4cb4-a271-60cc6635f92f' target='_blank'> Weekly Briefing ![](../../resources/icons/briefing.png) </a>
### Task 1: Introduction


This workbook continues on from where we left off last week, by implementing additional features to the rasterising renderer. We start by adding texture mapping to the existing renderer. Finally we make the whole thing interactive (so that the user can navigate through the model). If you didn't complete all of last week's tasks, you should finish these first before starting on this week's workbook.  


# 
### Task 2: Texture Mapping
 <a href='02%20Texture%20Mapping/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='02%20Texture%20Mapping/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a>

The aim of this task is to integrate the texture mapping function that you implemented in a previous workbook into your 3D rasteriser. Rather than drawing the floor of the Cornell Box model as two plain green triangles, you should texture these triangles using our usual <a href="models/texture.ppm" target="_blank">texture image</a>.

Take a look at the slides and audio narration above for more information on how OBJ files handle texture information. Use this knowledge to extend your file parsing function to read and then store the additional texture information. Note that the `ModelTriangle` class contains an array of `TexturePoints` to help you maintain the relationship between triangle vertices and texture point coordinates.

To help you in this task, we have provided an <a href="models/textured-cornell-box.mtl" target="_blank">updated OBJ materials file</a> and an <a href="models/textured-cornell-box.obj" target="_blank">updated OBJ geometry file</a> both of which contain additional texture information (namely a new textured material called "Cobbles", texture mapping points for some of the vertices and a reference to the texture image file).

When implemented correctly, you should end up with a Cornell Box with the textured floor, similar to that shown in the diagram below.  


![](02%20Texture%20Mapping/images/textured-floor.png)

**Hints & Tips:**  
Although this approach works OK, you might have an intuition that something isn't quite right - and that something is to do with depth and perspective. Try not to worry about this too much at the moment - we will return to the topic of 3D texture mapping in a later workbook. For the time being, this basic approach to texture mapping is fine.  


# 
### Task 3: Camera Position
 <a href='03%20Camera%20Position/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='03%20Camera%20Position/slides/segment-2.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='03%20Camera%20Position/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='03%20Camera%20Position/audio/segment-2.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a>

Your render of the Cornell Box is probably looking quite good by now ? The problem is that in order to _really_ test out your rendering code, you need to view the model from a number of different angles. Take a look and the slides and audio narration above relating to translation and rotation, then add some key event handlers to your code that will allow a user to interactively:

- translate the camera in 3 dimensions (up/down, left/right, forwards/backwards)
- rotate the camera position about the centre of the model in 2 dimensions (about the X axis and about the Y axis)

Both of these features will allow you to move the camera in order to see how your render looks from different points of view. **Important**: It is essential that your code only manipulates the position of the camera (i.e. you do not alter the vertices of the model artefacts themselves).  


**Hints & Tips:**  
Translating the position of the camera is relatively easy - it's just a case of adding (or subtracting) a suitable amount to either the x, y, or z coordinate of the camera position `vec3`. Rotation on the other hand is a little more complex - you will need to create a 3x3 rotation matrix to apply to the camera position. 

Take a look at <a href="https://en.wikibooks.org/wiki/GLSL_Programming/Vector_and_Matrix_Operations" target="_blank">this documentation on matrices</a> to find out how to create can manipulate matrices in GLM. When instantiating a `mat3`, remember that GLM is _column major_ (i.e. the order of parameters is first column, then second column, then third).

Once you have created a suitable `mat3` rotation matrix, this can be applied to the camera position vector. Luckily, GLM vectors and matrices override the `*` operator, so we can simply multiply the camera position vector by the rotation transformation matrix to get the new camera position !  


# 
### Task 4: Camera Orientation
 <a href='04%20Camera%20Orientation/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='04%20Camera%20Orientation/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='04%20Camera%20Orientation/animation/segment-1.mp4' target='_blank'> ![](../../resources/icons/animation.png) </a>

When you start to move the camera around using the features added in the previous task, you quickly get to the point where the model is no longer visible from the camera position. What we really want is to be able to _rotate the camera itself_ so that we can keep the model in view.

Explore the slides and audio narration above relating to camera orientation. Then add a new `mat3` matrix to your code to hold the current up/right/forward orientation of the camera. Be sure to take this camera orientation matrix into account when projecting vertices onto the image plane.

Also add additional event handlers so that the user can alter the orientation of the camera using suitable keys on the keyboard: rotating the camera in the Y axis (panning) and X axis (tilting). Rotating the camera in the Z axis (roll) is not a particularly common manipulation, so there is no need to implement this here.

  


**Hints & Tips:**  
When rotating the orientation of the camera, we again need to create a 3x3 rotation matrix. Once you have a created a suitable `mat3` rotation matrix, this should be applied to the camera orientation matrix. Again, we can make use of the `*` operator to multiply the camera orientation matrix by the rotation transformation matrix to calculate the new camera orientation.

**Important**: It is essential that your code only manipulates the position of the camera (i.e. you do not alter the vertices of the model artefacts themselves).  


# 
### Task 5: Orbit and LookAt
 <a href='05%20Orbit%20and%20LookAt/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='05%20Orbit%20and%20LookAt/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='05%20Orbit%20and%20LookAt/animation/segment-1.mp4' target='_blank'> ![](../../resources/icons/animation.png) </a>

It soon becomes a pain to have to manually move the camera around the scene using the keyboard each time we want to get a different perspective on a model. To ease this problem (and to make testing of your rendered easier) we will now include some additional code that automatically animates the position of your camera. More specifically, add some code to rotate (orbit) the position on the camera (in the Y axis) about the centre of the Cornell Box. This will involve applying a small rotation transformation to the camera position vector every time the `update` function is called.

The problem with the above feature is that you will soon lose sight of the model (because, although the camera position is rotating, it will always be looking off in the same direction !). To avoid this issue, implement a `lookAt` function as described in the above slides and audio narration. This will ensure that the camera is always oriented towards the centre of the scene, so that the Cornell Box room will always be visible. Call this `lookAt` function from your `update` method every time you shift the position of the camera.  


**Hints & Tips:**  
See the animation linked to above for a demonstration of the effect that you should be trying to achieve (note: there is no audio description for this particular animation !).  


# 
### Task 6: Homogeneous Coordinates (optional)
 <a href='06%20Homogeneous%20Coordinates%20%28optional%29/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='06%20Homogeneous%20Coordinates%20%28optional%29/slides/segment-2.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='06%20Homogeneous%20Coordinates%20%28optional%29/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='06%20Homogeneous%20Coordinates%20%28optional%29/audio/segment-2.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a>

This is an optional task that you may (or may not ?) wish to implement. It is useful to know about homogenous coordinates - so _do_ view the slides and audio narration above. However, it is up to you whether you choose to implement these in your rendering code at this point in time. They are not essential to complete the exercises and they offer only very marginal benefit for the kinds of rendering that we will be attempting in the remainder of this unit.

If you have the time (and the desire to do so) refactor your code to operate with homogenous coordinates. This will require you to replace _vec3_ variables with _vec4_ and _mat3_ with _mat4_. You may also find that you get some interesting "bonus features" and "side effects" with features such as _LookAt_ !  


# 
### Progress Logbook

It is important to be able to monitor the progress of students as they progress through these workbooks.
This is to ensure that everyone is keeping up to date with the work and to identify students who are struggling.
For these reasons, it is **ESSENTIAL** that you keep a logbook of progress that you are making.
To minimise the overhead in keeping this logbook, you are only required to capture a single screenshot each week
(you only need to include the DrawingWindow, not the whole desktop).
Try to capture something that summarises the progress that you have made and upload this onto the
[logbook submission area on blackboard](https://www.ole.bris.ac.uk/webapps/assignment/uploadAssignment?content_id=_4837989_1&course_id=_240795_1&group_id=&mode=cpview).
(Note: you might have to log into blackboard _first_ before your click on this link !).
It is important that **EVERYBODY** uploads an image (so we can be sure that everyone is engaging with the unit).
If you don't keep your logbook up to date then you'll find us getting in touch to see if everything is OK !