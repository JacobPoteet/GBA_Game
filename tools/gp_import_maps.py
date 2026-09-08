#!/usr/bin/env python3
"""Convert Tiled maps into the level data the game and the tests both read.

    python tools/gp_import_maps.py --maps maps --out generated

Writes <out>/include/gp_levels_generated.h and <out>/src/gp_levels_generated.cpp. The output has no
Butano types in it, so the ROM build and the host test build read exactly the same levels.

Two callers, one script:

  * the ROM build, through Butano's EXTTOOL hook, with --out=generated
  * the host tests, through tests/CMakeLists.txt, with --out=<cmake binary dir>/generated

The limits are read out of include/gp_level_data.h rather than repeated here, so a map cannot
quietly outgrow the array it will be loaded into.

Expected layers:

  collision   tile layer. Any non-zero cell is solid
  ground      tile layer. Parsed for size only until phase 3 gives it real art
  entities    object layer. One object named "player", the rest are characters
  zones       object layer. Polylines named by the flee_route property that points at them
"""

import argparse
import pathlib
import re
import sys
import xml.etree.ElementTree as ElementTree

ROLES = ("caller", "responder")


class MapError(Exception):
    """A map that cannot be imported. The message names the file and the problem."""


def read_limits(header_path):
    """Pull the array bounds out of gp_level_data.h so the two cannot drift apart."""
    try:
        text = header_path.read_text(encoding="utf-8")
    except OSError as error:
        raise MapError(f"cannot read {header_path}: {error}") from error

    wanted = ("max_characters", "max_routes", "max_route_points")
    limits = {}

    for name in wanted:
        match = re.search(rf"constexpr\s+int\s+{name}\s*=\s*(\d+)\s*;", text)

        if not match:
            raise MapError(f"{header_path}: could not find 'constexpr int {name}'")

        limits[name] = int(match.group(1))

    signature_header = header_path.parent / "gp_signature.h"
    signature_text = signature_header.read_text(encoding="utf-8") if signature_header.is_file() else ""
    match = re.search(r"constexpr\s+signature\s+player_signature\s*\{\s*(\d+)\s*,\s*(\d+)\s*,"
                      r"\s*role::(\w+)\s*\}", signature_text)

    if not match:
        raise MapError(f"{signature_header}: could not find 'constexpr signature player_signature'")

    limits["player_signature"] = (int(match.group(1)), int(match.group(2)), match.group(3))

    grid = header_path.parent / "gp_collision_map.h"
    grid_text = grid.read_text(encoding="utf-8") if grid.is_file() else ""

    for name in ("max_columns", "max_rows"):
        match = re.search(rf"constexpr\s+int\s+{name}\s*=\s*(\d+)\s*;", grid_text)

        if not match:
            raise MapError(f"{grid}: could not find 'constexpr int {name}'")

        limits[name] = int(match.group(1))

    return limits


