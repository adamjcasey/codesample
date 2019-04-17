#!/usr/bin/env python
#
# Copyright 2017 Continuum
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   a. Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer.
#   b. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#   c. Neither the name of Continuum nor the names of its contributors
#      may be used to endorse or promote products derived from this software
#      without specific prior written permission.
#
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Created by Adam Casey 2017

"""Creates .cc/.h files for a particular True Type font.

These are stored in the include/font and src/font directories.

These take as an argument a *.ttx file.
"""

import untangle
import copy
import sys


class GUIVectorPoint:
    def __init__(self, type="", control_x1=0.0, control_y1=0.0, control_x2=0.0, control_y2=0.0, end_x=0.0, end_y=0.0):
        self.type = type
        self.control_x1 = control_x1
        self.control_y1 = control_y1
        self.control_x2 = control_x2
        self.control_y2 = control_y2
        self.end_x = end_x
        self.end_y = end_y


class GlyphInfo:
    def __init__(self, name="nullptr", code=0, width=0, glyphpoints=[]):
        self.name = name
        self.code = code
        self.width = width
        self.glyphpoints = glyphpoints


def getXandY(point1, point2):
    if int(point1['on']) == 1:
        startx = float(point1['x'])
        starty = float(point1['y'])
    else:
        startx = (float(point1['x']) + float(point2['x'])) / 2.0
        starty = (float(point1['y']) + float(point2['y'])) / 2.0

    return (startx, starty)


# Extract the font name
fontname = sys.argv[1].rsplit('.', 1)[0].replace("-", "")
print fontname

# Create a dictionary of GlyphInfo objects for the ASCII characters
# and extend to get the degree symbol (0x20-0xB0)
glyphs = {}
doc = untangle.parse(sys.argv[1])
for m in doc.ttFont.cmap.cmap_format_4[0].map:
    # Only use glyphs in the printable ASCII range
    if 0x20 <= int(m["code"], 0) <= 0xB0:
        glyphs[m["name"]] = GlyphInfo(name=m["name"], code=int(m["code"], 0))

# Add the width from the mtx table
for glyphname, glyphinfo in glyphs.iteritems():
    for mtx in doc.ttFont.hmtx.mtx:
        if mtx["name"] == glyphname:
            glyphs[glyphname].width = int(mtx["width"])

# Add the curve info from the glyph table
print "Number of TTGlyph: %d" % (len(doc.ttFont.glyf.TTGlyph))
for glyphname, glyphinfo in glyphs.iteritems():
    # We need to determine the baseline height for scaling purposes.  For this, we will use the
    # capital letter Z, and determine the height from ymin and ymax
    if glyphname == 'Z':
        fontheight = int(g["yMax"]) - int(g["yMin"])
    for g in doc.ttFont.glyf.TTGlyph:

        # Find the Glyph Information for each glyph in our ascii table
        if g["name"] == glyphname:

            # Add the START font point
            glyphs[glyphname].glyphpoints = []
            glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::START"))
            num_contour = len(g) - 1    # remove the 'instructions' node

            if num_contour > 0:
                for contour in g.contour:
                    num_pts = len(contour)

                    # If the first point is an ON, then no problem, it's a MOVE.  If it's an OFF,
                    # then the move has to be between the midpoint of this and num_pts-1
                    (startx, starty) = getXandY(contour.pt[0], contour.pt[num_pts-1])
                    glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::MOVE", end_x=startx, end_y=starty))

                    # If off, we need to add a curve using this point
                    if int(contour.pt[0]['on']) == 0:
                        (endx, endy) = getXandY(contour.pt[0], contour.pt[1])
                        x = float(contour.pt[0]['x'])
                        y = float(contour.pt[0]['y'])
                        glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::CURVE_Q", end_x=endx, end_y=endy, control_x1=x, control_y1=y))

                    # Go through the rest of the points
                    for n in range(1, num_pts):
                        x = float(contour.pt[n]['x'])
                        y = float(contour.pt[n]['y'])
                        on = int(contour.pt[n]['on'])

                        # If the point is ON, then it's a LINE
                        if on == 1:
                            glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::LINE", end_x=x, end_y=y))

                        # If the point is OFF, then it's a control point for a quadratic bezier
                        else :
                            # If the next point is ON, then no problem, use that
                            # If the next point is also OFF, then use the midpoint
                            m = n+1
                            if m == num_pts:
                                m = 0

                            (startx, starty) = getXandY(contour.pt[n], contour.pt[m])
                            glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::CURVE_Q", end_x=startx, end_y=starty, control_x1=x, control_y1=y))

                    # Close the contour
                    glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::CLOSE"))

            # End the glyph
            glyphs[glyphname].glyphpoints.append(GUIVectorPoint(type="GUIVectorPointType::EXIT"))

# Determine the file and class strings
classname = "Font" + fontname

table = [GlyphInfo() for i in range(256)]

for glyphname, glyphinfo in glyphs.iteritems():
    table[glyphinfo.code] = copy.deepcopy(glyphinfo)


##------------------------------------------------------------------------------------
## Write the files

license = """/*------------------------------------------------------------------------------
 Copyright 2017 Continuum

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  a. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  b. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  c. Neither the name of Continuum nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

Created by Adam Casey 2017
------------------------------------------------------------------------------*/
"""

# Print the .cc file
f = open("../../../src/assets/" + classname + ".cc", "w")
f.write(license + "\n\n")
f.write("#include \"include/assets/" + classname + ".h\"\n\n")

# Print each glyph
for glyphname, glyphinfo in glyphs.iteritems():
    f.write("GUIVectorPoint %s::%s_data_[] = \n" % (classname, glyphinfo.name))
    f.write("{\n")
    for point in glyphinfo.glyphpoints:
         f.write("    GUIVectorPoint(%s, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f, %0.2f),\n" % (point.type, point.control_x1, point.control_y1, point.control_x2, point.control_y2, point.end_x, point.end_y))
    f.seek(-3, 1)
    f.write("\n};\n\n")

# Print the main table
f.write("GUIFontGlyph %s::glyphs_[] = \n" % (classname))
f.write("{\n")
for x in range(0, 256):
    if table[x].name == "nullptr":
        f.write("    GUIFontGlyph(),\n")
    else:
        f.write("    GUIFontGlyph(%d, %s),\n" % (table[x].width, table[x].name + "_data_"))
f.seek(-3, 1)
f.write("\n};\n\n")

f.write("double %s::height_ = %f;\n" % (classname, fontheight))

f.close()

# Print the .h file

f = open("../../../include/assets/" + classname + ".h", "w")
f.write(license + "\n\n")
f.write("#include \"include/gui_font.h\"\n\n")

f.write("class " + classname)
classdefinition = """
{
 public:
        static GUIVectorPoint * GetVectorDataForGlyph(char c)
        {
            uint8_t b = static_cast<uint8_t>(c);
            return glyphs_[b].data_;
        }
        static double GetWidthOfGlyph(char c)
        {
            uint8_t b = static_cast<uint8_t>(c);
            return glyphs_[b].width_;
        }
        static double Height() { return height_; }

 private:
        """
f.write(classdefinition)
f.write(classname + "() {}\n")
f.write("        static double height_;\n")
f.write("        static GUIFontGlyph glyphs_[];\n")
for x in range(0, 256):
    if table[x].name != "nullptr":
        f.write("        static GUIVectorPoint %s_data_[];\n" % table[x].name)

f.write("\n};\n\n")
f.close()
