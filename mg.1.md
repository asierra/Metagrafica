% mg(1)
% Alejandro Aguilar Sierra <algsierra@gmail.com>
% 2024

# NAME

mg -  Metagrafica Descriptive language to produce high quality graphics.


# SYNOPSIS

**mg** [**-h**] *filename.mg*

# GENERAL OPTIONS

**-h**, **--help**
:   Display this friendly help message.

# DESCRIPTION
 
MetaGrafica **mg** is a descriptive language to create 2D vector graphics of publication quality as Encapsulated PostScript. Being vectorial, the basic element is not a pixel but a *point*, defined by a pair of coordinates x, y. A series of points creates a *path*, with which we can build polygons, curves and text, which we call *graphics primitives*. You can assign atributes to the primitives, like color and line width, and also geometric transformations, like rotations and scale. With a set of primitives you can build higher level structures, which are also controlled with linear transformations.

The output area is defined in centimeters and is by default 10x10 but you can change it using the directive $D.
The _object space_ is the reference system defined by the user with the command WW, being 0 to 1 in both horizontal and vertical directions the default. If you want to change the defaults, you must use both $D and WW at the begining of the program. 

## Graphics primitives

Every primitive is defined by its name, which usually is a short mnemonic, followed by numeric arguments. In the following list we describe them with name, syntaxis and abrief explanation. A path is defined by a series of points separated by blank spaces and ending with a closing }. A typographic point is 1/72 inch. 
 
`PL path`
:  Polyline. Join all points of the path with straight lines.
 
`CR r [dq [q0]] : path`
:  Circles or arcs. Draws circles or arcs of `r` radius, optional `dq` wide (in degrees) and optional initial angle `q0`, centered in every point of the path. By default, `dq` is 360 and `q0` is 0.
 
`EL rx ry [dq[ q0]] : path`
:  Ellipses. Like **CR** but with horizontal *rx*  and vertical *ry* radius.

`BR path` 
:  Polyrectangle. A bar defined by a couple of points, one for the left lower corner and the other for the upper right corner. The path must have an even number of points. 
 
`PG path`
:  Filled polygon. Like **PL** but its interior will be filled with a color or pattern. 

`DOT r path`
:  Black circles with radius *r* defined in typographic points centered in every point of the path.
  
`BZ path`
:  Bezier curve. Uses the path to define segments of a bezier curve. Every segment needs four points, the first and the last are in the curve and the second and third are the corresponding tangent local vectors to those points.
  
## Graphics state

The graphics state manages properties that are used when the graphics is printed or displayed. The first type are attributes and the second are geometric transformations. The attributes has an integer number as argument. Every command that starts with the character L is for lines, F for filling and T for text.

`LPATRN n`
:  Line patterns like points and dashes. There is a number of predefined patterns.

`LWIDTH n`
:   Line width in units of 0.2 typographic points.

`FPATRN n`
:  Filling patterns. There is a number of predefined patterns. A negative number indicates to draw the contour.

`FGRAY n`
:   Gray level for filling, between 0 for black to 100 for white. A negative number indicates to draw the contour with the current line style. 

`FCOLOR s`
:   Color for filling, as a string which can be the color name or six letters HTML RGB color code, like 000000 for black and FFFFFF for white. A preceding *-* indicates to draw the contour with the current line style. 

`LGRAY n`
:   Gray level for lines, 0 for black to 100 for white.    

`LCOLOR s`
:   Color for filling, as a string which can be the color name or six letters HTML RGB color code, like 000000 for black and FFFFFF for white.

`TSIZE n`
:   Text size in units of typographic points.

`TALIGN n`
:  To align the text, with 0 to left (default), 1 to center, 2 to right.

`TSTYLE style`
:  Set the style for text with one or more of these keywords, in lower case: roman, sanserif, courier, bold or italic.

`FILL`
:  To fill all the next closed paths; no efect over the previous ones.

`NOFILL`
:  To stop filling the next closed paths. 

## Linear transformations

