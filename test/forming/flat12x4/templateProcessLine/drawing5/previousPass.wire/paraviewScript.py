# ParaView script to generate Bekaert virtual twin animation for the wire
# plastic strain field
# Script checked with paraview version 5.8.0
# Philip Cardiff, UCD
# 23-Apr-21

################################################################################
# Module import
################################################################################
from paraview.simple import *
from datetime import datetime
import argparse
import sys


################################################################################
# Read arguments
################################################################################

parser = argparse.ArgumentParser()

parser.add_argument('--virtual-twin-name', dest='virtual_twin_name',
                    help='name used in text box', type=str, required=True)

args = parser.parse_args()


################################################################################
# Read data
################################################################################

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

print("Reading ./case.foam")
casefoam = OpenFOAMReader(FileName='./case.foam')
casefoam.SkipZeroTime = 0
casefoam.MeshRegions = ['internalMesh']
casefoam.CellArrays = ['epsilonPEq', 'materials', 'sigmaMax', 'totalTraction']
#casefoam.Decomposepolyhedra = 0


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
renderView1.ViewSize = [1586, 934]

# set background colour
renderView1.Background = [1.0, 1.0, 1.0]

# get layout
layout1 = GetLayout()

# show data in view
casefoamDisplay = Show(casefoam, renderView1, 'UnstructuredGridRepresentation')

# trace defaults for the display properties.
casefoamDisplay.Representation = 'Surface'
casefoamDisplay.ColorArrayName = [None, '']
casefoamDisplay.OSPRayScaleArray = 'cellToRegion'
casefoamDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
casefoamDisplay.SelectOrientationVectors = 'None'
casefoamDisplay.ScaleFactor = 0.00599999986588955
casefoamDisplay.SelectScaleArray = 'None'
casefoamDisplay.GlyphType = 'Arrow'
casefoamDisplay.GlyphTableIndexArray = 'None'
casefoamDisplay.GaussianRadius = 0.00029999999329447744
casefoamDisplay.SetScaleArray = ['POINTS', 'cellToRegion']
casefoamDisplay.ScaleTransferFunction = 'PiecewiseFunction'
casefoamDisplay.OpacityArray = ['POINTS', 'cellToRegion']
casefoamDisplay.OpacityTransferFunction = 'PiecewiseFunction'
casefoamDisplay.DataAxesGrid = 'GridAxesRepresentation'
casefoamDisplay.PolarAxes = 'PolarAxesRepresentation'
casefoamDisplay.ScalarOpacityUnitDistance = 0.002290528368606591
casefoamDisplay.ExtractedBlockIndex = 1

# init the 'PiecewiseFunction' selected for 'OSPRayScaleFunction'
casefoamDisplay.OSPRayScaleFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.23124045133590698, 1.0, 0.5, 0.0]

# reset view to fit data
renderView1.ResetCamera()

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Hide orientation axes
renderView1.OrientationAxesVisibility = 0


################################################################################
# Create a slice
################################################################################

# create a new 'Slice'
print("Creating a slice through the geometry")
slice1 = Slice(Input=casefoam)
slice1.SliceType = 'Plane'
slice1.HyperTreeGridSlicer = 'Plane'
slice1.SliceOffsetValues = [0.0]
#slice1.Triangulatetheslice = 0

# init the 'Plane' selected for 'SliceType'
slice1.SliceType.Origin = [0.0, 0.0, 0.0]
slice1.HyperTreeGridSlicer.Origin = [0.0, 0.0, 0.0]

# show data in view
slice1Display = Show(slice1, renderView1, 'GeometryRepresentation')

# hide data in view
Hide(casefoam, renderView1)

# toggle 3D widget visibility (only when running from the GUI)
Hide3DWidgets(proxy=slice1.SliceType)

# reset view to fit data
renderView1.ResetCamera()

#change interaction mode for render view
renderView1.InteractionMode = '2D'


################################################################################
# Create reflection of slice if there are symmetry planes
################################################################################

sliceAll = slice1

