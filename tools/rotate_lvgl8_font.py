#!/usr/bin/env python3
"""
rotate_lvgl8_font.py — Rotate every glyph in an LVGL 8 font .c file by 90°.

Requires Python 3.8+, no extra packages.

The file produced is a valid LVGL 8 (lv_font_fmt_txt) .c source that you drop
straight into src/ and use with LV_FONT_DECLARE / lv_obj_set_style_text_font.

Usage
-----
# Rotate 90° CCW  (vertical label reads top-to-bottom when left-mounted)
python3 tools/rotate_lvgl8_font.py \\
    .pio/libdeps/nodemcu-32s/lvgl/src/font/lv_font_montserrat_14.c \\
    src/lv_font_montserrat_14_rot90ccw.c

# Rotate 90° CW  (vertical label reads bottom-to-top when left-mounted)
python3 tools/rotate_lvgl8_font.py \\
    .pio/libdeps/nodemcu-32s/lvgl/src/font/lv_font_montserrat_14.c \\
    src/lv_font_montserrat_14_rot90cw.c  --cw

# Override the output C variable name
python3 tools/rotate_lvgl8_font.py input.c output.c --name my_font_rot

Then add to your build-flags in platformio.ini:
    -D LV_FONT_MONTSERRAT_14=1      (if not already there)

And in your source:
    LV_FONT_DECLARE(lv_font_montserrat_14_rot90ccw);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14_rot90ccw, 0);
    lv_label_set_text(lbl, "S\\nC\\nO\\nR\\nE");  // one char per line

Bitmap format assumed
---------------------
bpp=4, no compression, NO per-row padding — the default produced by:
    lv_font_conv --bpp 4 --no-compress ...

In this format all pixels are packed continuously at 4 bits each (two per byte,
MSB = first pixel).  A glyph with box_w=W and box_h=H therefore needs exactly
ceil(W*H/2) bytes.  This is verified against the built-in Montserrat-14 font.

Metric transformation
---------------------
90° CCW (the default):
    new_box_w  = old box_h
    new_box_h  = old box_w
    new_ofs_x  = old ofs_y          (old bottom y → new left x)
    new_ofs_y  = −old ofs_x         (old left x  → new bottom y)
    new_adv_w  = (new_box_w + 2)×16

90° CW:
    new_box_w  = old box_h
    new_box_h  = old box_w
    new_ofs_x  = −old ofs_y − old box_h
    new_ofs_y  = old ofs_x
    new_adv_w  = (new_box_w + 2)×16

The font's line_height is set to max(new_box_h)+2 across all glyphs.
Tune the "+2" spacing by editing NEW_SPACING near the top of this file.
"""

import sys
import re
import math
import argparse
from typing import List, Tuple, Optional, Dict

# ── tuneable ──────────────────────────────────────────────────────────────────
NEW_SPACING = 2   # extra pixels added to adv_w and line_height after rotation
# ─────────────────────────────────────────────────────────────────────────────


# ═══════════════════════════════ Bitmap helpers ═══════════════════════════════

def _get4(data: bytes, idx: int) -> int:
    """Return the alpha nibble (0-15) at logical pixel position idx."""
    b = data[idx >> 1]
    return (b >> 4) & 0xF if (idx & 1) == 0 else b & 0xF


def _set4(buf: bytearray, idx: int, val: int) -> None:
    b = idx >> 1
    if (idx & 1) == 0:
        buf[b] = (buf[b] & 0x0F) | ((val & 0xF) << 4)
    else:
        buf[b] = (buf[b] & 0xF0) | (val & 0xF)


def rotate_ccw(raw: bytes, W: int, H: int) -> Tuple[bytes, int, int]:
    """90° CCW — new dimensions (H, W).  rotated[rx,ry] = orig[W-1-ry, rx]."""
    nW, nH = H, W
    out = bytearray((nW * nH + 1) >> 1)
    new_idx = 0
    for ry in range(nH):
        for rx in range(nW):
            ox, oy = W - 1 - ry, rx
            _set4(out, new_idx, _get4(raw, oy * W + ox))
            new_idx += 1
    return bytes(out), nW, nH


def rotate_cw(raw: bytes, W: int, H: int) -> Tuple[bytes, int, int]:
    """90° CW  — new dimensions (H, W).  rotated[rx,ry] = orig[ry, H-1-rx]."""
    nW, nH = H, W
    out = bytearray((nW * nH + 1) >> 1)
    new_idx = 0
    for ry in range(nH):
        for rx in range(nW):
            ox, oy = ry, H - 1 - rx
            _set4(out, new_idx, _get4(raw, oy * W + ox))
            new_idx += 1
    return bytes(out), nW, nH


# ═══════════════════════════════ C-source parser ══════════════════════════════