def object_tile(element, tile_width, tile_height, source):
    """Tile coordinates of a Tiled object. Rectangles and points both anchor at x,y."""
    try:
        x = float(element.get("x", "0"))
        y = float(element.get("y", "0"))
    except ValueError as error:
        raise MapError(f"{source}: object {element.get('id')} has a non-numeric position") from error

    return (int(x // tile_width), int(y // tile_height))


def object_properties(element):
    properties = {}

    for holder in element.findall("properties"):
        for prop in holder.findall("property"):
            name = prop.get("name")
            kind = prop.get("type", "string")
            raw = prop.get("value")

            if raw is None:
                raw = (prop.text or "").strip()

            if kind == "int":
                properties[name] = int(raw)
            elif kind == "bool":
                properties[name] = raw == "true"
            else:
                properties[name] = raw

    return properties


def parse_tile_layer(layer, columns, rows, source):
    data = layer.find("data")

    if data is None or (data.get("encoding") or "") != "csv":
        raise MapError(f"{source}: layer '{layer.get('name')}' must use CSV encoding. "
                       "Set Map > Map Properties > Tile Layer Format to CSV in Tiled")

    values = [int(piece) for piece in (data.text or "").replace("\n", "").split(",") if piece.strip()]

    if len(values) != columns * rows:
        raise MapError(f"{source}: layer '{layer.get('name')}' has {len(values)} cells, "
                       f"expected {columns * rows}")

    return values


def parse_map(path, limits):
    try:
        root = ElementTree.parse(path).getroot()
    except ElementTree.ParseError as error:
        raise MapError(f"{path}: not valid XML: {error}") from error

    source = path.name
    columns = int(root.get("width", "0"))
    rows = int(root.get("height", "0"))
    tile_width = int(root.get("tilewidth", "16"))
    tile_height = int(root.get("tileheight", "16"))

    if columns > limits["max_columns"] or rows > limits["max_rows"]:
        raise MapError(f"{source}: {columns}x{rows} tiles is larger than the "
                       f"{limits['max_columns']}x{limits['max_rows']} a Butano regular background "
                       "holds without big_map")

    layers = {layer.get("name"): layer for layer in root.findall("layer")}
    groups = {group.get("name"): group for group in root.findall("objectgroup")}

    if "collision" not in layers:
        raise MapError(f"{source}: no tile layer named 'collision'")

    collision = parse_tile_layer(layers["collision"], columns, rows, source)
    solid = [value != 0 for value in collision]

    # Routes first, so a character can name one.
    routes = {}
    order = []

    for group_name in ("zones",):
        for element in groups.get(group_name, []):
            polyline = element.find("polyline")

            if polyline is None:
                continue

            name = element.get("name")

            if not name:
                raise MapError(f"{source}: a polyline on '{group_name}' has no name, "
                               "so nothing can point at it")

            if name in routes:
                raise MapError(f"{source}: two polylines are named '{name}'")

            origin_x = float(element.get("x", "0"))
            origin_y = float(element.get("y", "0"))
            points = []

            for piece in (polyline.get("points") or "").split():
                offset_x, offset_y = piece.split(",")
                points.append((int((origin_x + float(offset_x)) // tile_width),
                               int((origin_y + float(offset_y)) // tile_height)))

            if not points:
                raise MapError(f"{source}: route '{name}' has no points")

            if len(points) > limits["max_route_points"]:
                raise MapError(f"{source}: route '{name}' has {len(points)} points, "
                               f"the limit is {limits['max_route_points']}")

            for x, y in points:
                if not (0 <= x < columns and 0 <= y < rows) or solid[y * columns + x]:
                    raise MapError(f"{source}: route '{name}' passes through a solid tile at {x},{y}")

            routes[name] = len(order)
            order.append((name, points))

    # Entities.
    player = None
    characters = []

    for element in groups.get("entities", []):
        name = element.get("name") or f"object {element.get('id')}"
        tile = object_tile(element, tile_width, tile_height, source)

        if name == "player":
            if player is not None:
                raise MapError(f"{source}: more than one object named 'player'")

            player = tile
            continue

        properties = object_properties(element)

        for required in ("clan", "family", "role"):
            if required not in properties:
                raise MapError(f"{source}: character '{name}' has no '{required}' property")

        role = properties["role"]

        if role not in ROLES:
            raise MapError(f"{source}: character '{name}' has role '{role}', expected one of {ROLES}")

        route_name = properties.get("flee_route", "")
        route_index = -1

        if route_name:
            if route_name not in routes:
                raise MapError(f"{source}: character '{name}' names flee route '{route_name}', "
                               "which is not a polyline on 'zones'")

            route_index = routes[route_name]

        x, y = tile

        if not (0 <= x < columns and 0 <= y < rows):
            raise MapError(f"{source}: character '{name}' sits outside the map at {x},{y}")

        if solid[y * columns + x]:
            raise MapError(f"{source}: character '{name}' stands inside a wall at {x},{y}")

        characters.append({
            "name": name,
            "tile": tile,
            "clan": properties["clan"],
            "family": properties["family"],
            "role": role,
            "is_blocker": bool(properties.get("is_blocker", False)),
            "flee_route": route_index,
        })

    if player is None:
        raise MapError(f"{source}: no object named 'player' on the 'entities' layer")

    px, py = player

    if not (0 <= px < columns and 0 <= py < rows) or solid[py * columns + px]:
        raise MapError(f"{source}: the player starts inside a wall at {px},{py}")

    if len(characters) > limits["max_characters"]:
        raise MapError(f"{source}: {len(characters)} characters, the limit is {limits['max_characters']}")

    # The player's own match is not a puzzle piece, so they are left out of the parity rule.
    player_clan, player_family, player_role = limits["player_signature"]
    player_answer = "responder" if player_role == "caller" else "caller"
    held_for_player = [character["name"] for character in characters
                       if character["clan"] == player_clan and character["family"] == player_family
                       and character["role"] == player_answer]

    if len(held_for_player) > 1:
        raise MapError(f"{source}: {len(held_for_player)} characters match the player "
                       f"({', '.join(held_for_player)}). A level holds at most one")

    pairable = len(characters) - len(held_for_player)

    if pairable % 2 != 0:
        raise MapError(f"{source}: {pairable} characters to pair off. "
                       "An odd count can never pair off")

    if len(order) > limits["max_routes"]:
        raise MapError(f"{source}: {len(order)} routes, the limit is {limits['max_routes']}")

    seen = {}

    for character in characters:
        key = character["tile"]

        if key in seen:
            raise MapError(f"{source}: '{character['name']}' and '{seen[key]}' both stand on "
                           f"{key[0]},{key[1]}")

        seen[key] = character["name"]

    return {
        "name": path.stem,
        "columns": columns,
        "rows": rows,
        "solid": solid,
        "player": player,
        "characters": characters,
        "routes": order,
    }


def pack_collision(level):
    """One bit per tile, least significant bit first, row major. Matches gp::collision_map."""
    columns, rows = level["columns"], level["rows"]
    packed = bytearray((columns * rows + 7) // 8)

    for index, value in enumerate(level["solid"]):
        if value:
            packed[index // 8] |= 1 << (index % 8)

    return bytes(packed)


def render(levels):
    header = """\
/*
 * Generated by tools/gp_import_maps.py. Do not edit.
 *
 * Edit the .tmx files in maps/ instead, then rebuild.
 */

#ifndef GP_LEVELS_GENERATED_H
#define GP_LEVELS_GENERATED_H

#include "gp_level_data.h"

namespace gp
{

extern const level_spec levels[];
extern const int level_count;

} // namespace gp

#endif
"""

    lines = ["""\
/*
 * Generated by tools/gp_import_maps.py. Do not edit.
 *
 * Edit the .tmx files in maps/ instead, then rebuild.
 */

#include "gp_levels_generated.h"

namespace gp
{

namespace
{
"""]

    for level in levels:
        name = level["name"]
        packed = pack_collision(level)
        rows = [packed[at:at + 12] for at in range(0, len(packed), 12)]
        body = ",\n    ".join(", ".join(f"0x{byte:02X}" for byte in row) for row in rows)
        lines.append(f"const uint8_t {name}_collision[] = {{\n    {body}\n}};\n")

        if level["characters"]:
            entries = []

            for character in level["characters"]:
                x, y = character["tile"]
                entries.append(
                    f"    {{{{{x}, {y}}}, {{{character['clan']}, {character['family']}, "
                    f"role::{character['role']}}}, {str(character['is_blocker']).lower()}, "
                    f"{character['flee_route']}}},  // {character['name']}")

            joined = "\n".join(entries)
            lines.append(f"const character_spec {name}_characters[] = {{\n{joined}\n}};\n")

        if level["routes"]:
            entries = []

            for route_name, points in level["routes"]:
                joined_points = ", ".join(f"{{{x}, {y}}}" for x, y in points)
                entries.append(f"    {{{{{joined_points}}}, {len(points)}}},  // {route_name}")

            joined = "\n".join(entries)
            lines.append(f"const route {name}_routes[] = {{\n{joined}\n}};\n")

    lines.append("} // namespace\n")

    if levels:
        entries = []

        for level in levels:
            name = level["name"]
            px, py = level["player"]
            characters = f"{name}_characters" if level["characters"] else "nullptr"
            routes = f"{name}_routes" if level["routes"] else "nullptr"
            entries.append(
                f'    {{"{name}", {level["columns"]}, {level["rows"]}, {name}_collision, '
                f'{len(pack_collision(level))}, {{{px}, {py}}}, {characters}, '
                f'{len(level["characters"])}, {routes}, {len(level["routes"])}}},')

        joined = "\n".join(entries)
        lines.append(f"const level_spec levels[] = {{\n{joined}\n}};\n")
        lines.append(f"const int level_count = {len(levels)};\n")
    else:
        # A zero length array is not valid C++, so ship one blank entry and a count of zero.
        lines.append("const level_spec levels[1] = {};\n")
        lines.append("const int level_count = 0;\n")

    lines.append("} // namespace gp\n")
    return header, "\n".join(lines)


def write_if_changed(path, text):
    """Leaves the mtime alone when nothing changed, so make and cmake do not rebuild the world."""
    path.parent.mkdir(parents=True, exist_ok=True)

    if path.is_file() and path.read_text(encoding="utf-8") == text:
        return False

    path.write_text(text, encoding="utf-8")
    return True


def main():
    parser = argparse.ArgumentParser(description="Convert Tiled maps into generated level data.")
    parser.add_argument("--maps", default="maps", help="directory holding .tmx files")
    parser.add_argument("--out", required=True, help="directory to write include/ and src/ into")
    parser.add_argument("--include", default="include", help="directory holding gp_level_data.h")
    args = parser.parse_args()

    maps_dir = pathlib.Path(args.maps)
    out_dir = pathlib.Path(args.out)

    try:
        limits = read_limits(pathlib.Path(args.include) / "gp_level_data.h")
        levels = [parse_map(path, limits) for path in sorted(maps_dir.glob("*.tmx"))]
    except MapError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    header, source = render(levels)
    wrote = write_if_changed(out_dir / "include" / "gp_levels_generated.h", header)
    wrote |= write_if_changed(out_dir / "src" / "gp_levels_generated.cpp", source)

    total = sum(len(level["characters"]) for level in levels)
    state = "updated" if wrote else "unchanged"
    print(f"gp_import_maps: {len(levels)} level(s), {total} character(s), {state} in {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
