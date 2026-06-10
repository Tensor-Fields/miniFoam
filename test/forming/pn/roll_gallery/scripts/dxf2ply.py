#!/usr/bin/env python3
#
# Usage:
# blender -b -P dxf2ply.py -- input.dxf output.ply 300
#
# Author:
# Maalik (Ali Shayegh, alishayegh@proton.me)
#    - Did operations in the Blender UI
#    - Recorded the python translation
# ChatGPT 
#    - Cleaned and re-wrote the python script
#
# Date:
# Jun 07 2026

import bpy
import sys
from pathlib import Path

# --------------------------------------------------
# args
# --------------------------------------------------

argv = sys.argv
argv = argv[argv.index("--") + 1:]

if len(argv) != 3:
    raise SystemExit(
        "Usage: blender -b -P dxf2ply.py -- input.dxf output.ply point_count"
    )

dxf_file  = str(Path(argv[0]).resolve())
ply_file  = str(Path(argv[1]).resolve())
n_points  = int(argv[2])

# --------------------------------------------------
# clean scene
# --------------------------------------------------

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

# --------------------------------------------------
# import DXF
# --------------------------------------------------
bpy.ops.wm.context_set_enum(data_path="area.type", value="VIEW_3D")

bpy.ops.import_scene.dxf(filepath=dxf_file)
# --------------------------------------------------
# rotate exactly as recorded
# --------------------------------------------------

obj.rotation_euler[1] += 1.5708

# --------------------------------------------------
# geometry nodes
#
# Group Input
#   -> Resample Curve
#   -> Curve to Points
#   -> Set Point Radius
#   -> Points to Vertices
#   -> Group Output
# --------------------------------------------------

mod = obj.modifiers.new("GN", "NODES")

ng = bpy.data.node_groups.new("DXF2PLY", "GeometryNodeTree")
mod.node_group = ng

nodes = ng.nodes
links = ng.links

nodes.clear()

group_in  = nodes.new("NodeGroupInput")
group_out = nodes.new("NodeGroupOutput")

ng.interface.new_socket(
    name="Geometry",
    in_out='INPUT',
    socket_type='NodeSocketGeometry'
)

ng.interface.new_socket(
    name="Geometry",
    in_out='OUTPUT',
    socket_type='NodeSocketGeometry'
)

resample = nodes.new("GeometryNodeResampleCurve")
curve2pts = nodes.new("GeometryNodeCurveToPoints")
setrad = nodes.new("GeometryNodeSetPointRadius")
pts2verts = nodes.new("GeometryNodePointsToVertices")

group_in.location = (-800, 0)
resample.location = (-500, 0)
curve2pts.location = (-200, 0)
setrad.location = (100, 0)
pts2verts.location = (400, 0)
group_out.location = (700, 0)

# count mode
resample.mode = 'COUNT'
resample.inputs["Count"].default_value = n_points

# tiny radius
setrad.inputs["Radius"].default_value = 0.001

# wiring
links.new(group_in.outputs["Geometry"], resample.inputs["Curve"])
links.new(resample.outputs["Curve"], curve2pts.inputs["Curve"])
links.new(curve2pts.outputs["Points"], setrad.inputs["Points"])
links.new(setrad.outputs["Points"], pts2verts.inputs["Points"])
links.new(pts2verts.outputs["Mesh"], group_out.inputs["Geometry"])

# --------------------------------------------------
# apply GN so exported vertices become real mesh
# --------------------------------------------------

bpy.ops.object.modifier_apply(modifier=mod.name)

# --------------------------------------------------
# export PLY
# --------------------------------------------------

obj.select_set(True)
bpy.context.view_layer.objects.active = obj

bpy.ops.wm.ply_export(
    filepath=ply_file,
    export_selected_objects=True,
    export_normals=False,
    export_uv=False,
    export_colors=False,
    export_triangulated_mesh=False
)

print(f"Exported {ply_file}")