if 'wireSymmetryY' in casefoam.MeshRegions.Available:
    print("Reflecting the slice in the Y direction")
    sliceAll = Reflect(Input=sliceAll)
    sliceAll.Plane = 'Y Min'

if 'wireSymmetryZ' in casefoam.MeshRegions.Available:
    print("Reflecting the slice in the Z direction")
    sliceAll = Reflect(Input=sliceAll)
    sliceAll.Plane = 'Z Min'

sliceAllDisplay = Show(sliceAll, renderView1, 'UnstructuredGridRepresentation')
sliceAllDisplay.Representation = 'Surface'

# hide data in view
Hide(casefoam, renderView1)
Hide(slice1, renderView1)
Hide(sliceAll)


################################################################################
# Threshold wire
################################################################################

print("Extracting the wire")
wire = Threshold(Input=sliceAll)
wire.Scalars = ['CELLS', 'materials']
wire.ThresholdRange = [0.0, 0.5]
wireDisplay = Show(wire, renderView1, 'UnstructuredGridRepresentation')


################################################################################
# Threshold tools
################################################################################

print("Extracting the tools")
tools = Threshold(Input=sliceAll)
tools.Scalars = ['CELLS', 'materials']
tools.ThresholdRange = [0.5, 100.0]

# show data in view
toolsDisplay = Show(tools, renderView1, 'UnstructuredGridRepresentation')


################################################################################
# Create feature edges
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
text1Display.FontSize = 5
text1Display.Color = [0.0, 0.0, 0.0]
text1.Text = "Virtual twin: " + args.virtual_twin_name + "\n" \
    + datetime.now().strftime("%d-%m-%Y") + "\n" \
    + "UCD/Resero/Bekaert"
text1Display.FontFamily = 'Courier'
text1Display.WindowLocation = 'AnyLocation'
text1Display.Position = [0.01, 0.01]


################################################################################
# Creating bounding rulers for the wire
################################################################################

print("Adding bounding rulers")
boundingRuler1 = BoundingRuler(Input=wire)
boundingRuler1.Axis = 'Z Axis'
boundingRuler1Display = Show(boundingRuler1, renderView1, 'RulerSourceRepresentation')
boundingRuler1Display.AxisColor = [0.5000076295109483, 0.0, 0.5000076295109483]
boundingRuler1Display.RulerMode = 1
boundingRuler1Display.Scale = 1000.0
boundingRuler1Display.Graduation = 1
boundingRuler1Display.LabelFormat = '%6.3g mm'
boundingRuler1Display.Color = [0.0, 0.0, 0.0]

boundingRuler2 = BoundingRuler(Input=wire)
boundingRuler2.Axis = 'Y Axis'
boundingRuler2Display = Show(boundingRuler2, renderView1, 'RulerSourceRepresentation')
boundingRuler2Display.AxisColor = [0.5000076295109483, 0.0, 0.5000076295109483]
boundingRuler2Display.RulerMode = 1
boundingRuler2Display.Scale = 1000.0
boundingRuler2Display.Graduation = 1
boundingRuler2Display.LabelFormat = '%6.3g mm'
boundingRuler2Display.Color = [0.0, 0.0, 0.0]


################################################################################
# Create animation of slice
################################################################################

print("Defining the slice animation")

# get animation track
slice1ContourValuesTrack = GetAnimationTrack('ContourValues', index=-1, proxy=slice1)

# create keyframes for this animation track

endtime = timeKeeper1.TimestepValues[-1]
numPasses = int(endtime) + 1

# Store keyframes in a list

allKeyFrames = []

# Extract contact region: we will start the slice slightly before this and end
# it slightly after
casefoamContact = OpenFOAMReader(FileName='./case.foam')
casefoamContact.SkipZeroTime = 0
casefoamContact.MeshRegions = ['wireContact']

# Check normalPressure field exists
if 'normalPressure' not in casefoam.CellArrays.Available:
    sys.exit("\nError:\n"
        "The field normalPressure was not found! "
        "Did the surfaceTractions utility run?")

