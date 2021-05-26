#include "map.h"
#include <assert.h>

gen_map(int_str, int, char *, ds_cmp_num_lt, DSDefault_shallowCopy, DSDefault_shallowDelete, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_map(strv_int, char *, int, ds_cmp_str_lt, DSDefault_deepCopyStr, DSDefault_deepDelete, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_map(strp_int, char *, int, ds_cmp_str_lt, DSDefault_shallowCopy, DSDefault_shallowDelete, DSDefault_shallowCopy, DSDefault_shallowDelete)
gen_map(nested, char *, Map_strv_int *, ds_cmp_str_lt, DSDefault_deepCopyStr, DSDefault_deepDelete, DSDefault_shallowCopy, __avltree_free_strv_int)

int ints_rand[] = {200,25,220,120,5,205,50,15,60,235,10,70,130,105,185,225,90,30,155,100,150,0,95,170,190,
125,210,75,45,160,175,145,55,230,35,65,110,140,115,20,215,85,195,240,245,135,80,180,40,165};
int ints[] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,
130,135,140,145,150,155,160,165,170,175,180,185,190,195,200,205,210,215,220,225,230,235,240,245};
char *strs_rand[] = {"200","025","220","120","005","205","050","015","060","235","010","070","130","105",
"185","225","090","030","155","100","150","000","095","170","190","125","210","075","045","160","175",
"145","055","230","035","065","110","140","115","020","215","085","195","240","245","135","080","180",
"040","165"};
char *strs[] = {"000","005","010","015","020","025","030","035","040","045","050","055","060",
"065","070","075","080","085","090","095","100","105","110","115","120","125","130","135","140","145",
"150","155","160","165","170","175","180","185","190","195","200","205","210","215","220","225","230",
"235","240","245"};

void compare_int_str(Map_int_str *m, int *keys, char **values, int size) {
    assert(map_size(m) == size);
    int i = 0;
    MapEntry_int_str *it;
    map_iter(int_str, m, it) {
        assert(it->data.first == keys[i]);
        assert(streq(it->data.second, values[i++]));
    }
    assert(i == size);
    i = size - 1;
    map_riter(int_str, m, it) {
        assert(it->data.first == keys[i]);
        assert(streq(it->data.second, values[i--]));
    }
    assert(i == -1);
}

void compare_str_int(Map_strv_int *m, char **keys, int *values, int size) {
    assert(map_size(m) == size);
    int i = 0;
    MapEntry_strv_int *it;
    map_iter(strv_int, m, it) {
        assert(streq(it->data.first, keys[i]));
        assert(it->data.second == values[i++]);
    }
    assert(i == size);
    i = size - 1;
    map_riter(strv_int, m, it) {
        assert(streq(it->data.first, keys[i]));
        assert(it->data.second == values[i--]);
    }
    assert(i == -1);
}

void test_basic_ints(void) {
    Map_int_str *m = map_new(int_str);
    assert(map_empty(m));
    compare_int_str(m, (int[]){}, (char*[]){}, 0);

    for (int i = 0; i < 50; ++i) {
        map_insert(int_str, m, pair_make(int_str, ints_rand[i], strs_rand[i]));
    }
    map_insert(int_str, m, pair_make(int_str, ints[0], strs[0]));
    map_insert(int_str, m, pair_make(int_str, ints[49], strs[49]));
    assert(!map_empty(m));
    compare_int_str(m, ints, strs, 50);

    assert(map_find(int_str, m, 121) == NULL);
    MapEntry_int_str *e = map_find(int_str, m, 120);
    assert(e != NULL);
    map_remove_entry(int_str, m, e);
    assert(map_find(int_str, m, 120) == NULL);
    assert(map_size(m) == 49);
    assert(m->root->data.first == 125);

    map_remove_key(int_str, m, 60);
    assert(map_find(int_str, m, 60) == NULL);
    map_remove_key(int_str, m, 60);
    assert(map_size(m) == 48);
    map_free(int_str, m);
}

void test_basic_strs(void) {
    Map_strp_int *m = map_new(strp_int);
    assert(map_empty(m));
    assert(map_size(m) == 0);

    {
        int i = 0;
        MapEntry_strp_int *it;
        map_iter(strp_int, m, it) {
            i++;
        }
        assert(it == NULL);
        assert(i == 0);
        i = -1;
        map_riter(strp_int, m, it) {
            i--;
        }
        assert(i == -1);
    }

    for (int i = 0; i < 50; ++i) {
        map_insert(strp_int, m, pair_make(strp_int, strs_rand[i], ints_rand[i]));
    }
    map_insert(strp_int, m, pair_make(strp_int, strs[0], ints[0]));
    map_insert(strp_int, m, pair_make(strp_int, strs[49], ints[49]));
    assert(!map_empty(m));

    {
        int i = 0;
        MapEntry_strp_int *it;
        map_iter(strp_int, m, it) {
            assert(streq(it->data.first, strs[i]));
            assert(it->data.second == ints[i++]);
        }
        assert(i == 50);
        i = 49;
        map_riter(strp_int, m, it) {
            assert(streq(it->data.first, strs[i]));
            assert(it->data.second == ints[i--]);
        }
        assert(i == -1);
    }

    assert(map_find(strp_int, m, "121") == NULL);
    MapEntry_strp_int *e = map_find(strp_int, m, "120");
    assert(e != NULL);
    map_remove_entry(strp_int, m, e);
    assert(map_find(strp_int, m, "120") == NULL);
    assert(map_size(m) == 49);
    assert(streq(m->root->data.first, "125"));

    map_remove_key(strp_int, m, "060");
    assert(map_find(strp_int, m, "060") == NULL);
    map_remove_key(strp_int, m, "060");
    assert(map_size(m) == 48);
    map_free(strp_int, m);
}

void test_init_clear(void) {
    Pair_int_str arrInt[50] = {}; Pair_strv_int arrStr[50] = {};
    for (int i = 0; i < 50; ++i) {
        arrInt[i] = pair_make(int_str, ints_rand[i], strs_rand[i]);
        arrStr[i] = pair_make(strv_int, strs_rand[i], ints_rand[i]);
    }
    Map_int_str *m1 = map_new_fromArray(int_str, arrInt, 50);
    Map_strv_int *m2 = map_new_fromArray(strv_int, arrStr, 50);
    compare_int_str(m1, ints, strs, 50);
    compare_str_int(m2, strs, ints, 50);

    Map_int_str *m3 = map_createCopy(int_str, m1);
    Map_strv_int *m4 = map_createCopy(strv_int, m2);
    compare_int_str(m1, ints, strs, 50);
    compare_str_int(m2, strs, ints, 50);

    map_clear(int_str, m1); map_clear(strv_int, m2);
    compare_int_str(m1, ints, strs, 0);
    compare_str_int(m2, strs, ints, 0);

    map_free(int_str, m1); map_free(strv_int, m2);
    map_free(int_str, m3); map_free(strv_int, m4);
}

void test_membership(void) {
    Pair_int_str arrInt[50] = {}; Pair_strv_int arrStr[50] = {};
    for (int i = 0; i < 50; ++i) {
        arrInt[i] = pair_make(int_str, ints_rand[i], strs_rand[i]);
        arrStr[i] = pair_make(strv_int, strs_rand[i], ints_rand[i]);
    }
    Map_int_str *m1 = map_new_fromArray(int_str, arrInt, 50);
    Map_strv_int *m2 = map_new_fromArray(strv_int, arrStr, 50);
    assert(!map_find(int_str, m1, 64) && !map_find(strv_int, m2, "064"));
    assert(!map_find(int_str, m1, 121) && !map_find(strv_int, m2, "121"));
    assert(map_find(int_str, m1, 0) && map_find(strv_int, m2, "000"));
    assert(map_find(int_str, m1, 245) && map_find(strv_int, m2, "245"));
    assert(!map_find(int_str, m1, -1) && !map_find(strv_int, m2, "..."));
    assert(!map_find(int_str, m1, 246) && !map_find(strv_int, m2, "246"));

    *map_at(int_str, m1, 120) = "500";
    MapEntry_int_str *p1 = map_find(int_str, m1, 120);
    assert(p1);
    assert(p1->data.first == 120 && streq(p1->data.second, "500"));
    *map_at(strv_int, m2, "120") = 500;
    MapEntry_strv_int *p2 = map_find(strv_int, m2, "120");
    assert(p2);
    assert(streq(p2->data.first, "120") && p2->data.second == 500);
    map_free(int_str, m1); map_free(strv_int, m2);
}

void test_remove(void) {
    Pair_int_str arrInt[10] = {}; Pair_strv_int arrStr[10] = {};
    for (int i = 0; i < 10; ++i) {
        arrInt[i] = pair_make(int_str, ints[i], strs[i]);
        arrStr[i] = pair_make(strv_int, strs[i], ints[i]);
    }
    Map_int_str *m1 = map_new_fromArray(int_str, arrInt, 10);
    Map_strv_int *m2 = map_new_fromArray(strv_int, arrStr, 10);

    map_remove_entry(int_str, m1, m1->root); map_remove_entry(strv_int, m2, m2->root);
    map_remove_key(int_str, m1, 5); map_remove_key(strv_int, m2, "005");

    MapEntry_int_str *begin1 = NULL, *end1 = NULL;
    MapEntry_strv_int *begin2 = NULL, *end2 = NULL;
    map_erase(int_str, m1, begin1, end1); map_erase(strv_int, m2, begin2, end2);
    begin1 = map_find(int_str, m1, 0), end1 = begin1;
    begin2 = map_find(strv_int, m2, "000"), end2 = begin2;
    map_erase(int_str, m1, begin1, end1); map_erase(strv_int, m2, begin2, end2);
    assert(map_size(m1) == 8 && map_size(m2) == 8);

    end1 = map_find(int_str, m1, 10), end2 = map_find(strv_int, m2, "010");
    map_erase(int_str, m1, begin1, end1); map_erase(strv_int, m2, begin2, end2);

    begin1 = map_find(int_str, m1, 45), end1 = NULL;
    begin2 = map_find(strv_int, m2, "045"), end2 = NULL;
    map_erase(int_str, m1, begin1, end1); map_erase(strv_int, m2, begin2, end2);

    begin1 = map_find(int_str, m1, 25), end1 = map_find(int_str, m1, 35);
    begin2 = map_find(strv_int, m2, "025"), end2 = map_find(strv_int, m2, "035");
    map_erase(int_str, m1, begin1, end1); map_erase(strv_int, m2, begin2, end2);

    compare_int_str(m1, (int[]){10,20,35,40}, (char*[]){"010","020","035","040"}, 4);
    compare_str_int(m2, (char*[]){"010","020","035","040"}, (int[]){10,20,35,40}, 4);
    map_free(int_str, m1); map_free(strv_int, m2);
}

void test_insert(void) {
    Pair_int_str arrInt[5] = {}; Pair_strv_int arrStr[5] = {};
    for (int i = 0; i < 5; ++i) {
        arrInt[i] = pair_make(int_str, ints[i], strs[i]);
        arrStr[i] = pair_make(strv_int, strs[i], ints[i]);
    }
    Map_int_str *m1 = map_new_fromArray(int_str, arrInt, 5);
    Map_strv_int *m2 = map_new_fromArray(strv_int, arrStr, 5);

    int inserted = -1;
    map_insert_withResult(int_str, m1, pair_make(int_str, 5, "500"), &inserted); //120
    assert(!inserted);
    inserted = -1;
    map_insert_withResult(strv_int, m2, pair_make(strv_int, "005", 500), &inserted);
    assert(!inserted);
    inserted = -1;
    map_insert_withResult(int_str, m1, pair_make(int_str, 4, "004"), &inserted); //119
    assert(inserted);
    inserted = -1;
    map_insert_withResult(strv_int, m2, pair_make(strv_int, "004", 4), &inserted);
    assert(inserted);

    {
        Pair_int_str arr1[3] = {pair_make(int_str, 14, "014"), pair_make(int_str, 15, "015"), pair_make(int_str, 16, "016")};
        Pair_strv_int arr2[3] = {pair_make(strv_int, "014", 14), pair_make(strv_int, "015", 15), pair_make(strv_int, "016", 16)};
        map_insert_fromArray(int_str, m1, arr1, 0);
        map_insert_fromArray(strv_int, m2, arr2, 0);
        map_insert_fromArray(int_str, m1, arr1, 3);
        map_insert_fromArray(strv_int, m2, arr2, 3);
    }
    compare_int_str(m1, (int[]){0,4,5,10,14,15,16,20}, (char*[]){"000","004","500","010","014","015","016","020"}, 8);
    compare_str_int(m2, (char*[]){"000","004","005","010","014","015","016","020"}, (int[]){0,4,500,10,14,15,16,20}, 8);

    Map_int_str *m3 = map_new(int_str);
    Map_strv_int *m4 = map_new(strv_int);
    MapEntry_int_str *begin1 = map_find(int_str, m1, 0), *end1 = begin1;
    MapEntry_strv_int *begin2 = map_find(strv_int, m2, "000"), *end2 = begin2;
    map_insert_fromMap(int_str, m3, begin1, end1); map_insert_fromMap(strv_int, m4, begin2, end2);
    assert(map_empty(m3) && map_empty(m4));

    end1 = map_find(int_str, m1, 5);
    end2 = map_find(strv_int, m2, "005");
    map_insert_fromMap(int_str, m3, begin1, end1); map_insert_fromMap(strv_int, m4, begin2, end2);

    begin1 = map_find(int_str, m1, 16), end1 = NULL;
    begin2 = map_find(strv_int, m2, "016"), end2 = NULL;
    map_insert_fromMap(int_str, m3, begin1, end1); map_insert_fromMap(strv_int, m4, begin2, end2);
    compare_int_str(m3, (int[]){0,4,16,20}, (char*[]){"000","004","016","020"}, 4);
    compare_str_int(m4, (char*[]){"000","004","016","020"}, (int[]){0,4,16,20}, 4);
    map_free(int_str, m3); map_free(strv_int, m4);
    map_free(int_str, m1); map_free(strv_int, m2);
}

void test_nested_dicts(void) {
    Pair_strv_int arrStr[50] = {};
    for (int i = 0; i < 50; ++i) {
        arrStr[i] = pair_make(strv_int, strs[i], ints[i]);
    }
    Map_nested *m = map_new(nested);

    for (int i = 0; i < 5; ++i) {
        int success = 0;
        Map_strv_int *inner = map_new_fromArray(strv_int, &arrStr[10 * i], 10);
        map_insert_withResult(nested, m, pair_make(nested, arrStr[10 * i].first, inner), &success);
        assert(success);
        assert(map_size(m) == i + 1);
    }

    MapEntry_nested *it = NULL;

    for (int i = 0; i < 5; ++i) {
        assert(it = map_find(nested, m, arrStr[10 * i].first));
        Map_strv_int *inner = it->data.second;
        for (int j = 0; j < 10; ++j) {
            int value = (i * 10) + j;
            MapEntry_strv_int *ptr = map_find(strv_int, inner, arrStr[value].first);
            assert(ptr && ptr->data.second == arrStr[value].second);
        }
    }

    int x = 0;
    map_iter(nested, m, it) {
        assert(streq(it->data.first, arrStr[10 * x].first));
        compare_str_int(it->data.second, &strs[10 * x], &ints[10 * x], 10);
        ++x;
    }
    assert(x == 5);
    x = 4;
    map_riter(nested, m, it) {
        --x;
    }
    assert(x == -1);
    map_free(nested, m);
}

int main(void) {
    test_basic_ints();
    test_basic_strs();
    test_init_clear();
    test_membership();
    test_remove();
    test_insert();
    test_nested_dicts();
    return 0;
}
