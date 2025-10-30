# ParaView script to save images from Bekaert virtual twins
# Script checked with paraview version 5.9.1
#
# Usage:
#
#     pvbatch \
#         ./templateCase/paraviewScriptImages.py \
#         --pass-directory ./rolling2 \
#         --virtual-twin-name "Agrafe Clip"
#
# where the provide virtual twin name is added as a text box.
# Optionally an integer "--number-of-frames" can be specified, where the default
# value is 30.
#
# Author: Philip Cardiff, UCD
# Date: July 2021

print("****************************************************")
print("* Bekaert virtual twins paraview script            *")
print("* Author: philip.cardiff@ucd.ie                    *")
print("****************************************************")


################################################################################
# Module import
################################################################################
from paraview.simple import *
from datetime import datetime
import argparse
import sys
import os
import shutil


################################################################################
# Global variables
################################################################################

# Resolution of saved images and animation frames
IMAGE_RES_X = 2*1584
IMAGE_RES_Y = 2*932

# Default number of animation frames
NUM_FRAMES = 30

################################################################################
# Function definition
################################################################################

def set_2D_view(zoom):
    """
    This function sets the view to a 2D slice
    """
    # 2D camera settings
    renderView1.InteractionMode = '2D'
    renderView1.CameraPosition = [-1, 0.0, 0.0]
    renderView1.CameraFocalPoint = [0.0, 0.0, 0.0]
    renderView1.CameraViewUp = [0.0, 1.0, 0.0]
    renderView1.CameraParallelScale = zoom


def set_3D_view(zoom):
    """
    This function sets the view to a 3D perspective
    """
    # 3D camera settings
    renderView1.InteractionMode = '3D'
    renderView1.CameraPosition = [13*zoom, 4*zoom, 7*zoom]
    renderView1.CameraFocalPoint = [0.0, 0.0, 0.0]
    renderView1.CameraViewUp = [0.0, 1.0, 0.0]


def save_image(name_prefix, image_dir):
    """
    This function save a screenshot of the current view
    """
    # Save screenshot
    image_addr = image_dir + '/' + name_prefix + '.png'
    print("Saving 2D screenshot for slice at x = 0: " + image_addr)
    SaveScreenshot(image_addr, renderView1, ImageResolution=[2*1584, 2*932])


def save_animation_frames(name_prefix, animation_dir, slice1, slice_start,
        slice_incr, num_frames):
    """
    This function save a series of animation frames (images) of a moving slice
    """
    # Save files for animation
    # Move the slice in the x direction based on the wire dimensions
    print("Saving frames for animation: " + animation_dir + '/' +  name_prefix
          + '*.png')
    slice_org = slice1.SliceOffsetValues[0]
    slice_cur = slice_start
    for i in range(1, (num_frames + 1)):
        slice1.SliceOffsetValues = [slice_cur]
        slice_cur += slice_incr
        SaveScreenshot(animation_dir + '/' + name_prefix + '_' + str(i) + '.png',
                       renderView1, ImageResolution=[IMAGE_RES_X, IMAGE_RES_Y])

    # Reset slice position
    slice1.SliceOffsetValues = [slice_org]


