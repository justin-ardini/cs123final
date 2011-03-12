# CS123 Final Project

## Summary:
Our project implements a mountain lake using Qt, OpenGL & GLSL.  It is our final project for CS123: Introduction to Computer Graphics at Brown.

Our project is based on the cs123 lab09 code, and the skybox handling was taken from it, as was the frame buffer handling. We based the terrain generation from the lab07 code, expanded the terrain, and modified it so that the terrain forms the shape of a lake. We implemented bump mapping, depth-of-field, reflection, and refraction. We implemented several shaders.

Our project models water in a mountain environment. We have a fractal-generated terrain in a skybox. Intersecting the terrain is a quad, on which water is modelled.


## Features:

### Bump Mapping:
For bump mapping, we took an image from the Internet (see normal map texture below) and used its colors in the texture as perturbtion normals. These normals were added to the normals of the water quad to alter them a little bit. This way, the water's normals were irregular, making the water ripple. The bump map's location was modified by offset values, and so the bump map is moved through the water quad, giving the appearence of animation.

### Depth of field:
For depth of field, we rendered the scene into a frame buffer, keeping track of depth values by using the alpha component of color. We then bound that to a texture and blurred the scene through a two-pass Gaussian blur (first in the x direction, then in the y). Finally, we interpolated between the initial scene and the blurred scene to obtain the final scene.

### Reflection:
For reflection on water, we rendered the terrain that was above the water level into a frame buffer, which was bound to a texture. Then reflected it about the water plane. Then, we used eye rays hitting the water to determine which point on that texture should be reflected by the water.

### Refraction:
For refraction on water, we rendered the terrain that was below the water level into a frame buffer, which was bound to a texture. The reflected and refracted textures were blended in the water.


## How to use:
The program can be run with the cs123final executable, run with qmake, or can be edited in QtCreator using the .pro file.

The following commands can be used:
* ** D ** - toggles depth-of-field
* ** M ** - displays the depth values
* ** Left ** and ** Right ** arrows - change the focal range of the depth of field shader
* ** Up ** and ** Down ** arrows - change the focal distance of the depth of field shader
* ** O ** and ** P ** - decrease/increase the blur size of the depth of field shader


## Credits:

Depth of Field implementation based on approach in [Encelo's Blog](http://encelo.netsons.org/blog/2008/04/15/depth-of-field-reloaded/).
This code is under General Public License (GPL).

Skybox created by [Hazel Whorley](http://www.hazelwhorley.com/textures.html).

Normal map texture taken from <http://i140.photobucket.com/albums/r3/kwistenbiebel1/water01_bumpmap.jpg>.

Special thanks to the CS123 TAs for being awesome and helping
with numerous bugs!


## License:

Copyright (C) 2010  Justin Ardini and Hobart Reynolds

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
