import bpy
import math
import os
import random
from mathutils import Vector


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MODEL_ROOT = os.path.join(ROOT, "Resources", "Assets", "models")
SOURCE_ROOT = os.path.join(ROOT, "Resources", "Assets", "AssetsWorkspace")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.cameras, bpy.data.lights):
        for block in list(datablocks):
            if block.users == 0:
                datablocks.remove(block)


def make_material(name, color):
    material = bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = False
    material.roughness = 0.92
    return material


def finish_mesh(obj, material):
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = False
    obj.data.update()
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    minimum_z = min(vertex.co.z for vertex in obj.data.vertices)
    for vertex in obj.data.vertices:
        vertex.co.z -= minimum_z
    return obj


def create_round_rock(material):
    random.seed(1203)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1.0, location=(-3.0, 0.0, 0.0))
    obj = bpy.context.object
    obj.name = "Rock_Round"
    obj.scale = (1.42, 1.15, 0.92)
    for vertex in obj.data.vertices:
        direction = vertex.co.normalized()
        angle = math.atan2(direction.y, direction.x)
        variation = 1.0 + 0.09 * math.sin(angle * 3.0 + 0.8) + random.uniform(-0.07, 0.07)
        vertex.co *= variation
        if vertex.co.z < -0.35:
            vertex.co.z = -0.46 + (vertex.co.z + 0.35) * 0.18
    return finish_mesh(obj, material)


def create_jagged_rock(material):
    random.seed(7319)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1.0, location=(0.0, 0.0, 0.0))
    obj = bpy.context.object
    obj.name = "Rock_Jagged"
    obj.scale = (0.92, 0.78, 1.52)
    for vertex in obj.data.vertices:
        direction = vertex.co.normalized()
        angle = math.atan2(direction.y, direction.x)
        height = direction.z
        ridge = 0.12 * math.cos(angle * 5.0 + height * 2.7)
        variation = 1.0 + ridge + random.uniform(-0.10, 0.10)
        if height > 0.45:
            variation += 0.10 * (height - 0.45)
        vertex.co *= variation
        vertex.co.x += 0.12 * max(vertex.co.z, 0.0)
        if vertex.co.z < -0.42:
            vertex.co.z = -0.50 + (vertex.co.z + 0.42) * 0.15
    return finish_mesh(obj, material)


def create_slab_rock(material):
    random.seed(4481)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1.0, location=(3.0, 0.0, 0.0))
    obj = bpy.context.object
    obj.name = "Rock_Slab"
    obj.scale = (1.62, 1.08, 0.48)
    for vertex in obj.data.vertices:
        direction = vertex.co.normalized()
        angle = math.atan2(direction.y, direction.x)
        variation = 1.0 + 0.10 * math.sin(angle * 4.0 - 0.3) + random.uniform(-0.055, 0.055)
        vertex.co.x *= variation
        vertex.co.y *= 1.0 + 0.07 * math.cos(angle * 3.0)
        vertex.co.z *= 1.0 + random.uniform(-0.09, 0.09)
        if vertex.co.z > 0.45:
            vertex.co.z = 0.48 + (vertex.co.z - 0.45) * 0.22
        if vertex.co.z < -0.38:
            vertex.co.z = -0.43 + (vertex.co.z + 0.38) * 0.14
    obj.rotation_euler.z = math.radians(-8.0)
    return finish_mesh(obj, material)


def export_obj(obj, folder_name):
    folder = os.path.join(MODEL_ROOT, folder_name)
    os.makedirs(folder, exist_ok=True)
    path = os.path.join(folder, f"{folder_name}.obj")
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    display_location = obj.location.copy()
    obj.location = Vector((0.0, 0.0, 0.0))
    bpy.ops.wm.obj_export(
        filepath=path,
        export_selected_objects=True,
        export_materials=True,
        export_triangulated_mesh=True,
        forward_axis="NEGATIVE_Z",
        up_axis="Y",
    )
    obj.location = display_location


def main():
    clear_scene()
    material = make_material("Rock_Gray", (0.30, 0.32, 0.34))
    rocks = [
        create_round_rock(material),
        create_jagged_rock(material),
        create_slab_rock(material),
    ]

    os.makedirs(SOURCE_ROOT, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(SOURCE_ROOT, "Rocks.blend"))

    for rock, folder_name in zip(rocks, ("RockRound", "RockJagged", "RockSlab")):
        export_obj(rock, folder_name)

    print("Created 3 low-poly rocks:")
    for rock in rocks:
        print(f"  {rock.name}: {len(rock.data.vertices)} vertices, {len(rock.data.polygons)} faces")


if __name__ == "__main__":
    main()