def set_colour_of_wire_and_tools_colour(
        field, display_name, wireDisplay, toolsDisplay,
        colour_wire_by_field, colour_tool_by_field, solidColour, solidOpacity,
        renderView1, scale_min, scale_max, label_format):
    """
    This function colours the wire and tools with the requested settings, along
    with related display settings like opactiy and the legend 
    """

    wireDisplay.SetScalarBarVisibility(renderView1, False)
    toolsDisplay.SetScalarBarVisibility(renderView1, False)

    # Set wire and tool colours
    if colour_wire_by_field:
        ColorBy(wireDisplay, ('POINTS', field))
        wireDisplay.Opacity = 1.0
        wireDisplay.SetScalarBarVisibility(renderView1, True)
    else:
        # It is an error to set to None if the field is already None so as a hack
        # we just set it to the field first and then to None
        ColorBy(wireDisplay, ('POINTS', field))
        ColorBy(wireDisplay, None)
        wireDisplay.Opacity = solidOpacity
        wireDisplay.AmbientColor = solidColour
        wireDisplay.DiffuseColor = solidColour

    if colour_tool_by_field:
        ColorBy(toolsDisplay, ('POINTS', field))
        toolsDisplay.Opacity = 1.0
        toolsDisplay.SetScalarBarVisibility(renderView1, True)
    else:
        # It is an error to set to None if the field is already None so as a hack
        # we just set it to the field first and then to None
        ColorBy(toolsDisplay, ('POINTS', field))
        ColorBy(toolsDisplay, None)
        toolsDisplay.Opacity = solidOpacity
        toolsDisplay.AmbientColor = solidColour
        toolsDisplay.DiffuseColor = solidColour

    # Set the colour scheme and scale
    fieldLUT = GetColorTransferFunction(field)
    fieldPWF = GetOpacityTransferFunction(field)
    fieldLUTColorBar = GetScalarBar(fieldLUT, renderView1)

    # Change scalar bar placement
    fieldLUTColorBar.WindowLocation = 'AnyLocation'
    fieldLUTColorBar.Orientation = 'Horizontal'
    fieldLUTColorBar.Position = [0.04, 0.9]
    fieldLUTColorBar.ScalarBarLength = 0.2

    # Properties modified on fieldLUTColorBar
    #fieldLUTColorBar.Title = 'Plastic strain [-]'
    fieldLUTColorBar.Title = display_name
    fieldLUTColorBar.TitleColor = [0.0, 0.0, 0.0]
    fieldLUTColorBar.LabelColor = [0.0, 0.0, 0.0]
    fieldLUTColorBar.AutomaticLabelFormat = 0
    fieldLUTColorBar.LabelFormat = label_format
    fieldLUTColorBar.AddRangeLabels = 0
    fieldLUTColorBar.DrawTickMarks = 0
    fieldLUTColorBar.DrawTickLabels = 0
    fieldLUTColorBar.AddRangeLabels = 1
    fieldLUTColorBar.RangeLabelFormat = label_format
    fieldLUTColorBar.TitleFontSize = 50
    fieldLUTColorBar.LabelFontSize = 50
    fieldLUTColorBar.TitleFontFamily = 'Courier'
    fieldLUTColorBar.LabelFontFamily = 'Courier'
    fieldLUTColorBar.ComponentTitle = ''
    fieldLUT.ApplyPreset('Rainbow Uniform', True)
    fieldLUT.Discretize = 1
    fieldLUT.NumberOfTableValues = 15

    # Set scale range
    fieldLUT.RescaleTransferFunction(scale_min, scale_max)
    fieldPWF.RescaleTransferFunction(scale_min, scale_max)


def create_glyphs(inputDomain, field, scaleFactor, renderView1):
    """
    This function creates glyphs of the given field and returns the glyphs
    and glyphsDisplay objects. The glyphs are visible by default.
    """

    glyphs1 = Glyph(Input=inputDomain, GlyphType='Arrow')
    glyphs1.OrientationArray = ['POINTS', field]
    glyphs1.ScaleArray = ['POINTS', field]
    glyphs1.ScaleFactor = scaleFactor
    glyphs1.GlyphMode = 'All Points'
    glyphs1Display = Show(glyphs1, renderView1, 'GeometryRepresentation')
    ColorBy(glyphs1Display, ('POINTS', field, 'Magnitude'))
    glyphs1Display.SetScalarBarVisibility(renderView1, True)

    return glyphs1, glyphs1Display
 

################################################################################
#
# MAIN PROGRAM
#
################################################################################


################################################################################
# Read arguments
################################################################################

parser = argparse.ArgumentParser()

parser.add_argument('--pass-directory', dest='pass_directory',
                    help='relative address of the pass directory', type=str,
                    required=True)

parser.add_argument('--virtual-twin-name', dest='virtual_twin_name',
                    help='name used in text box', type=str, required=True)

parser.add_argument('--number-of-frames', dest='num_frames',
                    help='number of frames creation for the animations box',
                    default=NUM_FRAMES, type=int, required=False)

args = parser.parse_args()


################################################################################
# Create directories for the images and animation frames
################################################################################

