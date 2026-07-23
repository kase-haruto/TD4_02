import bpy
import importlib.util
import os


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SOURCE_PATH = os.path.join(SCRIPT_DIR, "create_lowpoly_swaying_grass.py")
MODEL_DIR = os.path.normpath(
    os.path.join(SCRIPT_DIR, "..", "..", "models", "LowPoly_SwayingGrass")
)
BLEND_PATH = os.path.join(SCRIPT_DIR, "LowPoly_SwayingGrass_Static.blend")
GLTF_PATH = os.path.join(MODEL_DIR, "LowPoly_SwayingGrass.gltf")


def load_source_module():
    spec = importlib.util.spec_from_file_location("grass_source", SOURCE_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main():
    source = load_source_module()
    source.clear_scene()
    source.configure_scene()

    materials = [
        source.make_material("Grass_Green", (0.12, 0.42, 0.08)),
        source.make_material("Grass_Light", (0.22, 0.58, 0.11)),
        source.make_material("Grass_Dark", (0.07, 0.29, 0.05)),
    ]
    grass, _ = source.create_grass_mesh(materials)
    grass.name = "LowPoly_SwayingGrass"
    grass.data.name = "LowPoly_SwayingGrass_Mesh"

    bpy.ops.object.select_all(action="DESELECT")
    grass.hide_set(False)
    grass.hide_render = False
    grass.select_set(True)
    bpy.context.view_layer.objects.active = grass

    os.makedirs(MODEL_DIR, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.ops.export_scene.gltf(
        filepath=GLTF_PATH,
        export_format="GLTF_SEPARATE",
        use_selection=True,
        export_animations=False,
        export_skins=False,
        export_morph=False,
        export_cameras=False,
        export_lights=False,
    )

    print(f"SAVED_BLEND={BLEND_PATH}")
    print(f"SAVED_GLTF={GLTF_PATH}")
    print(
        f"VERTICES={len(grass.data.vertices)} "
        f"FACES={len(grass.data.polygons)} "
        f"OBJECTS={len(bpy.context.scene.objects)}"
    )


if __name__ == "__main__":
    main()
