import bpy
import math
import os
import random


OUT_DIR = os.path.dirname(os.path.abspath(__file__))
BLEND_PATH = os.path.join(OUT_DIR, "LoopFloor_GrassDirtRock.blend")
FBX_PATH = os.path.join(OUT_DIR, "LoopFloor_GrassDirtRock.fbx")

TILE_X_MIN = -7.0
TILE_X_MAX = 7.0
TILE_Y_MIN = -5.0
TILE_Y_MAX = 5.0
ROCK_X_MIN = 1.2
BOTTOM_Z = -0.28


def material(name, color, roughness=0.9):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = roughness
    return mat


def mesh_object(name, vertices, faces, mat, face_materials=None):
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    if isinstance(mat, (list, tuple)):
        for item in mat:
            mesh.materials.append(item)
    else:
        mesh.materials.append(mat)
    if face_materials:
        for poly, index in zip(mesh.polygons, face_materials):
            poly.material_index = index
    return obj


def boundary_x(y):
    # Periodic at +/-5m, so repeated tiles meet exactly.
    phase = 2.0 * math.pi * (y - TILE_Y_MIN) / (TILE_Y_MAX - TILE_Y_MIN)
    return -2.55 + 0.38 * math.sin(phase) + 0.12 * math.sin(2.0 * phase + 0.4)


def make_strip(name, left_fn, right_fn, top_z, mat, segments=20):
    verts = []
    faces = []
    for row in range(segments + 1):
        y = TILE_Y_MIN + (TILE_Y_MAX - TILE_Y_MIN) * row / segments
        verts.extend([
            (left_fn(y), y, top_z),
            (right_fn(y), y, top_z),
            (left_fn(y), y, BOTTOM_Z),
            (right_fn(y), y, BOTTOM_Z),
        ])
    for row in range(segments):
        a = row * 4
        b = (row + 1) * 4
        faces.extend([
            (a, b, b + 1, a + 1),       # top
            (a + 2, a + 3, b + 3, b + 2),
            (a, a + 2, b + 2, b),
            (a + 1, b + 1, b + 3, a + 3),
        ])
    # Close only the outer tile ends; when repeated they coincide exactly.
    faces.extend([(0, 1, 3, 2),
                  (segments * 4, segments * 4 + 2, segments * 4 + 3, segments * 4 + 1)])
    return mesh_object(name, verts, faces, mat)


def make_boundary_wall(mat, segments=20):
    verts = []
    faces = []
    for row in range(segments + 1):
        y = TILE_Y_MIN + (TILE_Y_MAX - TILE_Y_MIN) * row / segments
        x = boundary_x(y)
        verts.extend([(x, y, 0.05), (x, y, 0.0)])
    for row in range(segments):
        a = row * 2
        b = (row + 1) * 2
        faces.append((a, a + 1, b + 1, b))
    return mesh_object("GrassDirtBoundary", verts, faces, mat)


def make_rocks(rock_materials):
    random.seed(240717)
    cols = 4
    rows = 7
    dx = (TILE_X_MAX - ROCK_X_MIN) / cols
    dy = (TILE_Y_MAX - TILE_Y_MIN) / rows

    # Shared vertices make every polygonal rock meet its neighbors without gaps.
    nodes = {}
    for row in range(rows + 1):
        y = TILE_Y_MIN + row * dy
        periodic_offset = 0.22 * math.sin(2.0 * math.pi * row / rows)
        for col in range(cols + 1):
            x = ROCK_X_MIN + col * dx
            if 0 < col < cols:
                x += periodic_offset * (1.0 if col % 2 else -0.65)
            if 0 < row < rows:
                y_jitter = 0.12 * math.sin(col * 1.7 + row * 2.1)
            else:
                y_jitter = 0.0
            nodes[(col, row)] = (x, y + y_jitter)

    objects = []
    for row in range(rows):
        for col in range(cols):
            corners = [nodes[(col, row)], nodes[(col + 1, row)],
                       nodes[(col + 1, row + 1)], nodes[(col, row + 1)]]
            base_height = 0.09 + 0.035 * ((col + 2 * row) % 4)
            # Small per-corner variation gives a natural low-poly rock plane.
            top_z = [base_height + random.uniform(-0.025, 0.025) for _ in range(4)]
            verts = [(x, y, top_z[i]) for i, (x, y) in enumerate(corners)]
            verts += [(x, y, BOTTOM_Z) for x, y in corners]
            faces = [
                (0, 1, 2, 3), (4, 7, 6, 5),
                (0, 4, 5, 1), (1, 5, 6, 2),
                (2, 6, 7, 3), (3, 7, 4, 0),
            ]
            objects.append(mesh_object(
                f"Rock_{row:02d}_{col:02d}", verts, faces,
                rock_materials[(row + col) % len(rock_materials)]
            ))
    return objects


