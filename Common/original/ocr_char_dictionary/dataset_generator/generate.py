#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import random
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
from fontTools.ttLib import TTCollection, TTFont

SUPPORTED_FONT_EXTS = {".ttf", ".otf", ".ttc"}


@dataclass(frozen=True)
class Palette:
    name: str
    fg: tuple[int, int, int]
    bg: tuple[int, int, int]


@dataclass(frozen=True)
class FontFace:
    path: Path
    index: int
    name: str
    codepoints: frozenset[int]


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def load_dictionary(path: Path) -> list[str]:
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    chars: list[str] = []
    seen: set[str] = set()

    for line_no, char in enumerate(lines, start=1):
        if char == "":
            raise ValueError(f"空行があります: {path}:{line_no}")
        if len(char) != 1:
            raise ValueError(
                f"1行1 Unicode文字ではありません: {path}:{line_no}: {char!r}"
            )
        if char in seen:
            raise ValueError(f"重複文字があります: {path}:{line_no}: {char!r}")
        seen.add(char)
        chars.append(char)

    if not chars:
        raise ValueError(f"辞書が空です: {path}")
    return chars


def load_palettes(path: Path) -> list[Palette]:
    raw = load_json(path)
    palettes: list[Palette] = []
    names: set[str] = set()

    for index, item in enumerate(raw):
        name = str(item["name"])
        if name in names:
            raise ValueError(f"パレット名が重複しています: {name}")
        names.add(name)

        fg = tuple(int(v) for v in item["fg"])
        bg = tuple(int(v) for v in item["bg"])
        if len(fg) != 3 or len(bg) != 3:
            raise ValueError(f"RGBは3要素必要です: palette {index}")
        if any(v < 0 or v > 255 for v in (*fg, *bg)):
            raise ValueError(f"RGBは0〜255で指定してください: palette {index}")

        palettes.append(Palette(name=name, fg=fg, bg=bg))

    if not palettes:
        raise ValueError("パレットがありません")
    return palettes


def font_display_name(font: TTFont, fallback: str) -> str:
    name_table = font.get("name")
    if name_table is None:
        return fallback

    for name_id in (4, 1):
        for record in name_table.names:
            if record.nameID == name_id:
                try:
                    value = record.toUnicode().strip()
                except Exception:
                    continue
                if value:
                    return value
    return fallback


def unicode_codepoints(font: TTFont) -> frozenset[int]:
    cmap = font.getBestCmap() or {}
    return frozenset(cmap.keys())


def font_faces_from_file(path: Path) -> list[FontFace]:
    suffix = path.suffix.lower()
    faces: list[FontFace] = []

    if suffix == ".ttc":
        collection = TTCollection(str(path), lazy=True)
        try:
            for index, font in enumerate(collection.fonts):
                fallback = f"{path.stem}#{index}"
                faces.append(
                    FontFace(
                        path=path,
                        index=index,
                        name=font_display_name(font, fallback),
                        codepoints=unicode_codepoints(font),
                    )
                )
        finally:
            collection.close()
    else:
        font = TTFont(str(path), lazy=True)
        try:
            faces.append(
                FontFace(
                    path=path,
                    index=0,
                    name=font_display_name(font, path.stem),
                    codepoints=unicode_codepoints(font),
                )
            )
        finally:
            font.close()

    return faces