casefoamContact.CellArrays = ['normalPressure']
casefoamContactDisplay = Show(casefoamContact, renderView1, 'UnstructuredGridRepresentation')

# Also store the max Y or Z wire dimension to set the camera zoom later
wireMaxYZDim = 0.0


for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    thresholdContact = Threshold(Input=casefoamContact)
    thresholdContact.Scalars = ['CELLS', 'normalPressure']
    thresholdContact.ThresholdRange = [1e6, 1e20]
    thresholdContactDisplay = Show(thresholdContact, renderView1, 'UnstructuredGridRepresentation')
    bounds = thresholdContact.GetDataInformation().GetBounds()
    contactStart = bounds[0]
    contactEnd = bounds[1]
    wireMaxYZDim = max(wireMaxYZDim, max(abs(bounds[2] - bounds[3]), abs(bounds[4] - bounds[5])))
    #print("Start is ", contactStart)
    #print("End is ", contactEnd)

    keyFrameIs = CompositeKeyFrame()
    keyFrameIs.KeyTime = passI/endtime
    keyFrameIs.KeyValues = [1.5*contactStart]
    allKeyFrames.append(keyFrameIs)
    keyFrameIe = CompositeKeyFrame()
    keyFrameIe.KeyTime = (passI + 0.4)/endtime
    keyFrameIe.KeyValues = [1.4*contactEnd]
    allKeyFrames.append(keyFrameIe)

    Hide(thresholdContact)

# initialize the animation track
slice1ContourValuesTrack.KeyFrames = allKeyFrames

Hide(casefoamContact)


################################################################################
# Set camera animation 
################################################################################

print("Defining the camera animation")

renderView1.InteractionMode = '2D'

cameraAnimationCue1 = GetCameraTrack(view=renderView1)

zoom = 0.8*wireMaxYZDim

# create a key frame
keyFrameCam1 = CameraKeyFrame()
keyFrameCam1.Position = [-1, 0.0, 0.0]
keyFrameCam1.FocalPoint = [0.0, 0.002, 0.0]
keyFrameCam1.ViewUp = [0.0, 1.0, 0.0]
keyFrameCam1.ParallelScale = zoom
keyFrameCam1.PositionPathPoints = [0.04, 0.0, 0.0]
keyFrameCam1.FocalPathPoints = [0.0, 0.0, 0.0]
keyFrameCam1.ClosedPositionPath = 0

keyFrameCam2 = CameraKeyFrame()
keyFrameCam2.KeyTime = 1.0

# initialize the animation track
cameraAnimationCue1.Mode = 'Path-based'
cameraAnimationCue1.KeyFrames = [keyFrameCam1, keyFrameCam2]


################################################################################
# Display epsilonPEq on wire
################################################################################

ColorBy(wireDisplay, ('POINTS', 'epsilonPEq'))
wireDisplay.Opacity = 1.0

toolsDisplay.Opacity = 0.3
toolsDisplay.AmbientColor = [1.0, 1.0, 0.4980392156862745]
toolsDisplay.DiffuseColor = [1.0, 1.0, 0.4980392156862745]

epsilonPEqLUT = GetColorTransferFunction('epsilonPEq')
epsilonPEqPWF = GetOpacityTransferFunction('epsilonPEq')
epsilonPEqLUTColorBar = GetScalarBar(epsilonPEqLUT, renderView1)

# change scalar bar placement
epsilonPEqLUTColorBar.WindowLocation = 'AnyLocation'
epsilonPEqLUTColorBar.Orientation = 'Horizontal'
epsilonPEqLUTColorBar.Position = [0.04, 0.9]
epsilonPEqLUTColorBar.ScalarBarLength = 0.2

