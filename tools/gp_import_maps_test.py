#!/usr/bin/env python3
"""Tests for the Tiled importer.

    python tools/gp_import_maps_test.py

Every case builds a .tmx in a temporary directory, so the tests never depend on the real maps and
a broken level_01 cannot mask a broken importer.
"""

import pathlib
import re
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import gp_import_maps as importer  # noqa: E402

LIMITS = {
    "max_characters": 16,
    "max_routes": 16,
    "max_route_points": 8,
    "max_columns": 32,
    "max_rows": 32,
    "player_signature": (2, 1, "caller"),
}

# What a character has to be to match the player. Only the last level holds one.
PLAYER_ANSWER = {"clan": 2, "family": 1, "role": "responder"}


def build_map(columns=6, rows=5, walls=(), characters=(), routes=(), player=(1, 1),
              include_collision=True, encoding="csv"):
    """A bordered room, plus whatever the caller asks for."""
    solid = set(walls)

    for x in range(columns):
        solid.add((x, 0))
        solid.add((x, rows - 1))

    for y in range(rows):
        solid.add((0, y))
        solid.add((columns - 1, y))

    cells = [[2 if (x, y) in solid else 0 for x in range(columns)] for y in range(rows)]
    csv = ",\n".join(",".join(str(value) for value in row) for row in cells)

    layers = ""

    if include_collision:
        layers = (f'<layer id="1" name="collision" width="{columns}" height="{rows}">'
                  f'<data encoding="{encoding}">\n{csv}\n</data></layer>')

    entities = [f'<object id="1" name="player" x="{player[0] * 16}" y="{player[1] * 16}" '
                'width="16" height="16"/>']
    identifier = 2

    for spec in characters:
        props = "".join([
            f'<property name="clan" type="int" value="{spec["clan"]}"/>' if "clan" in spec else "",
            f'<property name="family" type="int" value="{spec["family"]}"/>' if "family" in spec else "",
            f'<property name="role" value="{spec["role"]}"/>' if "role" in spec else "",
            f'<property name="is_blocker" type="bool" value="{str(spec.get("blocker", False)).lower()}"/>',
            f'<property name="flee_route" value="{spec["route"]}"/>' if spec.get("route") else "",
        ])
        entities.append(
            f'<object id="{identifier}" name="{spec["name"]}" x="{spec["tile"][0] * 16}" '
            f'y="{spec["tile"][1] * 16}" width="16" height="16"><properties>{props}</properties></object>')
        identifier += 1

    zones = []

    for name, origin, offsets in routes:
        points = " ".join(f"{ox},{oy}" for ox, oy in offsets)
        zones.append(f'<object id="{identifier}" name="{name}" x="{origin[0] * 16}" '
                     f'y="{origin[1] * 16}"><polyline points="{points}"/></object>')
        identifier += 1

    return (f'<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<map version="1.10" orientation="orthogonal" renderorder="right-down" '
            f'width="{columns}" height="{rows}" tilewidth="16" tileheight="16" infinite="0">'
            f'{layers}'
            f'<objectgroup id="2" name="entities">{"".join(entities)}</objectgroup>'
            f'<objectgroup id="3" name="zones">{"".join(zones)}</objectgroup>'
            f'</map>\n')


def pair(first_tile, second_tile, clan=0, family=0, route=None):
    return [
        {"name": "a", "tile": first_tile, "clan": clan, "family": family, "role": "caller",
         "route": route},
        {"name": "b", "tile": second_tile, "clan": clan, "family": family, "role": "responder"},
    ]