# Create new directory for the image files
image_dir = args.pass_directory + '/images'
animation_dir = args.pass_directory + '/animations'
if os.path.exists(image_dir) and os.path.exists(animation_dir):
    print("The image directory " + image_dir + " and the animation directory " + 
          animation_dir + " already exist: skipping")
    quit()

# It is possible that either the image_dir or animation_dir exists, in that case
# we remove them
if os.path.exists(image_dir):
    print("Removing directory: " + image_dir)
    shutil.rmtree(image_dir)

if os.path.exists(animation_dir):
    print("Removing directory: " + animation_dir)
    shutil.rmtree(animation_dir)

# Create new directories
print("Creating directory: " + image_dir)
os.mkdir(image_dir)

print("Creating directory: " + animation_dir)
os.mkdir(animation_dir)


################################################################################
# Read data
################################################################################

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

case_file = args.pass_directory + '/case.foam'
print("Reading " + case_file)
casefoam = OpenFOAMReader(FileName=case_file)
casefoam.SkipZeroTime = 0
casefoam.MeshRegions = ['internalMesh']
casefoam.CellArrays = ['epsilonPEq', 'materials', 'sigmaMax', 'totalTraction']
casefoam.Decomposepolyhedra = 1


################################################################################
# Set up view objects
################################################################################

# get animation scene
animationScene1 = GetAnimationScene()

# get the time-keeper
timeKeeper1 = GetTimeKeeper()

# update animation scene based on data timesteps
animationScene1.UpdateAnimationUsingDataTimeSteps()
animationScene1.GoToFirst()
animationScene1.PlayMode = 'Snap To TimeSteps'

# get active view and set size
renderView1 = GetActiveViewOrCreate('RenderView')
renderView1.ViewSize = [IMAGE_RES_X, IMAGE_RES_Y]

# set background colour
renderView1.Background = [1.0, 1.0, 1.0]

# Hide orientation axes
renderView1.OrientationAxesVisibility = 0


################################################################################
# Set time time-step
################################################################################

# Set the time to the 3rd last time-step
if len(timeKeeper1.TimestepValues) < 3:
    timeKeeper1.Time = len(timeKeeper1.TimestepValues)
else:
    timeKeeper1.Time = timeKeeper1.TimestepValues[-3]
print("Setting the time to " + str(timeKeeper1.Time))


################################################################################
# Create a slice
################################################################################

# create a new 'Slice'
print("Creating a slice through the geometry")
slice1 = Slice(Input=casefoam)
slice1.SliceType = 'Plane'
slice1.HyperTreeGridSlicer = 'Plane'
slice1.SliceOffsetValues = [0.0]

# init the 'Plane' selected for 'SliceType'
slice1.SliceType.Origin = [0.0, 0.0, 0.0]

# show data in view
slice1Display = Show(slice1, renderView1, 'GeometryRepresentation')

# hide data in view
Hide(casefoam, renderView1)

# toggle 3D widget visibility (only when running from the GUI)
Hide3DWidgets(proxy=slice1.SliceType)

#change interaction mode for render view
renderView1.InteractionMode = '2D'


################################################################################
# Create reflection of slice if there are symmetry planes
################################################################################

sliceAll = slice1
casefoamAll = casefoam

if 'wireSymmetryY' in casefoam.MeshRegions.Available:
    print("Reflecting the geometry in the Y direction")
    sliceAll = Reflect(Input=sliceAll)
    sliceAll.Plane = 'Y Min'
    casefoamAll = Reflect(Input=casefoam)
    casefoamAll.Plane = 'Y Min'

if 'wireSymmetryZ' in casefoam.MeshRegions.Available:
    print("Reflecting the geometry in the Z direction")
    sliceAll = Reflect(Input=sliceAll)
    sliceAll.Plane = 'Z Min'
    casefoamAll = Reflect(Input=casefoam)
    casefoamAll.Plane = 'Z Min'

sliceAllDisplay = Show(sliceAll, renderView1, 'UnstructuredGridRepresentation')
sliceAllDisplay.Representation = 'Surface'

casefoamAllDisplay = Show(casefoamAll, renderView1, 'UnstructuredGridRepresentation')
casefoamAllDisplay.Representation = 'Surface'