# Properties modified on epsilonPEqLUTColorBar
epsilonPEqLUTColorBar.Title = 'Plastic strain [-]'
epsilonPEqLUTColorBar.TitleColor = [0.0, 0.0, 0.0]
epsilonPEqLUTColorBar.LabelColor = [0.0, 0.0, 0.0]
epsilonPEqLUTColorBar.AutomaticLabelFormat = 0
epsilonPEqLUTColorBar.LabelFormat = '%-#6.3f'
epsilonPEqLUTColorBar.AddRangeLabels = 0
epsilonPEqLUTColorBar.DrawTickMarks = 0
epsilonPEqLUTColorBar.DrawTickLabels = 0
epsilonPEqLUTColorBar.AddRangeLabels = 1
epsilonPEqLUTColorBar.RangeLabelFormat = '%-#6.1f'
epsilonPEqLUTColorBar.TitleFontSize = 14
epsilonPEqLUTColorBar.LabelFontSize = 14
epsilonPEqLUTColorBar.TitleFontFamily = 'Courier'
epsilonPEqLUTColorBar.LabelFontFamily = 'Courier'
epsilonPEqLUTColorBar.ComponentTitle = ''
epsilonPEqLUT.ApplyPreset('Rainbow Uniform', True)
epsilonPEqLUT.Discretize = 1
epsilonPEqLUT.NumberOfTableValues = 15
epsilonPEqLUT.RescaleTransferFunction(0.0, 2.0)
epsilonPEqPWF.RescaleTransferFunction(0.0, 2.0)


################################################################################
# Save screenshots and animation - epsilonPEq
################################################################################

# Screenshot saved for each pass
print("Saving screenshots for slice at x = 0: virtualTwinEpsilonPEq_*.png")
renderView1.CameraPosition = [-1, 0.0, 0.0]
renderView1.CameraFocalPoint = [0.0, 0.002, 0.0]
renderView1.CameraViewUp = [0.0, 1.0, 0.0]
renderView1.CameraParallelScale = zoom
for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    renderView1.InteractionMode = '2D'
    slice1.SliceType.Origin = [0.0, 0.0, 0.0]
    SaveScreenshot('./virtualTwinEpsilonPEq_pass' + str(passI) + '.png', renderView1, ImageResolution=[2*1584, 2*932])

# Animation
nframes = len(timeKeeper1.TimestepValues)
print("Saving animation: virtualTwinEpsilonPEq.avi")
SaveAnimation('./virtualTwinEpsilonPEq.avi', renderView1, ImageResolution=[1584, 932],
    FrameRate=3, FrameWindow=[0, nframes])


################################################################################
# Display sigmaMax on tools
################################################################################

wireDisplay.SetScalarBarVisibility(renderView1, False)

ColorBy(toolsDisplay, ('POINTS', 'sigmaMax'))
toolsDisplay.Opacity = 1.0

wireDisplay.Opacity = 0.3
ColorBy(wireDisplay, None)
wireDisplay.AmbientColor = [1.0, 1.0, 0.4980392156862745]
wireDisplay.DiffuseColor = [1.0, 1.0, 0.4980392156862745]

sigmaMaxLUT = GetColorTransferFunction('sigmaMax')
sigmaMaxPWF = GetOpacityTransferFunction('sigmaMax')
sigmaMaxLUTColorBar = GetScalarBar(sigmaMaxLUT, renderView1)

# change scalar bar placement
sigmaMaxLUTColorBar.WindowLocation = 'AnyLocation'
sigmaMaxLUTColorBar.Orientation = 'Horizontal'
sigmaMaxLUTColorBar.Position = [0.04, 0.9]
sigmaMaxLUTColorBar.ScalarBarLength = 0.2