class ImporterTest(unittest.TestCase):

    def parse(self, text):
        with tempfile.TemporaryDirectory() as folder:
            path = pathlib.Path(folder) / "fixture.tmx"
            path.write_text(text, encoding="utf-8")
            return importer.parse_map(path, LIMITS)

    def refuse(self, text, fragment):
        with self.assertRaises(importer.MapError) as caught:
            self.parse(text)

        self.assertIn(fragment, str(caught.exception))

    # Accepting good maps

    def test_a_bordered_room_with_one_pair_parses(self):
        level = self.parse(build_map(characters=pair((2, 2), (3, 3))))
        self.assertEqual(level["columns"], 6)
        self.assertEqual(level["rows"], 5)
        self.assertEqual(level["player"], (1, 1))
        self.assertEqual(len(level["characters"]), 2)

    def test_character_properties_survive_the_trip(self):
        level = self.parse(build_map(characters=[
            {"name": "door", "tile": (2, 2), "clan": 1, "family": 3, "role": "caller",
             "blocker": True},
            {"name": "other", "tile": (3, 2), "clan": 1, "family": 3, "role": "responder"},
        ]))
        door = level["characters"][0]
        self.assertEqual(door["clan"], 1)
        self.assertEqual(door["family"], 3)
        self.assertEqual(door["role"], "caller")
        self.assertTrue(door["is_blocker"])
        self.assertFalse(level["characters"][1]["is_blocker"])

    def test_a_character_with_no_route_gets_the_sentinel(self):
        level = self.parse(build_map(characters=pair((2, 2), (3, 3))))
        self.assertEqual(level["characters"][0]["flee_route"], -1)

    def test_a_named_polyline_becomes_a_route_index(self):
        level = self.parse(build_map(
            characters=pair((2, 2), (3, 3), route="away"),
            routes=[("away", (2, 2), [(0, 0), (16, 0)])]))
        self.assertEqual(level["characters"][0]["flee_route"], 0)
        self.assertEqual(level["routes"][0][1], [(2, 2), (3, 2)])

    def test_object_positions_convert_to_tiles(self):
        level = self.parse(build_map(columns=8, rows=8, characters=pair((5, 6), (4, 6))))
        self.assertEqual(level["characters"][0]["tile"], (5, 6))

    # Refusing bad maps

    def test_a_map_with_no_collision_layer_is_refused(self):
        self.refuse(build_map(characters=pair((2, 2), (3, 3)), include_collision=False),
                    "no tile layer named 'collision'")

    def test_a_non_csv_layer_is_refused_with_the_fix(self):
        self.refuse(build_map(characters=pair((2, 2), (3, 3)), encoding="base64"), "CSV")

    def test_a_map_with_no_player_is_refused(self):
        text = re.sub(r'<object id="1" name="player"[^>]*/>', "",
                      build_map(characters=pair((2, 2), (3, 3))))
        self.refuse(text, "no object named 'player'")

    def test_two_player_objects_are_refused(self):
        text = build_map(characters=pair((2, 2), (3, 3))).replace(
            '<objectgroup id="2" name="entities">',
            '<objectgroup id="2" name="entities">'
            '<object id="99" name="player" x="48" y="48" width="16" height="16"/>')
        self.refuse(text, "more than one object named 'player'")

    def test_an_odd_character_count_is_refused(self):
        self.refuse(build_map(characters=[
            {"name": "a", "tile": (2, 2), "clan": 0, "family": 0, "role": "caller"}]),
            "can never pair off")

    def test_the_players_own_match_is_left_out_of_the_parity_rule(self):
        # Three characters: one pair, plus the one the level holds for the player.
        level = self.parse(build_map(characters=pair((2, 2), (3, 3)) + [
            dict(PLAYER_ANSWER, name="yours", tile=(4, 2))]))
        self.assertEqual(len(level["characters"]), 3)

    def test_two_characters_matching_the_player_are_refused(self):
        self.refuse(build_map(characters=[
            dict(PLAYER_ANSWER, name="one", tile=(2, 2)),
            dict(PLAYER_ANSWER, name="two", tile=(3, 2))]),
            "A level holds at most one")

    def test_an_odd_count_alongside_the_players_match_is_refused(self):
        self.refuse(build_map(characters=[
            {"name": "a", "tile": (2, 2), "clan": 0, "family": 0, "role": "caller"},
            dict(PLAYER_ANSWER, name="yours", tile=(3, 2))]),
            "can never pair off")

    def test_a_character_standing_in_a_wall_is_refused(self):
        self.refuse(build_map(characters=pair((0, 2), (3, 3))), "stands inside a wall")

    def test_a_player_standing_in_a_wall_is_refused(self):
        self.refuse(build_map(characters=pair((2, 2), (3, 3)), player=(0, 0)),
                    "player starts inside a wall")

    def test_two_characters_on_one_tile_are_refused(self):
        self.refuse(build_map(characters=pair((2, 2), (2, 2))), "both stand on")

    def test_an_unknown_role_is_refused(self):
        self.refuse(build_map(characters=[
            {"name": "a", "tile": (2, 2), "clan": 0, "family": 0, "role": "shouter"},
            {"name": "b", "tile": (3, 2), "clan": 0, "family": 0, "role": "responder"}]),
            "expected one of")

    def test_a_missing_property_is_refused_by_name(self):
        self.refuse(build_map(characters=[
            {"name": "a", "tile": (2, 2), "clan": 0, "role": "caller"},
            {"name": "b", "tile": (3, 2), "clan": 0, "family": 0, "role": "responder"}]),
            "has no 'family' property")

    def test_a_flee_route_that_does_not_exist_is_refused(self):
        self.refuse(build_map(characters=pair((2, 2), (3, 3), route="nowhere")), "which is not a polyline")

    def test_a_route_through_a_wall_is_refused(self):
        self.refuse(build_map(
            characters=pair((2, 2), (3, 3), route="away"),
            routes=[("away", (2, 2), [(0, 0), (-32, 0)])]),
            "passes through a solid tile")

    def test_a_map_larger_than_the_background_is_refused(self):
        self.refuse(build_map(columns=40, rows=8, characters=pair((2, 2), (3, 3))), "larger than")

    def test_a_route_with_too_many_points_is_refused(self):
        offsets = [(index * 16, 0) for index in range(LIMITS["max_route_points"] + 1)]
        self.refuse(build_map(
            columns=20, rows=5,
            characters=pair((2, 2), (3, 3), route="away"),
            routes=[("away", (2, 2), offsets)]),
            "the limit is")

    # Packing and output

    def test_collision_packs_least_significant_bit_first_row_major(self):
        level = self.parse(build_map(columns=8, rows=8, characters=pair((2, 2), (3, 3))))
        packed = importer.pack_collision(level)
        self.assertEqual(len(packed), 8)
        self.assertEqual(packed[0], 0xFF)          # the whole top row is border
        self.assertEqual(packed[1] & 0x01, 0x01)   # x=0 of row 1 is border
        self.assertEqual(packed[1] & 0x02, 0x00)   # x=1 of row 1 is open

    def test_the_generated_source_names_every_level(self):
        level = self.parse(build_map(characters=pair((2, 2), (3, 3))))
        header, source = importer.render([level])
        self.assertIn("extern const level_spec levels[]", header)
        self.assertIn("const int level_count = 1;", source)
        self.assertIn("fixture_collision", source)
        self.assertIn("role::caller", source)

    def test_no_maps_still_produces_compilable_source(self):
        header, source = importer.render([])
        self.assertIn("const level_spec levels[1] = {};", source)
        self.assertIn("const int level_count = 0;", source)
        self.assertIn("extern const int level_count", header)

    def test_rewriting_identical_output_is_skipped(self):
        with tempfile.TemporaryDirectory() as folder:
            path = pathlib.Path(folder) / "out.h"
            self.assertTrue(importer.write_if_changed(path, "one"))
            self.assertFalse(importer.write_if_changed(path, "one"))
            self.assertTrue(importer.write_if_changed(path, "two"))

    def test_limits_are_read_from_the_real_header(self):
        limits = importer.read_limits(pathlib.Path(__file__).resolve().parent.parent /
                                      "include" / "gp_level_data.h")
        self.assertEqual(limits["max_characters"], 16)
        self.assertEqual(limits["max_route_points"], 8)
        self.assertEqual(limits["max_columns"], 32)
        self.assertEqual(limits["player_signature"], (2, 1, "caller"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
