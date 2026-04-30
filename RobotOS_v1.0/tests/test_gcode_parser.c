/* test_gcode_parser.c — Unit Tests for G-code Parser */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Include header directly by relative path for host build */
#include "../../include/app/gcode_parser.h"

#define TEST(name)  static void name(void)
#define RUN(name)   do { printf("  %-40s", #name); name(); printf("PASS\n"); } while(0)

#define ASSERT_EQ_INT(a,b)    assert((a) == (b))
#define ASSERT_EQ_FLOAT(a,b)  assert(fabsf((a)-(b)) < 0.001f)
#define ASSERT_TRUE(a)        assert((a))
#define ASSERT_FALSE(a)       assert(!(a))

/* ── Tests ──────────────────────────────────────────────────── */

TEST(test_empty_line)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("", &cmd));
    ASSERT_EQ_INT(GCODE_NONE, cmd.type);
}

TEST(test_comment_line)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("; this is a comment", &cmd));
    ASSERT_EQ_INT(GCODE_NONE, cmd.type);
}

TEST(test_g0_rapid)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G0 X10 Y20 Z5", &cmd));
    ASSERT_EQ_INT(GCODE_MOVE_RAPID, cmd.type);
    ASSERT_TRUE(cmd.has_x);
    ASSERT_TRUE(cmd.has_y);
    ASSERT_TRUE(cmd.has_z);
    ASSERT_EQ_FLOAT(10.0f, cmd.x);
    ASSERT_EQ_FLOAT(20.0f, cmd.y);
    ASSERT_EQ_FLOAT(5.0f, cmd.z);
}

TEST(test_g1_linear_with_feedrate)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G1 X100.5 Y-50.3 F600", &cmd));
    ASSERT_EQ_INT(GCODE_MOVE_LINEAR, cmd.type);
    ASSERT_TRUE(cmd.has_x);
    ASSERT_TRUE(cmd.has_y);
    ASSERT_FALSE(cmd.has_z);
    ASSERT_TRUE(cmd.has_f);
    ASSERT_EQ_FLOAT(100.5f, cmd.x);
    ASSERT_EQ_FLOAT(-50.3f, cmd.y);
    ASSERT_EQ_FLOAT(600.0f, cmd.f);
}

TEST(test_g28_home)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G28", &cmd));
    ASSERT_EQ_INT(GCODE_HOME, cmd.type);
}

TEST(test_g90_g91_mode)
{
    gcode_cmd_t cmd;
    gcode_parser_reset();
    ASSERT_TRUE(gcode_parser_is_absolute());

    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G91", &cmd));
    ASSERT_EQ_INT(GCODE_SET_REL, cmd.type);
    ASSERT_FALSE(gcode_parser_is_absolute());

    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G90", &cmd));
    ASSERT_EQ_INT(GCODE_SET_ABS, cmd.type);
    ASSERT_TRUE(gcode_parser_is_absolute());
}

TEST(test_m3_spindle_on)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("M3 S12000", &cmd));
    ASSERT_EQ_INT(GCODE_SPINDLE_ON, cmd.type);
    ASSERT_TRUE(cmd.has_s);
    ASSERT_EQ_FLOAT(12000.0f, cmd.s);
}

TEST(test_m5_spindle_off)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("M5", &cmd));
    ASSERT_EQ_INT(GCODE_SPINDLE_OFF, cmd.type);
}

TEST(test_m104_extruder)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("M104 S200", &cmd));
    ASSERT_EQ_INT(GCODE_EXTRUDER_TEMP, cmd.type);
    ASSERT_TRUE(cmd.has_s);
    ASSERT_EQ_FLOAT(200.0f, cmd.s);
}

TEST(test_invalid_command)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_EINVAL, gcode_parse_line("G999", &cmd));
}

TEST(test_partial_params)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G1 X50 F300", &cmd));
    ASSERT_TRUE(cmd.has_x);
    ASSERT_FALSE(cmd.has_y);
    ASSERT_FALSE(cmd.has_z);
    ASSERT_TRUE(cmd.has_f);
}

TEST(test_inline_comment)
{
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G0 X10 ; rapid move", &cmd));
    ASSERT_EQ_INT(GCODE_MOVE_RAPID, cmd.type);
    ASSERT_EQ_FLOAT(10.0f, cmd.x);
    ASSERT_FALSE(cmd.has_y);
}

/* ── Runner ─────────────────────────────────────────────────── */

int main(void)
{
    printf("test_gcode_parser:\n");
    gcode_parser_reset();

    RUN(test_empty_line);
    RUN(test_comment_line);
    RUN(test_g0_rapid);
    RUN(test_g1_linear_with_feedrate);
    RUN(test_g28_home);
    RUN(test_g90_g91_mode);
    RUN(test_m3_spindle_on);
    RUN(test_m5_spindle_off);
    RUN(test_m104_extruder);
    RUN(test_invalid_command);
    RUN(test_partial_params);
    RUN(test_inline_comment);

    printf("All gcode_parser tests passed!\n");
    return 0;
}