Internally we use 3D homogeneus coordinate matrices to join every linear transformation in a single matrix by matrix product. The two letters prefix is the operation and the two letters suffix is the corresponding matrix. We support the following user defined transformations:

`RTMT theta`
:  Rotate by *theta* degrees.

`SCMT sx sy`
:  Scale by *sx* in the X axis and *sy* in the Y axis.

`TLMT tx ty`
:  Translate to the point *tx ty*.

`IDMT`
:  Initialize the matrix with the identity. 

The suffix *MT* can be replaced by the following supported matrices:

`LC`
:  Local matrix applied in the first level.

`ST`
:  Applied for structures.
                  	
`PP`
:  Applied for the current point (plume position). 
 
 
## Structures

A _structure_ allows to associate primitives, attributes and matrices to create more complex graphics objects. Each structure has a user defined name.

`OPST name`
:  To create a structure you need to open a new one and set its name. Every command inside an open structure will be part of the structure.

`CLST`
:  To close the previusly opened structure.

`<Name of the structure> path`
:  The named structure will be reproduced at all points of the path, using the ST matrix for rotation and scale.

`MKST name`
:  To assign a structure of this *name* as the _marker_ for the following operations.

`PWST x1 y1 x2 y2`
:  Port window structure. Insert the marker exactly in the rectangle defined by the two points.

`LNST sc [shift]: x1 y1 x2 y2`
:  Draw a line and put the marker of scale *sc* at the end of the second point. The structure is rotated acording to the line inclination. If the scale is negative, both sides are used. An optional parameter indicates a shift from the edges.

`ARCST sc r dq q0 [shift]: x y`
:  Draw an arc of radius *r*, wide *dq* and initial angle *q0* and put the marker of scale *sc* at the end of the arc, centered at the point *x y*. The structure is rotated according to the angle. If the scale is negative, both sides are used. An optional parameter indicates a shift from the edges.

## Controls
 
`$D dx dy`
:  Dimensions of the display area in centimeters (by default 10 10).

`WW Xmin Xmax Ymin Ymax`
:  To define the user space limits, or *World Window*. Every user defined coordinates will be inside these limits and they correspond to the complete display area of the graphics (default 0 1 0 1). It is a good practice that *WW* uses the same aspect ratio than *$D*.
 
`$P n`
:  To define the text size in typographic points (default 10). This can be changed inside the program with the attribute *TSIZE*.

`INPUT  name[.mg]`
:  To include another mg file in the current graphics.

Comments
:  After a **%**, MG will ignore the rest of the line.

`EXIT`
:  After this command, the rest of the file will be ignored.

## Generators

The generators produce repetitive primitives.

`GNNUM i0 inc n decimals`
:  Generates a series of *n* numbers and position each one at the current point using the PP matrix, using an initial number *i0*, increment *inc*, number of numbers *n* and number of *decimals*.

`TICKS n x y`
:  Generates parallel lines and position them according with the PP matrix, with the parameters: number of lines *n*, generative vector *x y*.

## Text

Special commands to display text. We use standard PostScript fonts and an optional embeded LaTeX font for Greek and symbols.

`DT text`
:  To display a string of text at the current position, which is updated with the PP matrix every time this command is used.

`XYDT x y text`
:  To display a string of text at the position x y.
 
Inside the string you can insert some commands to get some text efects and control the reach of those effects with *{* and *}*.

^
:  superindex.

_ 
:  subindex.

/b
:  black.

/e
/i
:  emphasized or italics.

//
: Slant bar.
 
\\_symbol_
:  LaTeX symbols like Greek and math, par example \\alpha, \\infty.
  
# EXAMPLES

A simple MG file with a corner, a circle and a message.

    $D 12 8
    WW 0 24 0 16

    PL 12 15  12 8  20 8 }
    CR 6 : 12 8 }

    XYDT 8 10 Hello World!

See more examples in the *examples* directory.

# BUGS

Please report any bugs you find.

# COPYRIGHT
  
License: GPL 3.0
Copyright (c) 2024 Alejandro Aguilar Sierra. All right reserved.