_HEX_RE  = re.compile(r'0[xX][0-9a-fA-F]+')
_GDSC_RE = re.compile(
    r'\{\.bitmap_index\s*=\s*(\d+)\s*,\s*'
    r'\.adv_w\s*=\s*(\d+)\s*,\s*'
    r'\.box_w\s*=\s*(\d+)\s*,\s*'
    r'\.box_h\s*=\s*(\d+)\s*,\s*'
    r'\.ofs_x\s*=\s*(-?\d+)\s*,\s*'
    r'\.ofs_y\s*=\s*(-?\d+)\s*\}'
)


def strip_comments(src: str) -> str:
    """Remove /* */ and // comments (preserving line positions)."""
    def _rep(m: re.Match) -> str:
        t = m.group(0)
        if t.startswith('/'):
            return re.sub(r'[^\n]', ' ', t)
        return t
    return re.sub(r'/\*.*?\*/|//[^\n]*', _rep, src, flags=re.DOTALL)


def parse_bitmap(clean: str) -> List[int]:
    m = re.search(
        r'const\s+uint8_t\s+glyph_bitmap\[\]\s*=\s*\{(.*?)\}\s*;',
        clean, re.DOTALL)
    if not m:
        raise ValueError("glyph_bitmap[] array not found")
    return [int(h, 16) for h in _HEX_RE.findall(m.group(1))]


def parse_glyph_dsc(clean: str) -> List[Tuple[int,int,int,int,int,int]]:
    m = re.search(r'glyph_dsc\[\]\s*=\s*\{(.*?)\}\s*;', clean, re.DOTALL)
    if not m:
        raise ValueError("glyph_dsc[] not found")
    return [tuple(int(g.group(i)) for i in range(1, 7))   # type: ignore[return-value]
            for g in _GDSC_RE.finditer(m.group(1))]


def parse_metrics(clean: str) -> Dict[str, int]:
    result: Dict[str, int] = {}
    for key in ('line_height', 'base_line'):
        mm = re.search(rf'\.{key}\s*=\s*(\d+)', clean)
        if mm:
            result[key] = int(mm.group(1))
    return result


def parse_font_name(src: str) -> str:
    m = re.search(r'\bconst\s+lv_font_t\s+(\w+)\s*=', src)
    return m.group(1) if m else "lv_font_rotated"


def parse_bpp(clean: str) -> int:
    m = re.search(r'\.bpp\s*=\s*(\d+)', clean)
    return int(m.group(1)) if m else 4


# ═══════════════════════════════ Main logic ═══════════════════════════════════

