/* test_kinematics_cartesian.c — Unit Tests for Cartesian Kinematics */
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "../../include/app/kinematics_cartesian.h"

#define TEST(name)  static void name(void)
#define RUN(name)   do { printf("  %-40s", #name); name(); printf("PASS\n"); } while(0)

#define ASSERT_EQ_INT(a,b)    assert((a) == (b))
#define ASSERT_NEAR(a,b,eps)  assert(fabsf((a)-(b)) < (eps))

/* ── Tests ──────────────────────────────────────────────────── */

TEST(test_cart_to_steps_basic)
{
    cart_pos_t pos = { .x = 10.0f, .y = 5.0f, .z = 1.0f };
    cart_steps_t out;
    ro_status_t rc = kinematics_cart_to_steps(&pos, 80.0f, 80.0f, 400.0f, &out);
    ASSERT_EQ_INT(RO_OK, rc);
    ASSERT_EQ_INT(800, out.sx);
    ASSERT_EQ_INT(400, out.sy);
    ASSERT_EQ_INT(400, out.sz);
}

TEST(test_cart_to_steps_negative)
{
    cart_pos_t pos = { .x = -10.0f, .y = 0.0f, .z = -2.5f };
    cart_steps_t out;
    kinematics_cart_to_steps(&pos, 80.0f, 80.0f, 400.0f, &out);
    ASSERT_EQ_INT(-800, out.sx);
    ASSERT_EQ_INT(0,    out.sy);
    ASSERT_EQ_INT(-1000, out.sz);
}

TEST(test_cart_to_steps_fractional)
{
    cart_pos_t pos = { .x = 0.5f, .y = 0.125f, .z = 0.0f };
    cart_steps_t out;
    kinematics_cart_to_steps(&pos, 80.0f, 80.0f, 400.0f, &out);
    ASSERT_EQ_INT(40, out.sx);   /* 0.5 * 80 = 40 */
    ASSERT_EQ_INT(10, out.sy);   /* 0.125 * 80 = 10 */
    ASSERT_EQ_INT(0,  out.sz);
}

TEST(test_steps_to_cart_roundtrip)
{
    cart_pos_t pos = { .x = 12.5f, .y = -7.25f, .z = 3.0f };
    cart_steps_t steps;
    kinematics_cart_to_steps(&pos, 80.0f, 80.0f, 400.0f, &steps);

    cart_pos_t back;
    kinematics_steps_to_cart(&steps, 80.0f, 80.0f, 400.0f, &back);
    ASSERT_NEAR(pos.x, back.x, 0.02f);
    ASSERT_NEAR(pos.y, back.y, 0.02f);
    ASSERT_NEAR(pos.z, back.z, 0.005f);
}

TEST(test_null_returns_einval)
{
    cart_steps_t out;
    ASSERT_EQ_INT(RO_EINVAL, kinematics_cart_to_steps(NULL, 80, 80, 400, &out));
    ASSERT_EQ_INT(RO_EINVAL, kinematics_cart_to_steps(NULL, 80, 80, 400, NULL));

    cart_pos_t pos2;
    cart_steps_t s = { 0, 0, 0 };
    ASSERT_EQ_INT(RO_EINVAL, kinematics_steps_to_cart(&s, 0.0f, 80, 400, &pos2));
}

TEST(test_zero_delta)
{
    cart_pos_t pos = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
    cart_steps_t out;
    kinematics_cart_to_steps(&pos, 80.0f, 80.0f, 400.0f, &out);
    ASSERT_EQ_INT(0, out.sx);
    ASSERT_EQ_INT(0, out.sy);
    ASSERT_EQ_INT(0, out.sz);
}

/* ── Runner ─────────────────────────────────────────────────── */

int main(void)
{
    printf("test_kinematics_cartesian:\n");

    RUN(test_cart_to_steps_basic);
    RUN(test_cart_to_steps_negative);
    RUN(test_cart_to_steps_fractional);
    RUN(test_steps_to_cart_roundtrip);
    RUN(test_null_returns_einval);
    RUN(test_zero_delta);

    printf("All kinematics_cartesian tests passed!\n");
    return 0;
}