# Properties modified on sigmaMaxLUTColorBar
sigmaMaxLUTColorBar.Title = 'Max Prinicpal Stress [in Pa]'
sigmaMaxLUTColorBar.TitleColor = [0.0, 0.0, 0.0]
sigmaMaxLUTColorBar.LabelColor = [0.0, 0.0, 0.0]
sigmaMaxLUTColorBar.AutomaticLabelFormat = 0
sigmaMaxLUTColorBar.LabelFormat = '%-#6.3e'
sigmaMaxLUTColorBar.AddRangeLabels = 0
sigmaMaxLUTColorBar.DrawTickMarks = 0
sigmaMaxLUTColorBar.DrawTickLabels = 0
sigmaMaxLUTColorBar.AddRangeLabels = 1
sigmaMaxLUTColorBar.RangeLabelFormat = '%-#6.1e'
sigmaMaxLUTColorBar.TitleFontSize = 14
sigmaMaxLUTColorBar.LabelFontSize = 14
sigmaMaxLUTColorBar.TitleFontFamily = 'Courier'
sigmaMaxLUTColorBar.LabelFontFamily = 'Courier'
sigmaMaxLUTColorBar.ComponentTitle = ''
sigmaMaxLUT.ApplyPreset('Rainbow Uniform', True)
sigmaMaxLUT.Discretize = 1
sigmaMaxLUT.NumberOfTableValues = 15
sigmaMaxLUT.RescaleTransferFunction(-1e9, 1e9)
sigmaMaxPWF.RescaleTransferFunction(-1e9, 1e9)


################################################################################
# Ssave screenshots and animation - sigmaMax tools
################################################################################

# Screenshot saved for each pass
print("Saving screenshots for slice at x = 0: virtualTwinSigmaMaxTools_*.png")
renderView1.CameraPosition = [-1, 0.0, 0.0]
renderView1.CameraFocalPoint = [0.0, 0.002, 0.0]
renderView1.CameraViewUp = [0.0, 1.0, 0.0]
renderView1.CameraParallelScale = zoom
for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    renderView1.InteractionMode = '2D'
    slice1.SliceType.Origin = [0.0, 0.0, 0.0]
    SaveScreenshot('./virtualTwinSigmaMaxTools_pass' + str(passI) + '.png', renderView1, ImageResolution=[2*1584, 2*932])

# Animation
print("Saving animation: virtualTwinSigmaMaxTools.avi")
SaveAnimation('./virtualTwinSigmaMaxTools.avi', renderView1, ImageResolution=[1584, 932],
    FrameRate=3, FrameWindow=[0, nframes])


################################################################################
# Display sigmaMax on wire
################################################################################

wireDisplay.SetScalarBarVisibility(renderView1, True)
#toolsDisplay.SetScalarBarVisibility(renderView1, False)

ColorBy(wireDisplay, ('POINTS', 'sigmaMax'))
wireDisplay.Opacity = 1.0

toolsDisplay.Opacity = 0.3
ColorBy(toolsDisplay, None)
toolsDisplay.AmbientColor = [1.0, 1.0, 0.4980392156862745]
toolsDisplay.DiffuseColor = [1.0, 1.0, 0.4980392156862745]

sigmaMaxLUT = GetColorTransferFunction('sigmaMax')
sigmaMaxPWF = GetOpacityTransferFunction('sigmaMax')
sigmaMaxLUTColorBar = GetScalarBar(sigmaMaxLUT, renderView1)
wireDisplay.SetScalarBarVisibility(renderView1, True)

# change scalar bar placement
sigmaMaxLUTColorBar.WindowLocation = 'AnyLocation'
sigmaMaxLUTColorBar.Orientation = 'Horizontal'
sigmaMaxLUTColorBar.Position = [0.04, 0.9]
sigmaMaxLUTColorBar.ScalarBarLength = 0.2

