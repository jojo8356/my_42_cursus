/*
** test_parse.c — Unit tests for FDF file parsing.
**
** Verifies:
**   • Basic 3x3 map parsing
**   • Single-value map
**   • Flat map (all zeros)
**   • Stair map (incrementing values)
**   • Peak map (symmetric heights)
**   • Min/max z computation
**   • Error handling (non-existent file, ragged grid)
**   • Memory cleanup
*/

# include "fdf.h"
# include "test_framework.h"
# include <string.h>

int main(void)
{
    TEST_BEGIN();

    t_map map;
    memset(&map, 0, sizeof(map));

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Basic 3x3 map
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Basic 3x3 map parsing");

    TEST_ASSERT_EQ(parse_file("tests/maps/basic.fdf", &map), 0);
    TEST_ASSERT_EQ(map.width, 3);
    TEST_ASSERT_EQ(map.height, 3);
    TEST_ASSERT_EQ(map.min_z, 0);
    TEST_ASSERT_EQ(map.max_z, 2);

    /* Check specific values. */
    TEST_ASSERT_EQ(map.grid[0].z, 0);  /* (0,0) */
    TEST_ASSERT_EQ(map.grid[1].z, 0);  /* (1,0) */
    TEST_ASSERT_EQ(map.grid[2].z, 0);  /* (2,0) */
    TEST_ASSERT_EQ(map.grid[3].z, 1);  /* (0,1) */
    TEST_ASSERT_EQ(map.grid[4].z, 2);  /* (1,1) */
    TEST_ASSERT_EQ(map.grid[5].z, 0);  /* (2,1) */
    TEST_ASSERT_EQ(map.grid[6].z, 0);  /* (0,2) */
    TEST_ASSERT_EQ(map.grid[7].z, 0);  /* (1,2) */
    TEST_ASSERT_EQ(map.grid[8].z, 1);  /* (2,2) */

    /* Check coordinates. */
    TEST_ASSERT_EQ(map.grid[0].x, 0);
    TEST_ASSERT_EQ(map.grid[0].y, 0);
    TEST_ASSERT_EQ(map.grid[4].x, 1);
    TEST_ASSERT_EQ(map.grid[4].y, 1);
    TEST_ASSERT_EQ(map.grid[8].x, 2);
    TEST_ASSERT_EQ(map.grid[8].y, 2);

    free_map(&map);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Single value map
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Single value map");

    TEST_ASSERT_EQ(parse_file("tests/maps/single.fdf", &map), 0);
    TEST_ASSERT_EQ(map.width, 1);
    TEST_ASSERT_EQ(map.height, 1);
    TEST_ASSERT_EQ(map.min_z, 5);
    TEST_ASSERT_EQ(map.max_z, 5);
    TEST_ASSERT_EQ(map.grid[0].z, 5);
    free_map(&map);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Flat map
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Flat map");

    TEST_ASSERT_EQ(parse_file("tests/maps/flat.fdf", &map), 0);
    TEST_ASSERT_EQ(map.width, 4);
    TEST_ASSERT_EQ(map.height, 3);
    TEST_ASSERT_EQ(map.min_z, 0);
    TEST_ASSERT_EQ(map.max_z, 0);

    for (int i = 0; i < map.width * map.height; i++)
        TEST_ASSERT_EQ(map.grid[i].z, 0);
    free_map(&map);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Stair map
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Stair map");

    TEST_ASSERT_EQ(parse_file("tests/maps/stair.fdf", &map), 0);
    TEST_ASSERT_EQ(map.width, 5);
    TEST_ASSERT_EQ(map.height, 5);
    TEST_ASSERT_EQ(map.min_z, 0);
    TEST_ASSERT_EQ(map.max_z, 8);

    /* Row 0: 0 1 2 3 4 */
    TEST_ASSERT_EQ(map.grid[0].z, 0);
    TEST_ASSERT_EQ(map.grid[1].z, 1);
    TEST_ASSERT_EQ(map.grid[2].z, 2);
    TEST_ASSERT_EQ(map.grid[3].z, 3);
    TEST_ASSERT_EQ(map.grid[4].z, 4);

    /* Row 4: 4 5 6 7 8 */
    TEST_ASSERT_EQ(map.grid[20].z, 4);
    TEST_ASSERT_EQ(map.grid[21].z, 5);
    TEST_ASSERT_EQ(map.grid[24].z, 8);
    free_map(&map);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Peak map
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Peak map");

    TEST_ASSERT_EQ(parse_file("tests/maps/peak.fdf", &map), 0);
    TEST_ASSERT_EQ(map.width, 5);
    TEST_ASSERT_EQ(map.height, 5);
    TEST_ASSERT_EQ(map.min_z, 0);
    TEST_ASSERT_EQ(map.max_z, 9);

    /* Center should be 9. */
    TEST_ASSERT_EQ(map.grid[12].z, 9);  /* (2,2) center */
    /* Corners should be 0. */
    TEST_ASSERT_EQ(map.grid[0].z, 0);
    TEST_ASSERT_EQ(map.grid[4].z, 0);
    TEST_ASSERT_EQ(map.grid[20].z, 0);
    TEST_ASSERT_EQ(map.grid[24].z, 0);
    free_map(&map);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Error handling
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Error handling");

    /* Non-existent file. */
    memset(&map, 0, sizeof(map));
    TEST_ASSERT_EQ(parse_file("tests/maps/nonexistent.fdf", &map), -1);

    /* Ragged grid. */
    memset(&map, 0, sizeof(map));
    TEST_ASSERT_EQ(parse_file("tests/maps/ragged.fdf", &map), -1);

    /* Empty file. */
    memset(&map, 0, sizeof(map));
    TEST_ASSERT_EQ(parse_file("tests/maps/empty.fdf", &map), -1);

    /* ═══════════════════════════════════════════════════════════
    ** SUITE: Memory cleanup
    ** ═══════════════════════════════════════════════════════════ */
    TEST_SUITE("Memory cleanup");

    memset(&map, 0, sizeof(map));
    TEST_ASSERT_EQ(parse_file("tests/maps/basic.fdf", &map), 0);
    free_map(&map);
    TEST_ASSERT_EQ(map.grid, NULL);
    TEST_ASSERT_EQ(map.width, 0);
    TEST_ASSERT_EQ(map.height, 0);

    return TEST_END();
}
