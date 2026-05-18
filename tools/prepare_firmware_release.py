#!/usr/bin/env python3
"""Prepare firmware release assets and the static web flasher site."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path


PRODUCT_NAME = "RCJ Soccer Communication Module"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-dir", required=True, help="ESP-IDF project directory")
    parser.add_argument("--web-src", required=True, help="Static web flasher source directory")
    parser.add_argument("--output-dir", required=True, help="Output directory for release files")
    parser.add_argument("--version", required=True, help="Firmware version, usually the Git tag")
    return parser.parse_args()


def safe_name(value: str) -> str:
    value = re.sub(r"[^A-Za-z0-9_.-]+", "-", value).strip(".-")
    return value or "firmware"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def chip_family(chip: str) -> str:
    match = re.fullmatch(r"esp32([a-z]\d*)?", chip.lower())
    if not match:
        raise ValueError(f"Unsupported chip name from flasher_args.json: {chip}")
    suffix = match.group(1)
    return "ESP32" if suffix is None else f"ESP32-{suffix.upper()}"


def copy_static_site(web_src: Path, output_dir: Path) -> None:
    if output_dir.exists():
        shutil.rmtree(output_dir)
    shutil.copytree(web_src, output_dir)


def copy_optional_assets(repo_root: Path, output_dir: Path) -> None:
    image = repo_root / ".readme_images" / "modul_2024.png"
    if not image.exists():
        return

    assets_dir = output_dir / "assets"
    assets_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(image, assets_dir / image.name)


def load_flasher_args(project_dir: Path) -> dict:
    flasher_args_path = project_dir / "build" / "flasher_args.json"
    if not flasher_args_path.exists():
        raise FileNotFoundError(f"Missing build output: {flasher_args_path}")

    with flasher_args_path.open("r", encoding="utf-8") as file:
        return json.load(file)


def copy_build_file(build_dir: Path, relative_path: str, destination: Path) -> Path:
    source = build_dir / relative_path
    if not source.exists():
        raise FileNotFoundError(f"Missing build output: {source}")
    shutil.copy2(source, destination)
    return destination


def merge_firmware(build_dir: Path, flasher_args: dict, output_path: Path) -> None:
    settings = flasher_args["flash_settings"]
    chip = flasher_args["extra_esptool_args"]["chip"]
    flash_files = sorted(
        flasher_args["flash_files"].items(),
        key=lambda item: int(item[0], 16),
    )

    command = [
        sys.executable,
        "-m",
        "esptool",
        "--chip",
        chip,
        "merge_bin",
        "-o",
        str(output_path),
        "--flash_mode",
        settings["flash_mode"],
        "--flash_freq",
        settings["flash_freq"],
        "--flash_size",
        settings["flash_size"],
    ]

    for offset, relative_path in flash_files:
        path = build_dir / relative_path
        if not path.exists():
            raise FileNotFoundError(f"Missing build output: {path}")
        command.extend([offset, str(path)])

    subprocess.run(command, check=True)


def write_json(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def release_url(version: str) -> str | None:
    repository = os.environ.get("GITHUB_REPOSITORY")
    server_url = os.environ.get("GITHUB_SERVER_URL", "https://github.com")
    if not repository:
        return None
    return f"{server_url}/{repository}/releases/tag/{version}"


def main() -> int:
    args = parse_args()
    project_dir = Path(args.project_dir).resolve()
    web_src = Path(args.web_src).resolve()
    output_dir = Path(args.output_dir).resolve()
    repo_root = project_dir.parent.parent
    build_dir = project_dir / "build"
    version = args.version
    file_version = safe_name(version)

    flasher_args = load_flasher_args(project_dir)
    chip = flasher_args["extra_esptool_args"]["chip"]
    firmware_dir = output_dir / "firmware"

    copy_static_site(web_src, output_dir)
    copy_optional_assets(repo_root, output_dir)
    firmware_dir.mkdir(parents=True, exist_ok=True)

    app_bin = firmware_dir / f"rcj_comm_module-{file_version}.bin"
    bootloader_bin = firmware_dir / f"rcj_comm_module-{file_version}-bootloader.bin"
    partition_bin = firmware_dir / f"rcj_comm_module-{file_version}-partition-table.bin"
    merged_bin = firmware_dir / f"rcj_comm_module-{file_version}-merged.bin"

    copy_build_file(build_dir, flasher_args["app"]["file"], app_bin)
    copy_build_file(build_dir, flasher_args["bootloader"]["file"], bootloader_bin)
    copy_build_file(build_dir, flasher_args["partition-table"]["file"], partition_bin)
    merge_firmware(build_dir, flasher_args, merged_bin)

    files = {
        "app": app_bin,
        "bootloader": bootloader_bin,
        "partitionTable": partition_bin,
        "merged": merged_bin,
    }

    checksums = {
        name: {
            "path": path.relative_to(output_dir).as_posix(),
            "size": path.stat().st_size,
            "sha256": sha256(path),
        }
        for name, path in files.items()
    }

    manifest = {
        "name": PRODUCT_NAME,
        "version": version,
        "new_install_prompt_erase": True,
        "new_install_improv_wait_time": 0,
        "builds": [
            {
                "chipFamily": chip_family(chip),
                "parts": [
                    {
                        "path": checksums["merged"]["path"],
                        "offset": 0,
                    }
                ],
            }
        ],
    }

    metadata = {
        "name": PRODUCT_NAME,
        "version": version,
        "chip": chip,
        "chipFamily": chip_family(chip),
        "releaseUrl": release_url(version),
        "commit": os.environ.get("GITHUB_SHA"),
        "files": checksums,
    }

    notes = [
        f"# {PRODUCT_NAME} {version}",
        "",
        "Built automatically from the firmware tag.",
        "",
        "## Files",
        "",
        f"- Web flasher image: `{checksums['merged']['path']}`",
        f"- Application binary: `{checksums['app']['path']}`",
        f"- Bootloader: `{checksums['bootloader']['path']}`",
        f"- Partition table: `{checksums['partitionTable']['path']}`",
        "",
        "The web flasher uses the merged binary at flash offset `0x0`.",
        "",
    ]

    write_json(output_dir / "manifest.json", manifest)
    write_json(output_dir / "version.json", metadata)
    (output_dir / "release-notes.md").write_text("\n".join(notes), encoding="utf-8")

    print(f"Prepared release files in {output_dir}")
    print(f"Manifest uses {manifest['builds'][0]['chipFamily']} merged firmware")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