# Properties modified on sigmaMaxLUTColorBar
sigmaMaxLUTColorBar.Title = 'Max Prinicpal Stress [in Pa]'
sigmaMaxLUTColorBar.TitleColor = [0.0, 0.0, 0.0]
sigmaMaxLUTColorBar.LabelColor = [0.0, 0.0, 0.0]
sigmaMaxLUTColorBar.AutomaticLabelFormat = 0
sigmaMaxLUTColorBar.LabelFormat = '%-#6.3e'
sigmaMaxLUTColorBar.AddRangeLabels = 0
sigmaMaxLUTColorBar.DrawTickMarks = 0
sigmaMaxLUTColorBar.DrawTickLabels = 0
sigmaMaxLUTColorBar.AddRangeLabels = 1
sigmaMaxLUTColorBar.RangeLabelFormat = '%-#6.1e'
sigmaMaxLUTColorBar.TitleFontSize = 14
sigmaMaxLUTColorBar.LabelFontSize = 14
sigmaMaxLUTColorBar.TitleFontFamily = 'Courier'
sigmaMaxLUTColorBar.LabelFontFamily = 'Courier'
sigmaMaxLUTColorBar.ComponentTitle = ''
sigmaMaxLUT.ApplyPreset('Rainbow Uniform', True)
sigmaMaxLUT.Discretize = 1
sigmaMaxLUT.NumberOfTableValues = 15
sigmaMaxLUT.RescaleTransferFunction(-1e9, 1e9)
sigmaMaxPWF.RescaleTransferFunction(-1e9, 1e9)


################################################################################
# Ssave screenshots and animation - sigmaMax wire
################################################################################

# Screenshot saved for each pass
print("Saving screenshots for slice at x = 0: virtualTwinSigmaMaxWire_*.png")
renderView1.CameraPosition = [-1, 0.0, 0.0]
renderView1.CameraFocalPoint = [0.0, 0.002, 0.0]
renderView1.CameraViewUp = [0.0, 1.0, 0.0]
renderView1.CameraParallelScale = zoom
for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    renderView1.InteractionMode = '2D'
    slice1.SliceType.Origin = [0.0, 0.0, 0.0]
    SaveScreenshot('./virtualTwinSigmaMaxWire_pass' + str(passI) + '.png', renderView1, ImageResolution=[2*1584, 2*932])

# Animation
print("Saving animation: virtualTwinSigmaMaxWire.avi")
SaveAnimation('./virtualTwinSigmaMaxWire.avi', renderView1, ImageResolution=[1584, 932],
    FrameRate=3, FrameWindow=[0, nframes])


################################################################################
# Display contact total traction on tools
################################################################################

wireDisplay.SetScalarBarVisibility(renderView1, False)
toolsDisplay.SetScalarBarVisibility(renderView1, False)

toolsDisplay.Opacity = 0.3
ColorBy(toolsDisplay, None)
toolsDisplay.AmbientColor = [1.0, 1.0, 0.4980392156862745]
toolsDisplay.DiffuseColor = [1.0, 1.0, 0.4980392156862745]

wireDisplay.Opacity = 0.3
ColorBy(wireDisplay, None)
wireDisplay.AmbientColor = [1.0, 1.0, 0.4980392156862745]
wireDisplay.DiffuseColor = [1.0, 1.0, 0.4980392156862745]

# create glyphs
totalTraction = Glyph(Input=tools, GlyphType='Arrow')
totalTraction.OrientationArray = ['POINTS', 'totalTraction']
totalTraction.ScaleArray = ['POINTS', 'totalTraction']
totalTraction.ScaleFactor = 1e-12
totalTraction.GlyphMode = 'All Points'
totalTractionDisplay = Show(totalTraction, renderView1, 'GeometryRepresentation')

ColorBy(totalTractionDisplay, ('POINTS', 'totalTraction', 'Magnitude'))
totalTractionDisplay.SetScalarBarVisibility(renderView1, True)

totalTractionLUT = GetColorTransferFunction('totalTraction')
totalTractionPWF = GetOpacityTransferFunction('totalTraction')
totalTractionLUTColorBar = GetScalarBar(totalTractionLUT, renderView1)

# change scalar bar placement
totalTractionLUTColorBar.WindowLocation = 'AnyLocation'
totalTractionLUTColorBar.Orientation = 'Horizontal'
totalTractionLUTColorBar.Position = [0.04, 0.9]
totalTractionLUTColorBar.ScalarBarLength = 0.2

