"""Export reversed Enemy3 actions and overwrite models/Enemy_03_* glTFs."""

from __future__ import annotations

import hashlib
import json
import math
import re
import shutil
from pathlib import Path

import bpy


WORKSPACE_DIR = Path(__file__).resolve().parent
EXPECTED_SOURCE = WORKSPACE_DIR.parent / "Enemy3.blend"
MODELS_DIR = WORKSPACE_DIR.parents[1] / "models"
EXPECTED_ACTIONS = {"attack1", "attack2", "defence", "idle"}


def safe_name(name: str) -> str:
    value = re.sub(r'[<>:"/\\|?*\x00-\x1f]', "_", name).strip(" .")
    if not value:
        raise RuntimeError(f"Invalid action filename: {name!r}")
    return value


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def prepare_scene() -> bpy.types.Object:
    armatures = [
        obj
        for obj in bpy.data.objects
        if obj.type == "ARMATURE" and obj.animation_data is not None
    ]
    if len(armatures) != 1:
        raise RuntimeError(f"Expected one animated armature, found {len(armatures)}")

    for obj in bpy.data.objects:
        if obj.animation_data is not None:
            obj.animation_data.action = None
            for track in obj.animation_data.nla_tracks:
                track.mute = True
    for obj in bpy.context.scene.objects:
        if obj.parent is None:
            obj.rotation_euler.rotate_axis("Z", math.pi)
    return armatures[0]


def validate(gltf_path: Path, action_name: str) -> tuple[dict, Path]:
    data = json.loads(gltf_path.read_text(encoding="utf-8"))
    names = [animation.get("name") for animation in data.get("animations", [])]
    if names != [action_name]:
        raise RuntimeError(f"{gltf_path.name}: expected [{action_name!r}], got {names!r}")
    if (
        data.get("images")
        or data.get("textures")
        or data.get("materials")
        or data.get("cameras")
    ):
        raise RuntimeError(f"{gltf_path.name}: disallowed render resources were exported")
    buffers = data.get("buffers", [])
    if len(buffers) != 1 or not buffers[0].get("uri"):
        raise RuntimeError(f"{gltf_path.name}: expected one external buffer")
    bin_path = gltf_path.parent / buffers[0]["uri"]
    if not bin_path.is_file() or bin_path.stat().st_size == 0:
        raise RuntimeError(f"{gltf_path.name}: missing or empty binary buffer")
    return data, bin_path


def main() -> None:
    source = Path(bpy.data.filepath).resolve()
    if source != EXPECTED_SOURCE.resolve():
        raise RuntimeError(f"Expected source {EXPECTED_SOURCE}, opened {source}")

    action_names = sorted(action.name for action in bpy.data.actions)
    if set(action_names) != EXPECTED_ACTIONS:
        raise RuntimeError(
            f"Expected actions {sorted(EXPECTED_ACTIONS)}, got {action_names}"
        )

    armature = prepare_scene()
    results = []
    for action_name in action_names:
        action = bpy.data.actions[action_name]
        armature.animation_data.action = action
        bpy.context.scene.frame_start = int(action.frame_range[0])
        bpy.context.scene.frame_end = int(action.frame_range[1])

        basename = f"Enemy_03_{safe_name(action_name)}"
        gltf_path = WORKSPACE_DIR / f"{basename}.gltf"
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
            export_cameras=False,
            export_lights=False,
        )
        if result != {"FINISHED"}:
            raise RuntimeError(f"{gltf_path.name}: exporter returned {result}")

        data, bin_path = validate(gltf_path, action_name)
        target_dir = MODELS_DIR / basename
        target_gltf = target_dir / gltf_path.name
        target_bin = target_dir / bin_path.name
        if not target_gltf.is_file() or not target_bin.is_file():
            raise RuntimeError(f"Missing existing target files in {target_dir}")

        meta_path = target_dir / f"{gltf_path.name}.meta"
        meta_hash = sha256(meta_path) if meta_path.is_file() else None
        shutil.copy2(gltf_path, target_gltf)
        shutil.copy2(bin_path, target_bin)
        if sha256(gltf_path) != sha256(target_gltf):
            raise RuntimeError(f"{target_gltf}: glTF hash mismatch")
        if sha256(bin_path) != sha256(target_bin):
            raise RuntimeError(f"{target_bin}: buffer hash mismatch")
        if meta_hash is not None and sha256(meta_path) != meta_hash:
            raise RuntimeError(f"{meta_path}: metadata changed")

        results.append(
            {
                "action": action_name,
                "gltf": str(target_gltf),
                "bin": str(target_bin),
                "nodes": len(data.get("nodes", [])),
                "meshes": len(data.get("meshes", [])),
                "skins": len(data.get("skins", [])),
                "orientation": "Z+180deg",
            }
        )

    print("CODEX_ENEMY3_EXPORT_RESULTS=" + json.dumps(results, ensure_ascii=False))


if __name__ == "__main__":
    main()