# hide data in view
Hide(casefoam, renderView1)
Hide(slice1, renderView1)
Hide(sliceAll)
Hide(casefoamAll)


################################################################################
# Threshold wire
################################################################################

print("Extracting the wire")

# 2D
wire = Threshold(Input=sliceAll)
wire.Scalars = ['CELLS', 'materials']
wire.ThresholdRange = [0.0, 0.5]
wireDisplay = Show(wire, renderView1, 'UnstructuredGridRepresentation')

# 3D
wire3D = Threshold(Input=casefoamAll)
wire3D.Scalars = ['CELLS', 'materials']
wire3D.ThresholdRange = [0.0, 0.5]
wire3DDisplay = Show(wire3D, renderView1, 'UnstructuredGridRepresentation')

# Hide 3D
Hide(wire3D)


################################################################################
# Calculate the camera zoom and animation frame slice positions
################################################################################
bounds = wire.GetDataInformation().GetBounds()
wireMaxYZDim = max(abs(bounds[2] - bounds[3]), abs(bounds[4] - bounds[5]))
zoom = 0.6*wireMaxYZDim
#renderView1.CameraParallelScale = zoom

# Slice positions for the animation
slice_start = -2.0*wireMaxYZDim
slice_end = 1.5*wireMaxYZDim
slice_range = slice_end - slice_start
slice_incr = slice_range/float(args.num_frames)


################################################################################
# Threshold tools
################################################################################

print("Extracting the tools")

# 2D
tools = Threshold(Input=sliceAll)
tools.Scalars = ['CELLS', 'materials']
tools.ThresholdRange = [0.5, 100.0]
toolsDisplay = Show(tools, renderView1, 'UnstructuredGridRepresentation')

# 3D
tools3D = Threshold(Input=casefoamAll)
tools3D.Scalars = ['CELLS', 'materials']
tools3D.ThresholdRange = [0.5, 100.0]
tools3DDisplay = Show(tools3D, renderView1, 'UnstructuredGridRepresentation')

# Hide 3D
Hide(tools3D)


################################################################################
# Create feature edges for slice
################################################################################

print("Extracting the feature edges")
featureEdges1 = FeatureEdges(Input=slice1)
featureEdges1Display = Show(featureEdges1, renderView1, 'GeometryRepresentation')
Hide(featureEdges1)

featureEdgesAll = featureEdges1

if 'wireSymmetryY' in casefoam.MeshRegions.Available:
    featureEdgesAll = Reflect(Input=featureEdgesAll)
    featureEdgesAll.Plane = 'Y Min'

if 'wireSymmetryZ' in casefoam.MeshRegions.Available:
    featureEdgesAll = Reflect(Input=featureEdgesAll)
    featureEdgesAll.Plane = 'Z Min'

featureEdgesAllDisplay = Show(featureEdgesAll, renderView1, 'UnstructuredGridRepresentation')
featureEdgesAllDisplay.DiffuseColor = [0.0, 0.0, 0.0]


################################################################################
# Add text box
################################################################################

print("Creating the text information box")
text1 = Text()
text1Display = Show(text1, renderView1, 'TextSourceRepresentation')
text1Display.FontSize = 50
text1Display.Color = [0.0, 0.0, 0.0]
if args.virtual_twin_name == '':
    text1.Text = datetime.now().strftime("%d-%m-%Y") + "\n" \
        + "UCD/Bekaert"
else:
    text1.Text = "Virtual twin: " + args.virtual_twin_name + "\n" \
        + datetime.now().strftime("%d-%m-%Y") + "\n" \
        + "UCD/Bekaert"
text1Display.FontFamily = 'Courier'
text1Display.WindowLocation = 'AnyLocation'
text1Display.Position = [0.01, 0.01]


################################################################################
# Creating bounding rulers for the wire slice
################################################################################

print("Adding bounding rulers")
boundingRuler1 = BoundingRuler(Input=wire)
boundingRuler1.Axis = 'Z Axis'
boundingRuler1Display = Show(boundingRuler1, renderView1, 'RulerSourceRepresentation')
boundingRuler1Display.AxisColor = [0.5000076295109483, 0.0, 0.5000076295109483]
boundingRuler1Display.AxisLineWidth = 4
boundingRuler1Display.RulerMode = 1
boundingRuler1Display.Scale = 1000.0
boundingRuler1Display.Graduation = 1
boundingRuler1Display.LabelFormat = '%6.3g mm'
boundingRuler1Display.Color = [0.0, 0.0, 0.0]
boundingRuler1Display.FontSize = 50