# Properties modified on totalTractionLUTColorBar
totalTractionLUTColorBar.Title = 'Contact traction [in Pa]'
totalTractionLUTColorBar.TitleColor = [0.0, 0.0, 0.0]
totalTractionLUTColorBar.LabelColor = [0.0, 0.0, 0.0]
totalTractionLUTColorBar.AutomaticLabelFormat = 0
totalTractionLUTColorBar.LabelFormat = '%-#6.3e'
totalTractionLUTColorBar.AddRangeLabels = 0
totalTractionLUTColorBar.DrawTickMarks = 0
totalTractionLUTColorBar.DrawTickLabels = 0
totalTractionLUTColorBar.AddRangeLabels = 1
totalTractionLUTColorBar.RangeLabelFormat = '%-#6.1e'
totalTractionLUTColorBar.TitleFontSize = 14
totalTractionLUTColorBar.LabelFontSize = 14
totalTractionLUTColorBar.TitleFontFamily = 'Courier'
totalTractionLUTColorBar.LabelFontFamily = 'Courier'
totalTractionLUTColorBar.ComponentTitle = ''
totalTractionLUT.ApplyPreset('Rainbow Uniform', True)
totalTractionLUT.Discretize = 1
totalTractionLUT.NumberOfTableValues = 15
totalTractionLUT.RescaleTransferFunction(0, 2.6e9)
totalTractionPWF.RescaleTransferFunction(0, 2.6e9)


################################################################################
# Ssave screenshots and animation - contactTraction
################################################################################

# Screenshot saved for each pass
print("Saving screenshots for slice at x = 0: virtualTwinContactTraction_*.png")
renderView1.CameraPosition = [-1, 0.0, 0.0]
renderView1.CameraFocalPoint = [0.0, 0.002, 0.0]
renderView1.CameraViewUp = [0.0, 1.0, 0.0]
renderView1.CameraParallelScale = zoom
for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    renderView1.InteractionMode = '2D'
    slice1.SliceType.Origin = [0.0, 0.0, 0.0]
    SaveScreenshot('./virtualTwinContactTraction_pass' + str(passI) + '.png', renderView1, ImageResolution=[2*1584, 2*932])

# Animation
print("Saving animation: virtualTwinContactTraction.avi")
SaveAnimation('./virtualTwinContactTraction.avi', renderView1, ImageResolution=[1584, 932],
    FrameRate=3, FrameWindow=[0, nframes])


################################################################################
# Display mesh
################################################################################

Hide(tools)
Hide(wire)
Hide(featureEdgesAll)
Hide(totalTraction)
casefoam.Decomposepolyhedra = 0
slice1.Triangulatetheslice = 0
sliceAllDisplay = Show(sliceAll, renderView1, 'UnstructuredGridRepresentation')
sliceAllDisplay.SetRepresentationType('Wireframe')
sliceAllDisplay.AmbientColor = [0, 0, 0]


################################################################################
# Ssave screenshots and animation - mesh
################################################################################

# Screenshot saved for each pass
print("Saving screenshots for slice at x = 0: virtualTwinMesh_*.png")
renderView1.CameraPosition = [-1, 0.0, 0.0]
renderView1.CameraFocalPoint = [0.0, 0.002, 0.0]
renderView1.CameraViewUp = [0.0, 1.0, 0.0]
renderView1.CameraParallelScale = zoom
for passI in range(numPasses):
    timeKeeper1.Time = (passI + 0.4)
    renderView1.InteractionMode = '2D'
    slice1.SliceType.Origin = [0.0, 0.0, 0.0]
    SaveScreenshot('./virtualTwinMesh_pass' + str(passI) + '.png', renderView1, ImageResolution=[2*1584, 2*932])

# Animation
print("Saving animation: virtualTwinMesh.avi")
SaveAnimation('./virtualTwinMesh.avi', renderView1, ImageResolution=[1584, 932],
    FrameRate=3, FrameWindow=[0, nframes])


# Test - 3-D view
# Load wire patch, reflect, set colour and opacity
# Load die/roller (use threshold as patches may not exist), reflect, set colour and opacity
# Set camera position (maybe keyframes are needed)
#renderView1.CameraPosition = [8*wireMaxYZDim,0.3*8*wireMaxYZDim,0.5*8*wireMaxYZDim]