def discover_font_faces(fonts_dir: Path) -> list[FontFace]:
    faces: list[FontFace] = []
    for path in sorted(fonts_dir.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SUPPORTED_FONT_EXTS:
            continue
        try:
            faces.extend(font_faces_from_file(path))
        except Exception as exc:
            print(f"[WARN] フォントを読み込めません: {path}: {exc}", file=sys.stderr)
    return faces


def safe_codepoint(char: str) -> str:
    return f"U+{ord(char):04X}"


def fit_font_size(
    char: str,
    face: FontFace,
    target_size: int,
    minimum: int = 8,
) -> int:
    canvas = Image.new("L", (target_size * 4, target_size * 4), 0)
    draw = ImageDraw.Draw(canvas)
    low, high = minimum, target_size * 2
    best = minimum

    while low <= high:
        size = (low + high) // 2
        font = ImageFont.truetype(str(face.path), size=size, index=face.index)
        bbox = draw.textbbox((0, 0), char, font=font)
        width = bbox[2] - bbox[0]
        height = bbox[3] - bbox[1]
        if width <= target_size and height <= target_size:
            best = size
            low = size + 1
        else:
            high = size - 1
    return best


def render_char_image(
    char: str,
    face: FontFace,
    image_size: int,
    margin: int,
    rotation_deg: float,
    palette: Palette,
    jitter_xy: tuple[int, int],
) -> Image.Image:
    target = image_size - margin * 2
    font_size = fit_font_size(char, face, target)
    font = ImageFont.truetype(str(face.path), size=font_size, index=face.index)

    probe = Image.new("L", (image_size * 4, image_size * 4), 0)
    probe_draw = ImageDraw.Draw(probe)
    bbox = probe_draw.textbbox((0, 0), char, font=font)
    width = max(1, bbox[2] - bbox[0])
    height = max(1, bbox[3] - bbox[1])

    glyph = Image.new("RGBA", (width + 12, height + 12), (0, 0, 0, 0))
    draw = ImageDraw.Draw(glyph)
    draw.text(
        (6 - bbox[0], 6 - bbox[1]),
        char,
        font=font,
        fill=palette.fg + (255,),
    )

    rotated = glyph.rotate(
        rotation_deg,
        resample=Image.Resampling.BICUBIC,
        expand=True,
    )
    alpha_bbox = rotated.getchannel("A").getbbox()
    if alpha_bbox is not None:
        rotated = rotated.crop(alpha_bbox)

    rw, rh = rotated.size
    scale = min(1.0, target / max(rw, 1), target / max(rh, 1))
    if scale < 1.0:
        rotated = rotated.resize(
            (max(1, round(rw * scale)), max(1, round(rh * scale))),
            Image.Resampling.LANCZOS,
        )

    out = Image.new("RGB", (image_size, image_size), palette.bg)
    x = (image_size - rotated.width) // 2 + jitter_xy[0]
    y = (image_size - rotated.height) // 2 + jitter_xy[1]
    out.paste(rotated.convert("RGB"), (x, y), rotated.getchannel("A"))
    return out


def deterministic_jitter(
    seed: int,
    class_id: int,
    font_id: int,
    rotation_id: int,
    palette_id: int,
    max_px: int,
) -> tuple[int, int]:
    if max_px <= 0:
        return (0, 0)
    payload = f"{seed}:{class_id}:{font_id}:{rotation_id}:{palette_id}".encode()
    digest = hashlib.sha256(payload).digest()
    rng = random.Random(int.from_bytes(digest[:8], "big"))
    return (rng.randint(-max_px, max_px), rng.randint(-max_px, max_px))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="1行1文字辞書からOCR学習用画像を生成します。"
    )
    parser.add_argument("--config", default="config.json")
    parser.add_argument("--dictionary")
    parser.add_argument("--output-dir")
    parser.add_argument("--fonts-dir")
    parser.add_argument("--palettes-file")
    parser.add_argument("--limit-chars", type=int)
    parser.add_argument("--seed", type=int)
    parser.add_argument("--overwrite", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def resolve_path(base: Path, value: str) -> Path:
    path = Path(value)
    return path if path.is_absolute() else (base / path).resolve()


def prepare_output_dir(path: Path, overwrite: bool) -> None:
    if path.exists() and any(path.iterdir()):
        if not overwrite:
            raise FileExistsError(
                f"出力先が空ではありません: {path} (--overwrite で作り直せます)"
            )
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def main() -> int:
    args = parse_args()
    config_path = Path(args.config).resolve()
    config = load_json(config_path)
    base_dir = config_path.parent

    dictionary_path = resolve_path(base_dir, args.dictionary or config["dictionary"])
    output_dir = resolve_path(base_dir, args.output_dir or config["output_dir"])
    fonts_dir = resolve_path(base_dir, args.fonts_dir or config["fonts_dir"])
    palettes_path = resolve_path(
        base_dir, args.palettes_file or config["palettes_file"]
    )

    image_size = int(config.get("image_size", 64))
    margin = int(config.get("margin", 4))
    rotations = [float(v) for v in config.get("rotations", [])]
    file_format = str(config.get("file_format", "png")).lower()
    overwrite = bool(args.overwrite or config.get("overwrite", False))
    seed = int(args.seed if args.seed is not None else config.get("seed", 42))
    jitter = int(config.get("position_jitter_px", 0))
    limit_chars = (
        args.limit_chars
        if args.limit_chars is not None
        else config.get("limit_chars")
    )

    if image_size <= 0 or margin < 0 or margin * 2 >= image_size:
        raise ValueError("image_size / margin の設定が不正です")
    if jitter < 0 or jitter > margin:
        raise ValueError("position_jitter_px は0以上margin以下にしてください")
    if not rotations:
        raise ValueError("rotations が空です")
    if file_format not in {"png", "jpg", "jpeg", "webp"}:
        raise ValueError("file_format は png / jpg / jpeg / webp のいずれかです")

    chars = load_dictionary(dictionary_path)
    if limit_chars is not None:
        chars = chars[: int(limit_chars)]
    palettes = load_palettes(palettes_path)
    faces = discover_font_faces(fonts_dir)
    if not faces:
        raise FileNotFoundError(
            f"TTF/OTF/TTCフォントが見つかりません: {fonts_dir}"
        )

    possible = len(chars) * len(faces) * len(rotations) * len(palettes)
    print(f"[INFO] dictionary : {dictionary_path}")
    print(f"[INFO] chars      : {len(chars)}")
    print(f"[INFO] font faces : {len(faces)}")
    print(f"[INFO] rotations  : {len(rotations)}")
    print(f"[INFO] palettes   : {len(palettes)}")
    print(f"[INFO] max images : {possible:,}")

    for i, face in enumerate(faces):
        print(f"[FONT {i:02d}] {face.name} ({face.path.name}#{face.index})")

    if args.dry_run:
        return 0

    prepare_output_dir(output_dir, overwrite)
    images_dir = output_dir / "images"
    images_dir.mkdir(parents=True, exist_ok=True)
    labels_path = output_dir / "labels.csv"
    skipped_path = output_dir / "skipped.csv"

    generated = 0
    skipped = 0

    with (
        labels_path.open("w", encoding="utf-8", newline="") as labels_file,
        skipped_path.open("w", encoding="utf-8", newline="") as skipped_file,
    ):
        labels = csv.writer(labels_file)
        labels.writerow(
            [
                "class_id",
                "char",
                "codepoint",
                "font_id",
                "font_name",
                "font_file",
                "font_index",
                "rotation_deg",
                "palette_id",
                "palette_name",
                "image_relpath",
            ]
        )
        skipped_writer = csv.writer(skipped_file)
        skipped_writer.writerow(
            ["class_id", "char", "codepoint", "font_id", "font_name", "reason"]
        )

        for class_id, char in enumerate(chars):
            codepoint = ord(char)
            cp_text = safe_codepoint(char)
            class_dir = images_dir / f"{class_id:06d}"

            for font_id, face in enumerate(faces):
                if codepoint not in face.codepoints:
                    skipped_writer.writerow(
                        [
                            class_id,
                            char,
                            cp_text,
                            font_id,
                            face.name,
                            "glyph_not_in_font_cmap",
                        ]
                    )
                    skipped += 1
                    continue

                class_dir.mkdir(parents=True, exist_ok=True)
                for rotation_id, rotation in enumerate(rotations):
                    for palette_id, palette in enumerate(palettes):
                        jitter_xy = deterministic_jitter(
                            seed,
                            class_id,
                            font_id,
                            rotation_id,
                            palette_id,
                            jitter,
                        )
                        image = render_char_image(
                            char=char,
                            face=face,
                            image_size=image_size,
                            margin=margin,
                            rotation_deg=rotation,
                            palette=palette,
                            jitter_xy=jitter_xy,
                        )
                        filename = (
                            f"{class_id:06d}_{cp_text}_"
                            f"f{font_id:03d}_r{rotation_id:02d}_p{palette_id:02d}."
                            f"{file_format}"
                        )
                        out_path = class_dir / filename
                        image.save(out_path)
                        relpath = out_path.relative_to(output_dir).as_posix()
                        labels.writerow(
                            [
                                class_id,
                                char,
                                cp_text,
                                font_id,
                                face.name,
                                face.path.name,
                                face.index,
                                rotation,
                                palette_id,
                                palette.name,
                                relpath,
                            ]
                        )
                        generated += 1
                        if generated % 1000 == 0:
                            print(f"[INFO] generated: {generated:,}")

    summary = {
        "dictionary": str(dictionary_path),
        "num_chars": len(chars),
        "font_faces": [
            {
                "font_id": i,
                "name": face.name,
                "file": face.path.name,
                "index": face.index,
            }
            for i, face in enumerate(faces)
        ],
        "image_size": image_size,
        "margin": margin,
        "rotations": rotations,
        "num_palettes": len(palettes),
        "file_format": file_format,
        "seed": seed,
        "position_jitter_px": jitter,
        "max_images_before_glyph_filter": possible,
        "generated_images": generated,
        "skipped_char_font_pairs": skipped,
    }
    (output_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    print(f"[DONE] images     : {generated:,}")
    print(f"[DONE] skipped    : {skipped:,} char/font pairs")
    print(f"[DONE] labels     : {labels_path}")
    print(f"[DONE] skipped csv: {skipped_path}")
    print(f"[DONE] summary    : {output_dir / 'summary.json'}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, FileNotFoundError, FileExistsError) as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        raise SystemExit(2)
