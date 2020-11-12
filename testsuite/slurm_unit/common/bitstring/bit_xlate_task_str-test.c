#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/common/bitstring.h"
#include "src/common/xmalloc.h"
#include "src/common/xstring.h"

START_TEST(null_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(array_task_str == NULL);
	ck_assert(array_bitmap == NULL);

	array_task_str = xstrdup("");
	array_bitmap = bit_alloc(1);
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(array_task_str[0] == '\0');
	ck_assert(array_bitmap == NULL);

	/* Test must have 0x at front */
	array_task_str = xstrdup("h");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "h"));
	ck_assert(array_bitmap == NULL);

	array_task_str = xstrdup("hello");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "hello"));
	ck_assert(array_bitmap == NULL);
}
END_TEST

START_TEST(good_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	array_task_str = xstrdup("0x7");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "0-2"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "0-2"));

	array_task_str = xstrdup("0x9C6");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "1-2,6-8,11"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "1-2,6-8,11"));

	array_task_str = xstrdup("0x9C6");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "1-2,6-8,11%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "1-2,6-8,11"));

	/* Max task count */
	array_task_str = xstrdup("0x9C6");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "1-2,6-8,11%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "1-2,6-8,11"));

	/* Stepped task */
	array_task_str = xstrdup("0x55554");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "2-18:2%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "2,4,6,8,10,12,14,16,18"));

	/* Broken up stepped task */
	array_task_str = xstrdup("0x45174");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "2,4-6,8,12,14,18%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "2,4-6,8,12,14,18"));

	/* test with array_bitmap -- frees task_bitmap */
	array_task_str = xstrdup("0x55154");
	bit_xlate_task_str(&array_task_str, 9, NULL);
	ck_assert(!xstrcmp(array_task_str, "2,4,6,8,12,14,16,18%9"));

}
END_TEST

START_TEST(BITSTR_LEN_no_max_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	/* SLURM_BITSTR_LEN */
	setenv("SLURM_BITSTR_LEN", "10", 1);
	array_task_str = xstrdup("0x55154");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "2,4,6,..."));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "2,4,6,8,12,14,16,18"));
}
END_TEST

START_TEST(BITSTR_LEN_with_max_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	/* SLURM_BITSTR_LEN */
	setenv("SLURM_BITSTR_LEN", "10", 1);
	array_task_str = xstrdup("0x55154");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "2,4,6,...%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "2,4,6,8,12,14,16,18"));
}
END_TEST

START_TEST(BITSTR_LEN_negative_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	setenv("SLURM_BITSTR_LEN", "-1", 1);
	array_task_str = xstrdup("0x5555555555155");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44..."));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50"));
}
END_TEST

START_TEST(BITSTR_LEN_negative_max_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	setenv("SLURM_BITSTR_LEN", "-1", 1);
	array_task_str = xstrdup("0x5555555555155");
	bit_xlate_task_str(&array_task_str, 9, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44...%9"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50"));
}
END_TEST

START_TEST(BITSTR_LEN_65_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	setenv("SLURM_BITSTR_LEN", "65", 1);
	array_task_str = xstrdup("0x5555555555155");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,..."));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50"));
}
END_TEST

START_TEST(BITSTR_LEN_0_test)
{
	void *array_bitmap = NULL;
	char *array_task_str = NULL;

	setenv("SLURM_BITSTR_LEN", "0", 1);
	array_task_str = xstrdup("0x5555555555155");
	bit_xlate_task_str(&array_task_str, 0, &array_bitmap);
	ck_assert(!xstrcmp(array_task_str, "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50"));
	ck_assert(!xstrcmp(bit_fmt_full(array_bitmap), "0,2,4,6,8,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50"));
}
END_TEST

/*****************************************************************************
 * TEST SUITE                                                                *
 ****************************************************************************/

Suite *suite(void)
{
	Suite *s = suite_create("bit_unfmt_hexmask test");
	TCase *tc_core = tcase_create("Testing bit_unfmt_hexmask");

	tcase_add_test(tc_core, null_test);
	tcase_add_test(tc_core, good_test);

	/* has to be run in forked mode because bitstr_len is a static */
	tcase_add_test(tc_core, BITSTR_LEN_no_max_test);
	tcase_add_test(tc_core, BITSTR_LEN_with_max_test);
	tcase_add_test(tc_core, BITSTR_LEN_negative_test);
	tcase_add_test(tc_core, BITSTR_LEN_negative_max_test);
	tcase_add_test(tc_core, BITSTR_LEN_65_test);
	tcase_add_test(tc_core, BITSTR_LEN_0_test);

	suite_add_tcase(s, tc_core);
	return s;
}

/*****************************************************************************
 * TEST RUNNER                                                               *
 ****************************************************************************/

int main(void)
{
	int number_failed;
	SRunner *sr = srunner_create(suite());

	//srunner_set_fork_status(sr, CK_NOFORK);

	srunner_run_all(sr, CK_VERBOSE);
	//srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