def rotate_font(src_path: str, dst_path: str,
                clockwise: bool, new_name: Optional[str]) -> None:

    with open(src_path, encoding='utf-8') as f:
        content = f.read()

    clean        = strip_comments(content)
    raw_bitmap   = parse_bitmap(clean)
    glyph_dscs   = parse_glyph_dsc(clean)
    metrics      = parse_metrics(clean)
    orig_name    = parse_font_name(content)   # use un-stripped for accurate match
    bpp          = parse_bpp(clean)

    if bpp != 4:
        print(f"WARNING: bpp={bpp} detected — only bpp=4 is supported. "
              "The output may be incorrect.", file=sys.stderr)

    rotate_fn = rotate_cw if clockwise else rotate_ccw
    direction  = "CW" if clockwise else "CCW"

    if new_name is None:
        suffix = "_rot90cw" if clockwise else "_rot90ccw"
        new_name = orig_name + suffix

    new_bitmap : bytearray = bytearray()
    new_dscs   : List[str] = []
    max_new_box_h = 0

    for i, glyph in enumerate(glyph_dscs):
        bi, adv_w, box_w, box_h, ofs_x, ofs_y = glyph

        if box_w == 0 or box_h == 0:
            # Empty glyph (space, reserved id 0, etc.) — keep geometry as-is
            new_dscs.append(
                f"    {{.bitmap_index = {len(new_bitmap)}, .adv_w = {adv_w}, "
                f".box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0}} /* id={i} */")
            continue

        # Extract glyph bytes (continuous packing: ceil(W*H/2) bytes)
        n_bytes   = (box_w * box_h + 1) >> 1
        glyph_raw = bytes(raw_bitmap[bi : bi + n_bytes])

        # Rotate bitmap
        rot_bytes, nW, nH = rotate_fn(glyph_raw, box_w, box_h)

        new_bi = len(new_bitmap)
        new_bitmap.extend(rot_bytes)

        # Adjust glyph metrics
        if clockwise:
            # CW: new_x = -old_y  →  new_ofs_x = -(ofs_y + box_h)
            #     new_y =  old_x  →  new_ofs_y =  ofs_x
            n_ofs_x = -(ofs_y + box_h)
            n_ofs_y =  ofs_x
        else:
            # CCW: new_x =  old_y  →  new_ofs_x = ofs_y
            #      new_y = -old_x  →  new_ofs_y = -ofs_x
            n_ofs_x =  ofs_y
            n_ofs_y = -ofs_x

        n_adv_w = (nW + NEW_SPACING) * 16
        max_new_box_h = max(max_new_box_h, nH)

        new_dscs.append(
            f"    {{.bitmap_index = {new_bi}, .adv_w = {n_adv_w}, "
            f".box_w = {nW}, .box_h = {nH}, "
            f".ofs_x = {n_ofs_x}, .ofs_y = {n_ofs_y}}} /* id={i} */")

    # ── Build bitmap C text (16 bytes per line) ───────────────────────────────
    bmap_rows = []
    for i in range(0, len(new_bitmap), 16):
        bmap_rows.append('    ' + ', '.join(f'0x{b:02x}' for b in new_bitmap[i:i+16]))
    bmap_body = ',\n'.join(bmap_rows)

    new_line_height = max_new_box_h + NEW_SPACING
    orig_base_line  = metrics.get('base_line', 2)
    new_base_line   = min(orig_base_line, new_line_height - 1)

    # ── Patch the C source ────────────────────────────────────────────────────

    out = content

    # 1. Replace glyph_bitmap[] array (handles all LV_ATTRIBUTE_* variants)
    out = re.sub(
        r'static\s+(?:LV_ATTRIBUTE_\w+\s+)*const\s+uint8_t\s+glyph_bitmap\[\]\s*=\s*\{.*?\}\s*;',
        (f"static LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST "
         f"const uint8_t glyph_bitmap[] = {{\n{bmap_body}\n}};"),
        out, flags=re.DOTALL)

    # 2. Replace glyph_dsc[] array
    dsc_body = ',\n'.join(new_dscs)
    out = re.sub(
        r'static\s+const\s+lv_font_fmt_txt_glyph_dsc_t\s+glyph_dsc\[\]\s*=\s*\{.*?\}\s*;',
        f"static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {{\n{dsc_body}\n}};",
        out, flags=re.DOTALL)

    # 3. Update line_height and base_line
    out = re.sub(r'(\.line_height\s*=\s*)\d+', rf'\g<1>{new_line_height}', out)
    out = re.sub(r'(\.base_line\s*=\s*)\d+',   rf'\g<1>{new_base_line}',   out)

    # 4. Rename the lv_font_t variable everywhere
    out = re.sub(re.escape(orig_name), new_name, out)

    # 5. Fix the include path: ../../lvgl.h is relative to LVGL's src/font/ dir
    #    and won't resolve from src/ in a PlatformIO project.
    #    Replace the deep-relative path with the simple "lvgl.h" which PlatformIO
    #    resolves correctly from any src file.
    out = out.replace('"../../lvgl.h"', '"lvgl.h"')
    out = out.replace('"lvgl/lvgl.h"',  '"lvgl.h"')

    # 7. Prepend a generation header
    src_basename = src_path.replace('\\', '/').split('/')[-1]
    header = (
        f"/* ---- Generated by rotate_lvgl8_font.py --------------------------\n"
        f" * Source    : {src_basename}\n"
        f" * Rotation  : 90° {direction}\n"
        f" * Original  : {orig_name}\n"
        f" *\n"
        f" * Example usage:\n"
        f" *   LV_FONT_DECLARE({new_name});\n"
        f" *   lv_obj_set_style_text_font(lbl, &{new_name}, 0);\n"
        f" *   lv_label_set_text(lbl, \"S\\nC\\nO\\nR\\nE\");\n"
        f" * ---------------------------------------------------------------- */\n"
    )
    # Insert header right after the original lv_font_conv comment (if present)
    m_head = re.match(r'(/\*.*?\*/[ \t]*\n)', out, re.DOTALL)
    if m_head:
        insert_pos = m_head.end()
        out = out[:insert_pos] + header + out[insert_pos:]
    else:
        out = header + out

    with open(dst_path, 'w', encoding='utf-8') as f:
        f.write(out)

    orig_bmap_bytes = len(raw_bitmap)
    new_bmap_bytes  = len(new_bitmap)

    print(f"Written : {dst_path}")
    print(f"Font name: {new_name}")
    print(f"Glyphs   : {len(glyph_dscs)}")
    print(f"Bitmap   : {new_bmap_bytes} bytes  (original: {orig_bmap_bytes} bytes)")
    print(f"line_height: {new_line_height}  (original: {metrics.get('line_height','?')})")
    print()
    print("── Next steps ────────────────────────────────────────────────────")
    print(f"  1. The file is already written to  {dst_path}")
    print(f"  2. In your .cpp / .h:")
    print(f"       LV_FONT_DECLARE({new_name});")
    print(f"       lv_obj_set_style_text_font(lbl, &{new_name}, 0);")
    print(f"       lv_label_set_text(lbl, \"S\\nC\\nO\\nR\\nE\");")
    print(f"  3. Build with platformio run")


# ═══════════════════════════════ CLI ═════════════════════════════════════════

def main() -> None:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input",  help="Input LVGL 8 font .c file")
    ap.add_argument("output", help="Output .c file")
    ap.add_argument("--cw",   action="store_true",
                    help="Rotate 90° CW instead of the default CCW")
    ap.add_argument("--name", default=None,
                    help="Override the C variable name in the output file")
    args = ap.parse_args()
    rotate_font(args.input, args.output, args.cw, args.name)


if __name__ == "__main__":
    main()