boundingRuler2 = BoundingRuler(Input=wire)
boundingRuler2.Axis = 'Y Axis'
boundingRuler2Display = Show(boundingRuler2, renderView1, 'RulerSourceRepresentation')
boundingRuler2Display.AxisColor = [0.5000076295109483, 0.0, 0.5000076295109483]
boundingRuler2Display.AxisLineWidth = 4
boundingRuler2Display.RulerMode = 1
boundingRuler2Display.Scale = 1000.0
boundingRuler2Display.Graduation = 1
boundingRuler2Display.LabelFormat = '%6.3g mm'
boundingRuler2Display.Color = [0.0, 0.0, 0.0]
boundingRuler2Display.FontSize = 50


################################################################################
# Save screenshot and animation frames - epsilonPEq
################################################################################

# Slice views
set_colour_of_wire_and_tools_colour(
    "epsilonPEq", "Plastic Strain [-]", wireDisplay, toolsDisplay,
    True, False, [1.0, 1.0, 0.4980392156862745], 0.3, renderView1,
    0.0, 2.0, '%-#6.1f')

set_2D_view(zoom)
save_image("epsilonPEq", image_dir)
save_animation_frames("epsilonPEq", animation_dir, slice1, slice_start,
        slice_incr, args.num_frames)

set_3D_view(zoom)
save_image("epsilonPEq_3D_slice", image_dir)

# Full 3D view
Hide(wire); Hide(tools); Hide(featureEdgesAll)
Hide(boundingRuler1); Hide(boundingRuler2)
Show(wire3D); Show(tools3D)
set_colour_of_wire_and_tools_colour(
    "epsilonPEq", "Plastic Strain [-]", wire3DDisplay, tools3DDisplay,
    True, False, [1.0, 1.0, 1.0], 0.1, renderView1,
    0.0, 2.0, '%-#6.1f')

set_3D_view(zoom)

tools3DDisplay.SetRepresentationType('Surface')
save_image("epsilonPEq_3D", image_dir)
tools3DDisplay.SetRepresentationType('Wireframe')
tools3DDisplay.AmbientColor = [0, 0, 0]
save_image("epsilonPEq_3D_mesh", image_dir)

Show(wire); Show(tools); Show(featureEdgesAll)
Show(boundingRuler1); Show(boundingRuler2)
Hide(wire3D); Hide(tools3D)


################################################################################
# Save screenshot and animation frames - sigmaMax tools
################################################################################

# Slice views
set_colour_of_wire_and_tools_colour(
    "sigmaMax", "Max Principal Stress [Pa]", wireDisplay, toolsDisplay,
    False, True, [1.0, 1.0, 0.4980392156862745], 0.3, renderView1,
    -1e9, 1e9, '%-#6.1e')

set_2D_view(zoom)
save_image("sigmaMax", image_dir)
save_animation_frames("sigmaMax", animation_dir, slice1, slice_start,
        slice_incr, args.num_frames)

set_3D_view(zoom)
save_image("sigmaMax_3D_slice", image_dir)

# Full 3D view
Hide(wire); Hide(tools); Hide(featureEdgesAll)
Hide(boundingRuler1); Hide(boundingRuler2)
Show(wire3D); Show(tools3D)
set_colour_of_wire_and_tools_colour(
    "sigmaMax", "Max Principal Stress [Pa]", wire3DDisplay, tools3DDisplay,
    True, False, [1.0, 1.0, 1.0], 0.1, renderView1,
    -1e9, 1e9, '%-#6.1e')

set_3D_view(zoom)

tools3DDisplay.SetRepresentationType('Surface')
save_image("sigmaMax_3D", image_dir)
tools3DDisplay.SetRepresentationType('Wireframe')
tools3DDisplay.AmbientColor = [0, 0, 0]
save_image("sigmaMax_3D_mesh", image_dir)

