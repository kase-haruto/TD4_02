"""Export Enemy1 actions and overwrite the matching models/Enemy_01_* glTFs.

Run with Blender 4.4 or newer:
  blender --background ../Enemy.blend --python export_enemy1_animations.py
"""

from __future__ import annotations

import hashlib
import json
import math
import re
import shutil
from pathlib import Path

import bpy


WORKSPACE_DIR = Path(__file__).resolve().parent
EXPECTED_SOURCE = WORKSPACE_DIR.parent / "Enemy.blend"
MODELS_DIR = WORKSPACE_DIR.parents[1] / "models"


def safe_action_name(name: str) -> str:
    value = re.sub(r'[<>:"/\\|?*\x00-\x1f]', "_", name).strip(" .")
    if not value:
        raise RuntimeError(f"Invalid action filename: {name!r}")
    return value


def clear_active_animations() -> None:
    for obj in bpy.data.objects:
        animation_data = obj.animation_data
        if animation_data is None:
            continue
        animation_data.action = None
        for track in animation_data.nla_tracks:
            track.mute = True


def reverse_forward_direction() -> None:
    """Turn every scene root 180 degrees around Blender's vertical Z axis."""
    for obj in bpy.context.scene.objects:
        if obj.parent is None:
            obj.rotation_euler.rotate_axis("Z", math.pi)


def select_visible_rig(armature: bpy.types.Object) -> list[bpy.types.Object]:
    """Select only visible meshes driven by the armature, plus the required rig."""
    rigged_meshes = []
    for obj in bpy.context.scene.objects:
        if obj.type != "MESH" or not obj.visible_get():
            continue
        parent = obj.parent
        parented_to_rig = False
        while parent is not None:
            if parent == armature:
                parented_to_rig = True
                break
            parent = parent.parent
        modified_by_rig = any(
            modifier.type == "ARMATURE" and modifier.object == armature
            for modifier in obj.modifiers
        )
        if parented_to_rig or modified_by_rig:
            rigged_meshes.append(obj)

    if not rigged_meshes:
        raise RuntimeError("No visible mesh is driven by the Enemy1 armature")

    bpy.ops.object.select_all(action="DESELECT")
    # The rig may be hidden in the viewport for authoring, but it is a required
    # dependency of the visible skinned mesh.
    armature.hide_set(False)
    armature.select_set(True)
    for obj in rigged_meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = armature
    return rigged_meshes


def file_hash(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def validate_export(gltf_path: Path, expected_action: str) -> tuple[dict, Path]:
    data = json.loads(gltf_path.read_text(encoding="utf-8"))
    animation_names = [animation.get("name") for animation in data.get("animations", [])]
    if animation_names != [expected_action]:
        raise RuntimeError(
            f"{gltf_path.name}: expected [{expected_action!r}], got {animation_names!r}"
        )
    if data.get("images") or data.get("textures") or data.get("materials"):
        raise RuntimeError(f"{gltf_path.name}: texture or material data was exported")
    if data.get("cameras"):
        raise RuntimeError(f"{gltf_path.name}: camera data was exported")

    buffers = data.get("buffers", [])
    if len(buffers) != 1 or not buffers[0].get("uri"):
        raise RuntimeError(f"{gltf_path.name}: expected one external buffer")
    bin_path = gltf_path.parent / buffers[0]["uri"]
    if not bin_path.is_file() or bin_path.stat().st_size == 0:
        raise RuntimeError(f"{gltf_path.name}: missing or empty {bin_path.name}")
    return data, bin_path


def main() -> None:
    source = Path(bpy.data.filepath).resolve()
    if source != EXPECTED_SOURCE.resolve():
        raise RuntimeError(f"Expected source {EXPECTED_SOURCE}, opened {source}")

    action_names = sorted(action.name for action in bpy.data.actions)
    expected_actions = {"attack", "damage", "idle", "move"}
    if set(action_names) != expected_actions:
        raise RuntimeError(
            f"Expected Enemy1 actions {sorted(expected_actions)}, got {action_names}"
        )

    animated_armatures = [
        obj
        for obj in bpy.data.objects
        if obj.type == "ARMATURE" and obj.animation_data is not None
    ]
    if len(animated_armatures) != 1:
        raise RuntimeError(
            f"Expected one armature, found {[obj.name for obj in animated_armatures]}"
        )
    armature = animated_armatures[0]

    WORKSPACE_DIR.mkdir(parents=True, exist_ok=True)
    clear_active_animations()
    reverse_forward_direction()
    selected_meshes = select_visible_rig(armature)
    results = []

    for action_name in action_names:
        action = bpy.data.actions[action_name]
        armature.animation_data_create()
        armature.animation_data.action = action
        bpy.context.scene.frame_start = int(action.frame_range[0])
        bpy.context.scene.frame_end = int(action.frame_range[1])

        basename = f"Enemy_01_{safe_action_name(action_name)}"
        gltf_path = WORKSPACE_DIR / f"{basename}.gltf"
        result = bpy.ops.export_scene.gltf(
            filepath=str(gltf_path),
            export_format="GLTF_SEPARATE",
            use_selection=True,
            export_animations=True,
            export_animation_mode="ACTIVE_ACTIONS",
            export_nla_strips=False,
            export_nla_strips_merged_animation_name=action_name,
            export_frame_range=True,
            export_force_sampling=True,
            export_materials="NONE",
            export_cameras=False,
            export_lights=False,
        )
        if result != {"FINISHED"}:
            raise RuntimeError(f"{gltf_path.name}: exporter returned {result}")

        data, bin_path = validate_export(gltf_path, action_name)
        target_dir = MODELS_DIR / basename
        target_gltf = target_dir / gltf_path.name
        target_bin = target_dir / bin_path.name
        if not target_gltf.is_file() or not target_bin.is_file():
            raise RuntimeError(f"Missing existing Enemy1 target files in {target_dir}")

        meta_path = target_dir / f"{gltf_path.name}.meta"
        meta_hash = file_hash(meta_path) if meta_path.is_file() else None
        shutil.copy2(gltf_path, target_gltf)
        shutil.copy2(bin_path, target_bin)

        if file_hash(gltf_path) != file_hash(target_gltf):
            raise RuntimeError(f"{target_gltf}: copied glTF hash mismatch")
        if file_hash(bin_path) != file_hash(target_bin):
            raise RuntimeError(f"{target_bin}: copied buffer hash mismatch")
        if meta_hash is not None and file_hash(meta_path) != meta_hash:
            raise RuntimeError(f"{meta_path}: metadata changed")

        results.append(
            {
                "action": action_name,
                "gltf": str(target_gltf),
                "bin": str(target_bin),
                "nodes": len(data.get("nodes", [])),
                "meshes": len(data.get("meshes", [])),
                "skins": len(data.get("skins", [])),
                "selected_meshes": [obj.name for obj in selected_meshes],
                "orientation": "Z+180deg",
            }
        )

    print("CODEX_ENEMY1_EXPORT_RESULTS=" + json.dumps(results, ensure_ascii=False))


if __name__ == "__main__":
    main()
