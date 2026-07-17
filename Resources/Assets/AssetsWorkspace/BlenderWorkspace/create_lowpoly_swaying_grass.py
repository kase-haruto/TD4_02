import bpy
import math
import os
from mathutils import Vector


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
BLEND_PATH = os.path.join(OUTPUT_DIR, "LowPoly_SwayingGrass.blend")
GLTF_PATH = os.path.join(OUTPUT_DIR, "LowPoly_SwayingGrass.gltf")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.armatures,
                       bpy.data.materials, bpy.data.cameras, bpy.data.lights):
        for datablock in list(datablocks):
            if datablock.users == 0:
                datablocks.remove(datablock)


def make_material(name, color):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = False
    mat.roughness = 0.85
    return mat


def create_grass_mesh(materials):
    # x, y, height, width, lean-x, lean-y, material index
    blades = [
        (0.00, 0.00, 2.25, 0.17,  0.10,  0.03, 0),
        (0.18, 0.06, 1.85, 0.16,  0.20, -0.04, 1),
        (-0.18, 0.05, 2.00, 0.15, -0.18,  0.05, 2),
        (0.08, -0.17, 1.65, 0.18, 0.04, -0.22, 1),
        (-0.14, -0.15, 1.72, 0.15, -0.10, -0.18, 0),
        (0.28, -0.08, 1.42, 0.16, 0.25, -0.08, 2),
        (-0.29, -0.03, 1.52, 0.14, -0.24, -0.02, 1),
        (0.02, 0.23, 1.58, 0.15, 0.02, 0.22, 0),
        (-0.08, 0.27, 1.30, 0.16, -0.08, 0.25, 2),
    ]

    verts = []
    faces = []
    face_materials = []
    vertex_heights = []
    levels = (0.0, 0.20, 0.42, 0.64, 0.83, 1.0)

    for blade_index, (bx, by, height, width, lean_x, lean_y, mat_index) in enumerate(blades):
        angle = blade_index * 2.399963 + 0.28
        side = Vector((math.cos(angle), math.sin(angle), 0.0))
        base_index = len(verts)

        for t in levels:
            # A gentle pre-bend keeps the silhouette organic while remaining low-poly.
            curve = t * t
            center = Vector((bx + lean_x * curve, by + lean_y * curve, height * t))
            half_width = width * (1.0 - t) * 0.5
            if t == 1.0:
                half_width = 0.0
            left = center - side * half_width
            right = center + side * half_width
            verts.extend((tuple(left), tuple(right)))
            vertex_heights.extend((center.z, center.z))

        for level in range(len(levels) - 1):
            a = base_index + level * 2
            b = a + 1
            c = a + 3
            d = a + 2
            faces.append((a, b, c, d))
            face_materials.append(mat_index)

    mesh = bpy.data.meshes.new("GrassBlades_Mesh")
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new("GrassBlades", mesh)
    bpy.context.collection.objects.link(obj)
    for mat in materials:
        obj.data.materials.append(mat)
    for polygon, mat_index in zip(mesh.polygons, face_materials):
        polygon.material_index = mat_index
        polygon.use_smooth = False

    return obj, vertex_heights


def create_armature():
    arm_data = bpy.data.armatures.new("GrassRig")
    arm_obj = bpy.data.objects.new("GrassRig", arm_data)
    bpy.context.collection.objects.link(arm_obj)
    arm_obj.show_in_front = True
    arm_obj.data.display_type = "STICK"

    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    bone_specs = [
        ("root", (0, 0, 0.00), (0, 0, 0.28), None),
        ("sway_lower", (0, 0, 0.28), (0, 0, 0.95), "root"),
        ("sway_mid", (0, 0, 0.95), (0, 0, 1.62), "sway_lower"),
        ("sway_tip", (0, 0, 1.62), (0, 0, 2.28), "sway_mid"),
    ]
    edit_bones = {}
    for name, head, tail, parent_name in bone_specs:
        bone = arm_data.edit_bones.new(name)
        bone.head = head
        bone.tail = tail
        bone.use_deform = True
        if parent_name:
            bone.parent = edit_bones[parent_name]
            bone.use_connect = True
        edit_bones[name] = bone

    bpy.ops.object.mode_set(mode="POSE")
    for pose_bone in arm_obj.pose.bones:
        pose_bone.rotation_mode = "XYZ"
    bpy.ops.object.mode_set(mode="OBJECT")
    return arm_obj


