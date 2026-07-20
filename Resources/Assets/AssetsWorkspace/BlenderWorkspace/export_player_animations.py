"""Export every Player action as an individual, texture-free glTF file.

Run with Blender 4.4 or newer:
  blender --background ../Player.blend --python export_player_animations.py
"""

from __future__ import annotations

import json
import re
from pathlib import Path

import bpy


OUTPUT_DIR = Path(__file__).resolve().parent
EXPECTED_SOURCE = OUTPUT_DIR.parent / "Player.blend"


def safe_action_name(name: str) -> str:
    """Keep the existing action name while making it safe as a filename."""
    value = re.sub(r'[<>:"/\\|?*\x00-\x1f]', "_", name).strip(" .")
    if not value:
        raise RuntimeError(f"Action name cannot be converted to a filename: {name!r}")
    return value


def clear_active_animations() -> None:
    for obj in bpy.data.objects:
        animation_data = obj.animation_data
        if animation_data is None:
            continue
        animation_data.action = None
        for track in animation_data.nla_tracks:
            track.mute = True


def validate_export(gltf_path: Path, expected_action: str) -> dict[str, object]:
    data = json.loads(gltf_path.read_text(encoding="utf-8"))
    animation_names = [animation.get("name") for animation in data.get("animations", [])]
    if animation_names != [expected_action]:
        raise RuntimeError(
            f"{gltf_path.name}: expected one animation named {expected_action!r}, "
            f"got {animation_names!r}"
        )
    if data.get("images") or data.get("textures") or data.get("materials"):
        raise RuntimeError(f"{gltf_path.name}: texture or material data was exported")

    buffers = data.get("buffers", [])
    if len(buffers) != 1 or not buffers[0].get("uri"):
        raise RuntimeError(f"{gltf_path.name}: expected one external binary buffer")
    bin_path = gltf_path.parent / buffers[0]["uri"]
    if not bin_path.is_file() or bin_path.stat().st_size == 0:
        raise RuntimeError(f"{gltf_path.name}: missing or empty buffer {bin_path.name}")

    return {
        "file": gltf_path.name,
        "animation": expected_action,
        "frame_range": list(bpy.data.actions[expected_action].frame_range),
        "nodes": len(data.get("nodes", [])),
        "meshes": len(data.get("meshes", [])),
        "skins": len(data.get("skins", [])),
        "bin": bin_path.name,
        "bin_bytes": bin_path.stat().st_size,
    }


def main() -> None:
    source = Path(bpy.data.filepath).resolve()
    if source != EXPECTED_SOURCE.resolve():
        raise RuntimeError(f"Expected source {EXPECTED_SOURCE}, opened {source}")

    action_names = sorted(action.name for action in bpy.data.actions)
    if not action_names:
        raise RuntimeError("Player.blend contains no actions")

    animated_armatures = [
        obj
        for obj in bpy.data.objects
        if obj.type == "ARMATURE"
        and obj.animation_data is not None
        and (
            obj.animation_data.action is not None
            or any(track.strips for track in obj.animation_data.nla_tracks)
        )
    ]
    if len(animated_armatures) != 1:
        raise RuntimeError(
            f"Expected one animated armature, found {[obj.name for obj in animated_armatures]}"
        )
    armature = animated_armatures[0]

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    results: list[dict[str, object]] = []
    clear_active_animations()
    exported_objects = [
        obj
        for obj in bpy.data.objects
        if obj == armature or (obj.type == "MESH" and obj.visible_get())
    ]

    def enable_layer_collections(layer_collection: bpy.types.LayerCollection) -> None:
        layer_collection.exclude = False
        layer_collection.hide_viewport = False
        for child in layer_collection.children:
            enable_layer_collections(child)

    enable_layer_collections(bpy.context.view_layer.layer_collection)
    armature.hide_set(False)
    bpy.context.view_layer.update()
    bpy.ops.object.select_all(action="DESELECT")
    for obj in exported_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = armature

    for action_name in action_names:
        action = bpy.data.actions[action_name]
        armature.animation_data_create()
        armature.animation_data.action = action
        bpy.context.scene.frame_start = int(action.frame_range[0])
        bpy.context.scene.frame_end = int(action.frame_range[1])

        basename = f"Player_{safe_action_name(action_name)}"
        gltf_path = OUTPUT_DIR / f"{basename}.gltf"
        result = bpy.ops.export_scene.gltf(
            filepath=str(gltf_path),
            export_format="GLTF_SEPARATE",
            export_animations=True,
            export_animation_mode="ACTIVE_ACTIONS",
            export_nla_strips=False,
            export_nla_strips_merged_animation_name=action_name,
            export_frame_range=True,
            export_force_sampling=True,
            export_materials="NONE",
            use_selection=True,
            export_cameras=False,
            export_lights=False,
        )
        if result != {"FINISHED"}:
            raise RuntimeError(f"{gltf_path.name}: Blender exporter returned {result}")
        results.append(validate_export(gltf_path, action_name))

    print("CODEX_PLAYER_EXPORT_RESULTS=" + json.dumps(results, ensure_ascii=False))


if __name__ == "__main__":
    main()
