## Ray Tracing and Intersection
### <a href='https://web.microsoftstream.com/channel/b84051cb-1dba-4cb4-a271-60cc6635f92f' target='_blank'> Weekly Briefing ![](../../resources/icons/briefing.png) </a>
### Task 1: Introduction
 <a href='01%20Introduction/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='01%20Introduction/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='01%20Introduction/animation/segment-1.mp4' target='_blank'> ![](../../resources/icons/animation.png) </a>

Rasterising is a fast and efficient approach to rendering a 3 dimensional scene. However, it has limitations it that there are a number of phenomena that are difficult (or impossible) to achieve using this approach. In particular, calculating light and shadow is not well suited to rasterising. For this reason, we now turn our attention to an alternative form of rendering, namely _ray tracing_. Take a look at the slides, audio narration and animation linked to above in order to gain a high-level understanding of the operation of ray tracing. Once you have grasped the basic concepts, move on to the tasks in the remainder of this workbook.

It is advisable to continue working with your current project and augment it with additional ray tracing functions. This will not only allow you to switch between different rendering models (wireframe, rasterised, ray-traced) but will also permit "hybrid" rendering to take place (don't worry what this means yet - we will discuss it in more detail later).  


**Hints & Tips:**  
The code that you will write in this workbook will be very resource intensive. If you find that your renderer is running too slowly to allow you to test it effectively, then you might like to try using the optimised `make speedy` build rule.  


# 
### Task 2: Finding the Closest Intersection
 <a href='02%20Finding%20the%20Closest%20Intersection/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='02%20Finding%20the%20Closest%20Intersection/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='02%20Finding%20the%20Closest%20Intersection/audio/segment-2.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a> <a href='02%20Finding%20the%20Closest%20Intersection/animation/segment-1.mp4' target='_blank'> ![](../../resources/icons/animation.png) </a> <a href='02%20Finding%20the%20Closest%20Intersection/animation/segment-2.mp4' target='_blank'> ![](../../resources/icons/animation.png) </a>

The first activity we need to perform when attempting ray tracing is to detect when a projected ray intersects with a triangle. Watch the narrated slides and animations above to gain an understanding of how to perform this operation, then write a function called `getClosestIntersection` that given:

- The position vector of the camera
- The direction vector of a ray being cast into the scene
- The set of all of the triangles in the model

... will search through the triangles and return details of the intersected point on the _closest_ triangle (if any !). You are going to need to return various details about the intersection point found. There is a class in the SDW folder called `RayTriangleIntersection` that you might like to use to store and return all of these details.

You will not be able to test your function just yet. Write it as best you can for the moment, we will test it fully in a later task (when we have build more of the ray tracer). At which point you may have to return to this task to revise your `getClosestIntersection` function.  


**Hints & Tips:**  
To help you towards implementing your `getClosestIntersection` function, below is a code equivalent of the ray/triangle intersection equation that was shown in the slides at the top of this section:  

``` cpp
glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
glm::mat3 DEMatrix(-rayDirection, e0, e1);
glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
```
Remember that the `possibleSolution` calculated by the above code is **not** the [x,y,z] coordinates of a point in 3 dimensional space, but rather a three-element data structure that consists of:

- `t` the distance along the ray from the camera to the intersection point
- `u` the proportion along the triangle's first edge that the intersection point occurs
- `v` the proportion along the triangle's second edge that the intersection point occurs
  


# 
### Task 3: Validating the Intersection


We need to be careful when searching for ray/triangle intersections because the `getClosestIntersection` function can often return false-positive results. Any possible solution **must** be validated against the following constraints on `u` and `v`:

```
(u >= 0.0) && (u <= 1.0)
(v >= 0.0) && (v <= 1.0)
(u + v) <= 1.0
```

This is important because the found solution might well be on the same plane as the triangle, but **not** within the bounds of the triangle itself (as illustrated in the diagram below). You should also check that the distance `t` from the camera to the intersection is positive (otherwise you may end up rendering triangles that are actually _behind_ the camera !)  


![](03%20Validating%20the%20Intersection/images/outside-bounds.jpg)

**Hints & Tips:**  
Test your `getClosestIntersection` function by passing it a direction vector with a _known_ intersection (e.g. the ray that is fired through the dead centre of the image plane _should_ intersect with the front of the blue box in the test model).  


# 
### Task 4: Ray Tracing the Scene
 <a href='04%20Ray%20Tracing%20the%20Scene/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='04%20Ray%20Tracing%20the%20Scene/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a>

Write a new `draw` function that renders the Cornell Box model using the closest intersection algorithm. Don't throw away your old rasterising draw function - you are going to need this later !

In your new draw function, you should loop through each pixel on the image plane (top-to-bottom, left-to-right), casting a ray from the camera, through the current pixel and into the scene. You should determine if this ray intersects with a triangle from the model and, when it does, shade the image plane pixel with the colour of the closest intersected triangle.  


**Hints & Tips:**  
At this stage, this doesn't seem like progress - if anything, this is a _slower_ way of getting the same results as with the rasteriser (although don't forget about the `make speedy` build rule). In the following task however (and next week as well), we will be doing things that are hard or impossible to do with a rasteriser - so stick with it, this is all worth doing !  


# 
### Task 5: Shadows
 <a href='05%20Shadows/slides/segment-1.pdf' target='_blank'> ![](../../resources/icons/slides.png) </a> <a href='05%20Shadows/audio/segment-1.mp4' target='_blank'> ![](../../resources/icons/audio.png) </a>

Shadows are a key element of 3D rendering that we have as yet not addressed. We see them all the time in the real world, so if we want our renders to look realistic, we are going to need to simulate them.

Determining when a point in a scene should be drawn in shadow is conceptually relatively straight-forward. However there are some additional complexities that we have to deal with when implementing a consistent shadow effect. Review the slides and audio narrative relating to shadows above and then implement the concepts in your ray tracer.

You are going to need a new `vec3` variable to hold the position of a single-point light source. A location in the middle of the room, above the origin, somewhere near the ceiling would seem sensible place for it !

Once correctly implemented, your render of the Cornell box should look something like the image below (depending on where you position your light source):  


![](05%20Shadows/images/without-ambient.jpg)

**Hints & Tips:**  
It is worth noting that your already-implemented `getClosestIntersection` function does a lot of the work required in order to check for the visibility of the light from a point on a surface. You may however need to invest a bit of time refactoring it to make it versatile enough to be used in detecting the existence of shadows.

When comparing two triangles for equality, use their index value as the basis for comparison (i.e. their position in the triangle array). If you attempt to compare two triangles using the `==` operator, you may just be testing the memory address of those variables, rather than checking to see if those two variables hold the same triangle !  


# 
### Task 6: Interactive Renderer Switching


Add some additional key event handlers to your program that allows the user to switch between the three main modes of rendering:

- Wireframe: Stroked/Outline triangles
- Rasterised: Filled Triangles drawn using your rasterising code
- Raytraced: Filled Triangles drawn using your brand-new raytracing code

This will be very useful later on as it will allow you to navigate the camera around the scene using the FAST wireframe renderer and then switch to the (much slower) ray traced renderer to view the fully lit scene from a particular angle (great for testing !)  

  


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