def bind_mesh(obj, arm_obj, vertex_heights):
    groups = {name: obj.vertex_groups.new(name=name) for name in
              ("root", "sway_lower", "sway_mid", "sway_tip")}

    # Linear blends at joints keep the very small mesh visibly smooth in motion.
    for vertex_index, z in enumerate(vertex_heights):
        if z <= 0.28:
            weights = {"root": 1.0}
        elif z <= 0.95:
            t = (z - 0.28) / (0.95 - 0.28)
            weights = {"root": 1.0 - t, "sway_lower": t}
        elif z <= 1.62:
            t = (z - 0.95) / (1.62 - 0.95)
            weights = {"sway_lower": 1.0 - t, "sway_mid": t}
        else:
            t = min(1.0, (z - 1.62) / (2.28 - 1.62))
            weights = {"sway_mid": 1.0 - t, "sway_tip": t}
        for group_name, weight in weights.items():
            if weight > 0.0001:
                groups[group_name].add([vertex_index], weight, "REPLACE")

    modifier = obj.modifiers.new("GrassArmature", "ARMATURE")
    modifier.object = arm_obj
    obj.parent = arm_obj


def animate_sway(arm_obj):
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 64
    scene.render.fps = 24

    action = bpy.data.actions.new("Grass_Sway_Loop")
    arm_obj.animation_data_create()
    arm_obj.animation_data.action = action

    frames = (1, 9, 17, 25, 33, 41, 49, 57, 64)
    # Wind reverses direction softly; the last frame matches the first.
    wave = (0.0, 0.65, 1.0, 0.55, 0.0, -0.65, -1.0, -0.55, 0.0)
    bone_strength = {
        "sway_lower": (math.radians(5.0), math.radians(7.0)),
        "sway_mid": (math.radians(9.0), math.radians(12.0)),
        "sway_tip": (math.radians(13.0), math.radians(17.0)),
    }

    for bone_name, (x_strength, y_strength) in bone_strength.items():
        bone = arm_obj.pose.bones[bone_name]
        for frame, value in zip(frames, wave):
            # A phase-varying secondary axis avoids a mechanical pendulum look.
            side_value = math.sin((frame - 1) / 63.0 * math.tau) * 0.32
            bone.rotation_euler = (x_strength * value,
                                   y_strength * value + x_strength * side_value,
                                   math.radians(1.5) * value)
            bone.keyframe_insert("rotation_euler", frame=frame, group=bone_name)

    for fcurve in action.fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "BEZIER"
            keyframe.handle_left_type = "AUTO_CLAMPED"
            keyframe.handle_right_type = "AUTO_CLAMPED"
        cycles = fcurve.modifiers.new("CYCLES")
        cycles.mode_before = "REPEAT"
        cycles.mode_after = "REPEAT"


def configure_scene():
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene.world.color = (0.05, 0.05, 0.05)
    scene.tool_settings.transform_pivot_point = "MEDIAN_POINT"


def main():
    clear_scene()
    configure_scene()
    materials = [
        make_material("Grass_Green", (0.12, 0.42, 0.08)),
        make_material("Grass_Light", (0.22, 0.58, 0.11)),
        make_material("Grass_Dark", (0.07, 0.29, 0.05)),
    ]
    grass, heights = create_grass_mesh(materials)
    rig = create_armature()
    bind_mesh(grass, rig, heights)
    animate_sway(rig)

    grass.select_set(True)
    rig.select_set(True)
    bpy.context.view_layer.objects.active = rig
    bpy.context.scene.frame_set(1)

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.ops.export_scene.gltf(
        filepath=GLTF_PATH,
        export_format="GLTF_SEPARATE",
        use_selection=True,
        export_animations=True,
        export_skins=True,
        export_morph=False,
        export_cameras=False,
        export_lights=False,
    )
    print(f"SAVED_BLEND={BLEND_PATH}")
    print(f"SAVED_GLTF={GLTF_PATH}")
    print(f"VERTICES={len(grass.data.vertices)} FACES={len(grass.data.polygons)} BONES={len(rig.data.bones)}")


if __name__ == "__main__":
    main()
