import bpy
import math
from pathlib import Path
from mathutils import Vector


OUTPUT_DIR = Path(__file__).resolve().parent
BLEND_PATH = OUTPUT_DIR / "LowPolyVase.blend"
FBX_PATH = OUTPUT_DIR / "LowPolyVase.fbx"
SEGMENTS = 16


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.materials):
        for block in list(datablocks):
            if block.users == 0:
                datablocks.remove(block)


def make_material(name, color, roughness=0.72):
    material = bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = True
    bsdf = material.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    return material


def ring_vertices(profile):
    vertices = []
    for radius, z in profile:
        for i in range(SEGMENTS):
            angle = 2.0 * math.pi * i / SEGMENTS
            vertices.append((radius * math.cos(angle), radius * math.sin(angle), z))
    return vertices


def connect_rings(faces, ring_count, reverse=False):
    for ring in range(ring_count - 1):
        start_a = ring * SEGMENTS
        start_b = (ring + 1) * SEGMENTS
        for i in range(SEGMENTS):
            j = (i + 1) % SEGMENTS
            face = (start_a + i, start_a + j, start_b + j, start_b + i)
            faces.append(tuple(reversed(face)) if reverse else face)


def create_body(material):
    outer = [
        (0.46, 0.00),
        (0.58, 0.12),
        (0.76, 0.42),
        (0.86, 0.90),
        (0.78, 1.35),
        (0.60, 1.63),
        (0.39, 1.78),
        (0.34, 2.18),
        (0.43, 2.24),
        (0.43, 2.36),
    ]
    inner = [
        (0.34, 2.36),
        (0.34, 2.25),
        (0.27, 2.18),
        (0.27, 1.82),
        (0.44, 1.68),
        (0.63, 1.40),
        (0.70, 0.92),
        (0.60, 0.52),
        (0.36, 0.30),
    ]

    vertices = ring_vertices(outer)
    faces = []
    connect_rings(faces, len(outer))

    inner_offset = len(vertices)
    vertices.extend(ring_vertices(inner))
    inner_faces = []
    connect_rings(inner_faces, len(inner), reverse=True)
    faces.extend(tuple(index + inner_offset for index in face) for face in inner_faces)

    # Join the lip's outer and inner edges.
    outer_top = (len(outer) - 1) * SEGMENTS
    inner_top = inner_offset
    for i in range(SEGMENTS):
        j = (i + 1) % SEGMENTS
        faces.append((outer_top + i, outer_top + j, inner_top + j, inner_top + i))

    # Close the underside and the interior floor with triangle fans.
    outer_bottom_center = len(vertices)
    vertices.append((0.0, 0.0, 0.0))
    for i in range(SEGMENTS):
        j = (i + 1) % SEGMENTS
        faces.append((outer_bottom_center, j, i))

    inner_bottom_center = len(vertices)
    vertices.append((0.0, 0.0, inner[-1][1]))
    inner_bottom = inner_offset + (len(inner) - 1) * SEGMENTS
    for i in range(SEGMENTS):
        j = (i + 1) % SEGMENTS
        faces.append((inner_bottom_center, inner_bottom + i, inner_bottom + j))

    mesh = bpy.data.meshes.new("LowPolyVase_Body_Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    body = bpy.data.objects.new("LowPolyVase_Body", mesh)
    bpy.context.collection.objects.link(body)
    body.data.materials.append(material)
    for polygon in mesh.polygons:
        polygon.use_smooth = False
    return body


def create_handle(name, side, material):
    curve = bpy.data.curves.new(name + "_Curve", type="CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 1
    curve.bevel_depth = 0.085
    curve.bevel_resolution = 0
    curve.resolution_u = 1
    curve.resolution_v = 0

    spline = curve.splines.new(type="POLY")
    points = [
        (0.47 * side, 0.0, 1.72),
        (0.76 * side, 0.0, 1.66),
        (1.00 * side, 0.0, 1.48),
        (1.03 * side, 0.0, 1.20),
        (0.88 * side, 0.0, 1.02),
        (0.73 * side, 0.0, 1.00),
    ]
    spline.points.add(len(points) - 1)
    for point, coordinate in zip(spline.points, points):
        point.co = (*coordinate, 1.0)

    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(material)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.convert(target="MESH")
    for polygon in obj.data.polygons:
        polygon.use_smooth = False
    obj.select_set(False)
    return obj


def add_metadata(body):
    body["asset_type"] = "Low-poly vase"
    body["units"] = "meters"
    body["texture_usage"] = "none"
    body["lighting_included"] = False


def main():
    clear_scene()
    clay = make_material("Clay_Terracotta", (0.48, 0.17, 0.075))
    body = create_body(clay)
    create_handle("LowPolyVase_Handle_L", 1.0, clay)
    create_handle("LowPolyVase_Handle_R", -1.0, clay)
    add_metadata(body)

    for obj in bpy.context.scene.objects:
        obj.select_set(obj.type == "MESH")
    bpy.context.view_layer.objects.active = body

    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.length_unit = "METERS"
    bpy.context.scene.render.engine = "BLENDER_EEVEE_NEXT"

    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    bpy.ops.export_scene.fbx(
        filepath=str(FBX_PATH),
        use_selection=True,
        object_types={"MESH"},
        apply_unit_scale=True,
        add_leaf_bones=False,
        bake_anim=False,
        path_mode="AUTO",
    )
    print(f"Created: {BLEND_PATH}")
    print(f"Created: {FBX_PATH}")
    print(
        "Mesh totals:",
        sum(len(obj.data.vertices) for obj in bpy.context.scene.objects if obj.type == "MESH"),
        "vertices,",
        sum(len(obj.data.polygons) for obj in bpy.context.scene.objects if obj.type == "MESH"),
        "polygons",
    )


if __name__ == "__main__":
    main()