Show(wire); Show(tools); Show(featureEdgesAll)
Show(boundingRuler1); Show(boundingRuler2)
Hide(wire3D); Hide(tools3D)


################################################################################
# Save screenshot and animation frames - contactTraction
################################################################################

# Slice views
set_colour_of_wire_and_tools_colour(
    "totalTraction", "Contact Traction [Pa]", wireDisplay, toolsDisplay,
    False, False, [1.0, 1.0, 0.4980392156862745], 0.3, renderView1,
    0, 2.6e9, '%-#6.1e')

# Create and show glyphs
glyphs1, glyphs1Display = create_glyphs(tools, "totalTraction", 1e-12, renderView1)

set_2D_view(zoom)
save_image("contactTraction", image_dir)
save_animation_frames("contactTraction", animation_dir, slice1, slice_start,
        slice_incr, args.num_frames)

set_3D_view(zoom)
save_image("contactTraction_3D_slice", image_dir)

# Delete the glyphs
Hide(glyphs1)
del glyphs1
del glyphs1Display

# Full 3D view
Hide(wire); Hide(tools); Hide(featureEdgesAll)
Hide(boundingRuler1); Hide(boundingRuler2)
Show(wire3D); Show(tools3D)
set_colour_of_wire_and_tools_colour(
    "totalTraction", "Contact Traction [Pa]", wire3DDisplay, tools3DDisplay,
    True, False, [1.0, 1.0, 1.0], 0.1, renderView1,
    0, 2.6e9, '%-#6.1e')

set_3D_view(zoom)

tools3DDisplay.SetRepresentationType('Surface')
save_image("contactTraction_3D", image_dir)
tools3DDisplay.SetRepresentationType('Wireframe')
tools3DDisplay.AmbientColor = [0, 0, 0]
save_image("contactTraction_3D_mesh", image_dir)

Show(wire); Show(tools); Show(featureEdgesAll)
Show(boundingRuler1); Show(boundingRuler2)
Hide(wire3D); Hide(tools3D)


################################################################################
# Save screenshot and animation frames - mesh
################################################################################

# Slice views
Hide(tools)
Hide(wire)
Hide(featureEdgesAll)
casefoam.Decomposepolyhedra = 0
slice1.Triangulatetheslice = 0
sliceAllDisplay = Show(sliceAll, renderView1, 'UnstructuredGridRepresentation')
sliceAllDisplay.SetRepresentationType('Wireframe')
sliceAllDisplay.AmbientColor = [0, 0, 0]

set_2D_view(zoom)
save_image("mesh", image_dir)
save_animation_frames("mesh", animation_dir, slice1, slice_start,
        slice_incr, args.num_frames)

set_3D_view(zoom)
save_image("mesh_3D", image_dir)

# Full 3D view
Hide(sliceAll); Hide(boundingRuler1); Hide(boundingRuler2)
Show(wire3D); Show(tools3D)

set_3D_view(zoom)

ColorBy(wire3DDisplay, None)
wire3DDisplay.SetScalarBarVisibility(renderView1, False)
wire3DDisplay.SetRepresentationType('Wireframe')
wire3DDisplay.AmbientColor = [0, 0, 0]
wire3DDisplay.DiffuseColor = [0, 0, 0]
wire3DDisplay.Opacity = 0.1
ColorBy(tools3DDisplay, None)
tools3DDisplay.SetScalarBarVisibility(renderView1, False)
tools3DDisplay.SetRepresentationType('Wireframe')
tools3DDisplay.AmbientColor = [0, 0, 0]
tools3DDisplay.DiffuseColor = [0, 0, 0]
tools3DDisplay.Opacity = 0.1
save_image("mesh_3D", image_dir)


################################################################################
# Print instructions on how to create the animations
################################################################################
print("\n")
print("****************************************************")
print("The animation frames can be converted to a MP4 by\nrunning the " +
      "following command in the animation directory:\n" +
      "for file in *_1.png; do echo ${file}; " +
      "filebase=$(basename \"${file}\" _1.png) ; ffmpeg -framerate 6 " +
      "-i ${filebase}_%d.png ${filebase}.mp4; done")
print("****************************************************")
print("\n")