def main():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.materials,
                       bpy.data.cameras, bpy.data.lights):
        for block in list(datablocks):
            datablocks.remove(block)

    grass = material("Grass_Material", (0.19, 0.42, 0.12))
    dirt = material("Dirt_Material", (0.32, 0.16, 0.07))
    boundary = material("Boundary_Soil_Material", (0.20, 0.085, 0.03))
    rocks = [
        material("Rock_Gray_A", (0.28, 0.30, 0.29)),
        material("Rock_Gray_B", (0.34, 0.35, 0.32)),
        material("Rock_Gray_C", (0.23, 0.25, 0.25)),
    ]

    parts = [
        make_strip("GrassArea", lambda y: TILE_X_MIN, boundary_x, 0.05, grass),
        make_strip("DirtArea", boundary_x, lambda y: ROCK_X_MIN, 0.0, dirt),
        make_boundary_wall(boundary),
    ]
    parts.extend(make_rocks(rocks))

    bpy.ops.object.select_all(action="DESELECT")
    for obj in parts:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    floor = bpy.context.active_object
    floor.name = "LoopFloor_Integrated"
    floor.data.name = "LoopFloor_Integrated_Mesh"

    # Preserve a crisp, deliberately low-poly silhouette.
    for poly in floor.data.polygons:
        poly.use_smooth = False

    # One non-overlapping UV set for the complete integrated mesh.
    bpy.context.view_layer.objects.active = floor
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66.0), island_margin=0.02,
                             area_weight=0.0, correct_aspect=True, scale_to_bounds=True)
    bpy.ops.object.mode_set(mode="OBJECT")
    if floor.data.uv_layers.active:
        floor.data.uv_layers.active.name = "UVMap"

    floor["tile_size_x"] = TILE_X_MAX - TILE_X_MIN
    floor["tile_size_y"] = TILE_Y_MAX - TILE_Y_MIN
    floor["loop_axis"] = "Y"
    floor["model_notes"] = "Low-poly integrated grass/dirt/packed-rock looping floor; no textures"

    # Keep only the requested model in the scene.
    bpy.context.scene.world.color = (0.05, 0.05, 0.05)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.length_unit = "METERS"

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.ops.export_scene.fbx(
        filepath=FBX_PATH,
        use_selection=True,
        object_types={"MESH"},
        apply_unit_scale=True,
        bake_space_transform=False,
        add_leaf_bones=False,
        path_mode="AUTO",
        embed_textures=False,
    )

    print("RESULT_OBJECTS", len(bpy.context.scene.objects))
    print("RESULT_VERTICES", len(floor.data.vertices))
    print("RESULT_POLYGONS", len(floor.data.polygons))
    print("RESULT_UV_LAYERS", len(floor.data.uv_layers))
    print("RESULT_UV_LOOPS", len(floor.data.uv_layers.active.data))
    print("RESULT_MATERIALS", len(floor.data.materials))
    print("RESULT_LIGHTS", len(bpy.data.lights))
    print("RESULT_CAMERAS", len(bpy.data.cameras))
    print("RESULT_BLEND", BLEND_PATH)
    print("RESULT_FBX", FBX_PATH)


if __name__ == "__main__":
    main()
