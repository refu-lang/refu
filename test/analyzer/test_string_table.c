#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>


#include "../testsupport.h"

#include CLIB_TEST_HELPERS


START_TEST(test_string_table_add_some) {
    struct string_table st;
    const struct RFstring *ret;

    static const struct RFstring s1 = RF_STRING_STATIC_INIT("str1");
    uint32_t h1;
    static const struct RFstring s2 = RF_STRING_STATIC_INIT("str2");
    uint32_t h2;
    static const struct RFstring s3 = RF_STRING_STATIC_INIT("str3");
    uint32_t h3;

    string_table_init(&st);

    ck_assert(string_table_add_str(&st, &s1, &h1));
    ck_assert(string_table_add_str(&st, &s2, &h2));
    ck_assert(string_table_add_str(&st, &s3, &h3));

    ret = string_table_get_str(&st, h1);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s1));

    ret = string_table_get_str(&st, h2);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s2));

    ret = string_table_get_str(&st, h3);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s3));

    string_table_deinit(&st);
}END_TEST

START_TEST(test_string_table_add_existing) {
    struct string_table st;
    const struct RFstring *ret;
    uint32_t prev_hash;

    static const struct RFstring s1 = RF_STRING_STATIC_INIT("str1");
    uint32_t h1;
    static const struct RFstring s2 = RF_STRING_STATIC_INIT("str2");
    uint32_t h2;
    static const struct RFstring s3 = RF_STRING_STATIC_INIT("str3");
    uint32_t h3;

    string_table_init(&st);

    ck_assert(string_table_add_str(&st, &s1, &h1));
    ck_assert(string_table_add_str(&st, &s2, &h2));
    ck_assert(string_table_add_str(&st, &s2, &prev_hash));
    ck_assert_uint_eq(h2, prev_hash);
    ck_assert(string_table_add_str(&st, &s3, &h3));
    ck_assert(string_table_add_str(&st, &s3, &prev_hash));
    ck_assert_uint_eq(h3, prev_hash);

    ret = string_table_get_str(&st, h1);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s1));

    ret = string_table_get_str(&st, h2);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s2));

    ret = string_table_get_str(&st, h3);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s3));

    string_table_deinit(&st);
} END_TEST

START_TEST(test_string_table_get_non_existing) {
    struct string_table st;
    const struct RFstring *ret;

    static const struct RFstring s1 = RF_STRING_STATIC_INIT("str1");
    uint32_t some_hash = 23415;
    uint32_t h1;

    string_table_init(&st);

    ck_assert(string_table_add_str(&st, &s1, &h1));

    ret = string_table_get_str(&st, h1);
    ck_assert(ret);
    ck_assert(rf_string_equal(ret, &s1));

    ck_assert(!string_table_get_str(&st, some_hash));

    string_table_deinit(&st);
}END_TEST

struct st_test_record {
    struct RFstring s;
    uint32_t hash;
};

static struct st_test_record recs_arr[] = {
    {.s=RF_STRING_STATIC_INIT("test_string1")},
    {.s=RF_STRING_STATIC_INIT("test_string2")},
    {.s=RF_STRING_STATIC_INIT("test_string3")},
    {.s=RF_STRING_STATIC_INIT("test_string4")},
    {.s=RF_STRING_STATIC_INIT("test_string5")},
    {.s=RF_STRING_STATIC_INIT("test_string6")},
    {.s=RF_STRING_STATIC_INIT("test_string7")},
    {.s=RF_STRING_STATIC_INIT("test_string8")},
    {.s=RF_STRING_STATIC_INIT("test_string9")},
    {.s=RF_STRING_STATIC_INIT("test_string10")},
    {.s=RF_STRING_STATIC_INIT("test_string11")},
    {.s=RF_STRING_STATIC_INIT("test_string12")},
    {.s=RF_STRING_STATIC_INIT("test_string13")},
    {.s=RF_STRING_STATIC_INIT("test_string14")},
    {.s=RF_STRING_STATIC_INIT("test_string15")},
    {.s=RF_STRING_STATIC_INIT("test_string16")},
    {.s=RF_STRING_STATIC_INIT("test_string17")},
    {.s=RF_STRING_STATIC_INIT("test_string18")},
    {.s=RF_STRING_STATIC_INIT("test_string19")},
    {.s=RF_STRING_STATIC_INIT("test_string20")},
    {.s=RF_STRING_STATIC_INIT("test_string21")},
    {.s=RF_STRING_STATIC_INIT("test_string22")},
    {.s=RF_STRING_STATIC_INIT("test_string23")},
    {.s=RF_STRING_STATIC_INIT("test_string24")},
    {.s=RF_STRING_STATIC_INIT("test_string25")},
    {.s=RF_STRING_STATIC_INIT("test_string26")},
    {.s=RF_STRING_STATIC_INIT("test_string27")},
    {.s=RF_STRING_STATIC_INIT("test_string28")},
    {.s=RF_STRING_STATIC_INIT("test_string29")},
    {.s=RF_STRING_STATIC_INIT("test_string30")},
    {.s=RF_STRING_STATIC_INIT("test_string31")},
    {.s=RF_STRING_STATIC_INIT("test_string32")},
    {.s=RF_STRING_STATIC_INIT("test_string33")},
    {.s=RF_STRING_STATIC_INIT("test_string34")},
    {.s=RF_STRING_STATIC_INIT("test_string35")},
    {.s=RF_STRING_STATIC_INIT("test_string36")},
    {.s=RF_STRING_STATIC_INIT("test_string37")},
    {.s=RF_STRING_STATIC_INIT("test_string38")},
    {.s=RF_STRING_STATIC_INIT("test_string39")},
    {.s=RF_STRING_STATIC_INIT("test_string40")},
    {.s=RF_STRING_STATIC_INIT("test_string41")},
    {.s=RF_STRING_STATIC_INIT("test_string42")},
    {.s=RF_STRING_STATIC_INIT("test_string43")},
    {.s=RF_STRING_STATIC_INIT("test_string44")},
    {.s=RF_STRING_STATIC_INIT("test_string45")},
    {.s=RF_STRING_STATIC_INIT("test_string46")},
    {.s=RF_STRING_STATIC_INIT("test_string47")},
    {.s=RF_STRING_STATIC_INIT("test_string48")},
    {.s=RF_STRING_STATIC_INIT("test_string49")},
    {.s=RF_STRING_STATIC_INIT("test_string50")},
    {.s=RF_STRING_STATIC_INIT("test_string51")},
    {.s=RF_STRING_STATIC_INIT("test_string52")},
    {.s=RF_STRING_STATIC_INIT("test_string53")},
    {.s=RF_STRING_STATIC_INIT("test_string54")},
    {.s=RF_STRING_STATIC_INIT("test_string55")},
    {.s=RF_STRING_STATIC_INIT("test_string56")},
    {.s=RF_STRING_STATIC_INIT("test_string57")},
    {.s=RF_STRING_STATIC_INIT("test_string58")},
    {.s=RF_STRING_STATIC_INIT("test_string59")},
    {.s=RF_STRING_STATIC_INIT("test_string60")},
    {.s=RF_STRING_STATIC_INIT("test_string61")},
    {.s=RF_STRING_STATIC_INIT("test_string62")},
    {.s=RF_STRING_STATIC_INIT("test_string63")},
    {.s=RF_STRING_STATIC_INIT("test_string64")},
    {.s=RF_STRING_STATIC_INIT("test_string65")},
    {.s=RF_STRING_STATIC_INIT("test_string66")},
    {.s=RF_STRING_STATIC_INIT("test_string67")},
    {.s=RF_STRING_STATIC_INIT("test_string68")},
    {.s=RF_STRING_STATIC_INIT("test_string69")},
    {.s=RF_STRING_STATIC_INIT("test_string70")},
    {.s=RF_STRING_STATIC_INIT("test_string71")},
    {.s=RF_STRING_STATIC_INIT("test_string72")},
    {.s=RF_STRING_STATIC_INIT("test_string73")},
    {.s=RF_STRING_STATIC_INIT("test_string74")},
    {.s=RF_STRING_STATIC_INIT("test_string75")},
    {.s=RF_STRING_STATIC_INIT("test_string76")},
    {.s=RF_STRING_STATIC_INIT("test_string77")},
    {.s=RF_STRING_STATIC_INIT("test_string78")},
    {.s=RF_STRING_STATIC_INIT("test_string79")},
    {.s=RF_STRING_STATIC_INIT("test_string80")},
    {.s=RF_STRING_STATIC_INIT("test_string81")},
    {.s=RF_STRING_STATIC_INIT("test_string82")},
    {.s=RF_STRING_STATIC_INIT("test_string83")},
    {.s=RF_STRING_STATIC_INIT("test_string84")},
    {.s=RF_STRING_STATIC_INIT("test_string85")},
    {.s=RF_STRING_STATIC_INIT("test_string86")},
    {.s=RF_STRING_STATIC_INIT("test_string87")},
    {.s=RF_STRING_STATIC_INIT("test_string88")},
    {.s=RF_STRING_STATIC_INIT("test_string89")},
    {.s=RF_STRING_STATIC_INIT("test_string90")},
    {.s=RF_STRING_STATIC_INIT("test_string91")},
    {.s=RF_STRING_STATIC_INIT("test_string92")},
    {.s=RF_STRING_STATIC_INIT("test_string93")},
    {.s=RF_STRING_STATIC_INIT("test_string94")},
    {.s=RF_STRING_STATIC_INIT("test_string95")},
    {.s=RF_STRING_STATIC_INIT("test_string96")},
    {.s=RF_STRING_STATIC_INIT("test_string97")},
    {.s=RF_STRING_STATIC_INIT("test_string98")},
    {.s=RF_STRING_STATIC_INIT("test_string99")},
    {.s=RF_STRING_STATIC_INIT("test_string100")},
    {.s=RF_STRING_STATIC_INIT("test_string101")},
    {.s=RF_STRING_STATIC_INIT("test_string102")},
    {.s=RF_STRING_STATIC_INIT("test_string103")},
    {.s=RF_STRING_STATIC_INIT("test_string104")},
    {.s=RF_STRING_STATIC_INIT("test_string105")},
    {.s=RF_STRING_STATIC_INIT("test_string106")},
    {.s=RF_STRING_STATIC_INIT("test_string107")},
    {.s=RF_STRING_STATIC_INIT("test_string108")},
    {.s=RF_STRING_STATIC_INIT("test_string109")},
    {.s=RF_STRING_STATIC_INIT("test_string110")},
    {.s=RF_STRING_STATIC_INIT("test_string111")},
    {.s=RF_STRING_STATIC_INIT("test_string112")},
    {.s=RF_STRING_STATIC_INIT("test_string113")},
    {.s=RF_STRING_STATIC_INIT("test_string114")},
    {.s=RF_STRING_STATIC_INIT("test_string115")},
    {.s=RF_STRING_STATIC_INIT("test_string116")},
    {.s=RF_STRING_STATIC_INIT("test_string117")},
    {.s=RF_STRING_STATIC_INIT("test_string118")},
    {.s=RF_STRING_STATIC_INIT("test_string119")},
    {.s=RF_STRING_STATIC_INIT("test_string120")},
    {.s=RF_STRING_STATIC_INIT("test_string121")},
    {.s=RF_STRING_STATIC_INIT("test_string122")},
    {.s=RF_STRING_STATIC_INIT("test_string123")},
    {.s=RF_STRING_STATIC_INIT("test_string124")},
    {.s=RF_STRING_STATIC_INIT("test_string125")},
    {.s=RF_STRING_STATIC_INIT("test_string126")},
    {.s=RF_STRING_STATIC_INIT("test_string127")},
    {.s=RF_STRING_STATIC_INIT("test_string128")},
    {.s=RF_STRING_STATIC_INIT("test_string129")},
    {.s=RF_STRING_STATIC_INIT("test_string130")},
    {.s=RF_STRING_STATIC_INIT("test_string131")},
    {.s=RF_STRING_STATIC_INIT("test_string132")},
    {.s=RF_STRING_STATIC_INIT("test_string133")},
    {.s=RF_STRING_STATIC_INIT("test_string134")},
    {.s=RF_STRING_STATIC_INIT("test_string135")},
    {.s=RF_STRING_STATIC_INIT("test_string136")},
    {.s=RF_STRING_STATIC_INIT("test_string137")},
    {.s=RF_STRING_STATIC_INIT("test_string138")},
    {.s=RF_STRING_STATIC_INIT("test_string139")},
    {.s=RF_STRING_STATIC_INIT("test_string140")},
    {.s=RF_STRING_STATIC_INIT("test_string141")},
    {.s=RF_STRING_STATIC_INIT("test_string142")},
    {.s=RF_STRING_STATIC_INIT("test_string143")},
    {.s=RF_STRING_STATIC_INIT("test_string144")},
    {.s=RF_STRING_STATIC_INIT("test_string145")},
    {.s=RF_STRING_STATIC_INIT("test_string146")},
    {.s=RF_STRING_STATIC_INIT("test_string147")},
    {.s=RF_STRING_STATIC_INIT("test_string148")},
    {.s=RF_STRING_STATIC_INIT("test_string149")},
    {.s=RF_STRING_STATIC_INIT("test_string150")},
    {.s=RF_STRING_STATIC_INIT("test_string151")},
    {.s=RF_STRING_STATIC_INIT("test_string152")},
    {.s=RF_STRING_STATIC_INIT("test_string153")},
    {.s=RF_STRING_STATIC_INIT("test_string154")},
    {.s=RF_STRING_STATIC_INIT("test_string155")},
    {.s=RF_STRING_STATIC_INIT("test_string156")},
    {.s=RF_STRING_STATIC_INIT("test_string157")},
    {.s=RF_STRING_STATIC_INIT("test_string158")},
    {.s=RF_STRING_STATIC_INIT("test_string159")},
    {.s=RF_STRING_STATIC_INIT("test_string160")},
    {.s=RF_STRING_STATIC_INIT("test_string161")},
    {.s=RF_STRING_STATIC_INIT("test_string162")},
    {.s=RF_STRING_STATIC_INIT("test_string163")},
    {.s=RF_STRING_STATIC_INIT("test_string164")},
    {.s=RF_STRING_STATIC_INIT("test_string165")},
    {.s=RF_STRING_STATIC_INIT("test_string166")},
    {.s=RF_STRING_STATIC_INIT("test_string167")},
    {.s=RF_STRING_STATIC_INIT("test_string168")},
    {.s=RF_STRING_STATIC_INIT("test_string169")},
    {.s=RF_STRING_STATIC_INIT("test_string170")},
    {.s=RF_STRING_STATIC_INIT("test_string171")},
    {.s=RF_STRING_STATIC_INIT("test_string172")},
    {.s=RF_STRING_STATIC_INIT("test_string173")},
    {.s=RF_STRING_STATIC_INIT("test_string174")},
    {.s=RF_STRING_STATIC_INIT("test_string175")},
    {.s=RF_STRING_STATIC_INIT("test_string176")},
    {.s=RF_STRING_STATIC_INIT("test_string177")},
    {.s=RF_STRING_STATIC_INIT("test_string178")},
    {.s=RF_STRING_STATIC_INIT("test_string179")},
    {.s=RF_STRING_STATIC_INIT("test_string180")},
    {.s=RF_STRING_STATIC_INIT("test_string181")},
    {.s=RF_STRING_STATIC_INIT("test_string182")},
    {.s=RF_STRING_STATIC_INIT("test_string183")},
    {.s=RF_STRING_STATIC_INIT("test_string184")},
    {.s=RF_STRING_STATIC_INIT("test_string185")},
    {.s=RF_STRING_STATIC_INIT("test_string186")},
    {.s=RF_STRING_STATIC_INIT("test_string187")},
    {.s=RF_STRING_STATIC_INIT("test_string188")},
    {.s=RF_STRING_STATIC_INIT("test_string189")},
    {.s=RF_STRING_STATIC_INIT("test_string190")},
    {.s=RF_STRING_STATIC_INIT("test_string191")},
    {.s=RF_STRING_STATIC_INIT("test_string192")},
    {.s=RF_STRING_STATIC_INIT("test_string193")},
    {.s=RF_STRING_STATIC_INIT("test_string194")},
    {.s=RF_STRING_STATIC_INIT("test_string195")},
    {.s=RF_STRING_STATIC_INIT("test_string196")},
    {.s=RF_STRING_STATIC_INIT("test_string197")},
    {.s=RF_STRING_STATIC_INIT("test_string198")},
    {.s=RF_STRING_STATIC_INIT("test_string199")},
    {.s=RF_STRING_STATIC_INIT("test_string200")},
    {.s=RF_STRING_STATIC_INIT("test_string201")},
    {.s=RF_STRING_STATIC_INIT("test_string202")},
    {.s=RF_STRING_STATIC_INIT("test_string203")},
    {.s=RF_STRING_STATIC_INIT("test_string204")},
    {.s=RF_STRING_STATIC_INIT("test_string205")},
    {.s=RF_STRING_STATIC_INIT("test_string206")},
    {.s=RF_STRING_STATIC_INIT("test_string207")},
    {.s=RF_STRING_STATIC_INIT("test_string208")},
    {.s=RF_STRING_STATIC_INIT("test_string209")},
    {.s=RF_STRING_STATIC_INIT("test_string210")},
    {.s=RF_STRING_STATIC_INIT("test_string211")},
    {.s=RF_STRING_STATIC_INIT("test_string212")},
    {.s=RF_STRING_STATIC_INIT("test_string213")},
    {.s=RF_STRING_STATIC_INIT("test_string214")},
    {.s=RF_STRING_STATIC_INIT("test_string215")},
    {.s=RF_STRING_STATIC_INIT("test_string216")},
    {.s=RF_STRING_STATIC_INIT("test_string217")},
    {.s=RF_STRING_STATIC_INIT("test_string218")},
    {.s=RF_STRING_STATIC_INIT("test_string219")},
    {.s=RF_STRING_STATIC_INIT("test_string220")},
    {.s=RF_STRING_STATIC_INIT("test_string221")},
    {.s=RF_STRING_STATIC_INIT("test_string222")},
    {.s=RF_STRING_STATIC_INIT("test_string223")},
    {.s=RF_STRING_STATIC_INIT("test_string224")},
    {.s=RF_STRING_STATIC_INIT("test_string225")},
    {.s=RF_STRING_STATIC_INIT("test_string226")},
    {.s=RF_STRING_STATIC_INIT("test_string227")},
    {.s=RF_STRING_STATIC_INIT("test_string228")},
    {.s=RF_STRING_STATIC_INIT("test_string229")},
    {.s=RF_STRING_STATIC_INIT("test_string230")},
    {.s=RF_STRING_STATIC_INIT("test_string231")},
    {.s=RF_STRING_STATIC_INIT("test_string232")},
    {.s=RF_STRING_STATIC_INIT("test_string233")},
    {.s=RF_STRING_STATIC_INIT("test_string234")},
    {.s=RF_STRING_STATIC_INIT("test_string235")},
    {.s=RF_STRING_STATIC_INIT("test_string236")},
    {.s=RF_STRING_STATIC_INIT("test_string237")},
    {.s=RF_STRING_STATIC_INIT("test_string238")},
    {.s=RF_STRING_STATIC_INIT("test_string239")},
    {.s=RF_STRING_STATIC_INIT("test_string240")},
    {.s=RF_STRING_STATIC_INIT("test_string241")},
    {.s=RF_STRING_STATIC_INIT("test_string242")},
    {.s=RF_STRING_STATIC_INIT("test_string243")},
    {.s=RF_STRING_STATIC_INIT("test_string244")},
    {.s=RF_STRING_STATIC_INIT("test_string245")},
    {.s=RF_STRING_STATIC_INIT("test_string246")},
    {.s=RF_STRING_STATIC_INIT("test_string247")},
    {.s=RF_STRING_STATIC_INIT("test_string248")},
    {.s=RF_STRING_STATIC_INIT("test_string249")},
    {.s=RF_STRING_STATIC_INIT("test_string250")},
    {.s=RF_STRING_STATIC_INIT("test_string251")},
    {.s=RF_STRING_STATIC_INIT("test_string252")},
    {.s=RF_STRING_STATIC_INIT("test_string253")},
    {.s=RF_STRING_STATIC_INIT("test_string254")},
    {.s=RF_STRING_STATIC_INIT("test_string255")},
    {.s=RF_STRING_STATIC_INIT("test_string256")},
    {.s=RF_STRING_STATIC_INIT("test_string257")},
    {.s=RF_STRING_STATIC_INIT("test_string258")},
    {.s=RF_STRING_STATIC_INIT("test_string259")},
    {.s=RF_STRING_STATIC_INIT("test_string260")},
    {.s=RF_STRING_STATIC_INIT("test_string261")},
    {.s=RF_STRING_STATIC_INIT("test_string262")},
    {.s=RF_STRING_STATIC_INIT("test_string263")},
    {.s=RF_STRING_STATIC_INIT("test_string264")},
    {.s=RF_STRING_STATIC_INIT("test_string265")},
    {.s=RF_STRING_STATIC_INIT("test_string266")},
    {.s=RF_STRING_STATIC_INIT("test_string267")},
    {.s=RF_STRING_STATIC_INIT("test_string268")},
    {.s=RF_STRING_STATIC_INIT("test_string269")},
    {.s=RF_STRING_STATIC_INIT("test_string270")},
    {.s=RF_STRING_STATIC_INIT("test_string271")},
    {.s=RF_STRING_STATIC_INIT("test_string272")},
    {.s=RF_STRING_STATIC_INIT("test_string273")},
    {.s=RF_STRING_STATIC_INIT("test_string274")},
    {.s=RF_STRING_STATIC_INIT("test_string275")},
    {.s=RF_STRING_STATIC_INIT("test_string276")},
    {.s=RF_STRING_STATIC_INIT("test_string277")},
    {.s=RF_STRING_STATIC_INIT("test_string278")},
    {.s=RF_STRING_STATIC_INIT("test_string279")},
    {.s=RF_STRING_STATIC_INIT("test_string280")},
    {.s=RF_STRING_STATIC_INIT("test_string281")},
    {.s=RF_STRING_STATIC_INIT("test_string282")},
    {.s=RF_STRING_STATIC_INIT("test_string283")},
    {.s=RF_STRING_STATIC_INIT("test_string284")},
    {.s=RF_STRING_STATIC_INIT("test_string285")},
    {.s=RF_STRING_STATIC_INIT("test_string286")},
    {.s=RF_STRING_STATIC_INIT("test_string287")},
    {.s=RF_STRING_STATIC_INIT("test_string288")},
    {.s=RF_STRING_STATIC_INIT("test_string289")},
    {.s=RF_STRING_STATIC_INIT("test_string290")},
    {.s=RF_STRING_STATIC_INIT("test_string291")},
    {.s=RF_STRING_STATIC_INIT("test_string292")},
    {.s=RF_STRING_STATIC_INIT("test_string293")},
    {.s=RF_STRING_STATIC_INIT("test_string294")},
    {.s=RF_STRING_STATIC_INIT("test_string295")},
    {.s=RF_STRING_STATIC_INIT("test_string296")},
    {.s=RF_STRING_STATIC_INIT("test_string297")},
    {.s=RF_STRING_STATIC_INIT("test_string298")},
    {.s=RF_STRING_STATIC_INIT("test_string299")},
    {.s=RF_STRING_STATIC_INIT("test_string300")},
    {.s=RF_STRING_STATIC_INIT("test_string301")},
    {.s=RF_STRING_STATIC_INIT("test_string302")},
    {.s=RF_STRING_STATIC_INIT("test_string303")},
    {.s=RF_STRING_STATIC_INIT("test_string304")},
    {.s=RF_STRING_STATIC_INIT("test_string305")},
    {.s=RF_STRING_STATIC_INIT("test_string306")},
    {.s=RF_STRING_STATIC_INIT("test_string307")},
    {.s=RF_STRING_STATIC_INIT("test_string308")},
    {.s=RF_STRING_STATIC_INIT("test_string309")},
    {.s=RF_STRING_STATIC_INIT("test_string310")},
    {.s=RF_STRING_STATIC_INIT("test_string311")},
    {.s=RF_STRING_STATIC_INIT("test_string312")},
    {.s=RF_STRING_STATIC_INIT("test_string313")},
    {.s=RF_STRING_STATIC_INIT("test_string314")},
    {.s=RF_STRING_STATIC_INIT("test_string315")},
    {.s=RF_STRING_STATIC_INIT("test_string316")},
    {.s=RF_STRING_STATIC_INIT("test_string317")},
    {.s=RF_STRING_STATIC_INIT("test_string318")},
    {.s=RF_STRING_STATIC_INIT("test_string319")},
    {.s=RF_STRING_STATIC_INIT("test_string320")},
    {.s=RF_STRING_STATIC_INIT("test_string321")},
    {.s=RF_STRING_STATIC_INIT("test_string322")},
    {.s=RF_STRING_STATIC_INIT("test_string323")},
    {.s=RF_STRING_STATIC_INIT("test_string324")},
    {.s=RF_STRING_STATIC_INIT("test_string325")},
    {.s=RF_STRING_STATIC_INIT("test_string326")},
    {.s=RF_STRING_STATIC_INIT("test_string327")},
    {.s=RF_STRING_STATIC_INIT("test_string328")},
    {.s=RF_STRING_STATIC_INIT("test_string329")},
    {.s=RF_STRING_STATIC_INIT("test_string330")},
    {.s=RF_STRING_STATIC_INIT("test_string331")},
    {.s=RF_STRING_STATIC_INIT("test_string332")},
    {.s=RF_STRING_STATIC_INIT("test_string333")},
    {.s=RF_STRING_STATIC_INIT("test_string334")},
    {.s=RF_STRING_STATIC_INIT("test_string335")},
    {.s=RF_STRING_STATIC_INIT("test_string336")},
    {.s=RF_STRING_STATIC_INIT("test_string337")},
    {.s=RF_STRING_STATIC_INIT("test_string338")},
    {.s=RF_STRING_STATIC_INIT("test_string339")},
    {.s=RF_STRING_STATIC_INIT("test_string340")},
    {.s=RF_STRING_STATIC_INIT("test_string341")},
    {.s=RF_STRING_STATIC_INIT("test_string342")},
    {.s=RF_STRING_STATIC_INIT("test_string343")},
    {.s=RF_STRING_STATIC_INIT("test_string344")},
    {.s=RF_STRING_STATIC_INIT("test_string345")},
    {.s=RF_STRING_STATIC_INIT("test_string346")},
    {.s=RF_STRING_STATIC_INIT("test_string347")},
    {.s=RF_STRING_STATIC_INIT("test_string348")},
    {.s=RF_STRING_STATIC_INIT("test_string349")},
    {.s=RF_STRING_STATIC_INIT("test_string350")},
    {.s=RF_STRING_STATIC_INIT("test_string351")},
    {.s=RF_STRING_STATIC_INIT("test_string352")},
    {.s=RF_STRING_STATIC_INIT("test_string353")},
    {.s=RF_STRING_STATIC_INIT("test_string354")},
    {.s=RF_STRING_STATIC_INIT("test_string355")},
    {.s=RF_STRING_STATIC_INIT("test_string356")},
    {.s=RF_STRING_STATIC_INIT("test_string357")},
    {.s=RF_STRING_STATIC_INIT("test_string358")},
    {.s=RF_STRING_STATIC_INIT("test_string359")},
    {.s=RF_STRING_STATIC_INIT("test_string360")},
    {.s=RF_STRING_STATIC_INIT("test_string361")},
    {.s=RF_STRING_STATIC_INIT("test_string362")},
    {.s=RF_STRING_STATIC_INIT("test_string363")},
    {.s=RF_STRING_STATIC_INIT("test_string364")},
    {.s=RF_STRING_STATIC_INIT("test_string365")},
    {.s=RF_STRING_STATIC_INIT("test_string366")},
    {.s=RF_STRING_STATIC_INIT("test_string367")},
    {.s=RF_STRING_STATIC_INIT("test_string368")},
    {.s=RF_STRING_STATIC_INIT("test_string369")},
    {.s=RF_STRING_STATIC_INIT("test_string370")},
    {.s=RF_STRING_STATIC_INIT("test_string371")},
    {.s=RF_STRING_STATIC_INIT("test_string372")},
    {.s=RF_STRING_STATIC_INIT("test_string373")},
    {.s=RF_STRING_STATIC_INIT("test_string374")},
    {.s=RF_STRING_STATIC_INIT("test_string375")},
    {.s=RF_STRING_STATIC_INIT("test_string376")},
    {.s=RF_STRING_STATIC_INIT("test_string377")},
    {.s=RF_STRING_STATIC_INIT("test_string378")},
    {.s=RF_STRING_STATIC_INIT("test_string379")},
    {.s=RF_STRING_STATIC_INIT("test_string380")},
    {.s=RF_STRING_STATIC_INIT("test_string381")},
    {.s=RF_STRING_STATIC_INIT("test_string382")},
    {.s=RF_STRING_STATIC_INIT("test_string383")},
    {.s=RF_STRING_STATIC_INIT("test_string384")},
    {.s=RF_STRING_STATIC_INIT("test_string385")},
    {.s=RF_STRING_STATIC_INIT("test_string386")},
    {.s=RF_STRING_STATIC_INIT("test_string387")},
    {.s=RF_STRING_STATIC_INIT("test_string388")},
    {.s=RF_STRING_STATIC_INIT("test_string389")},
    {.s=RF_STRING_STATIC_INIT("test_string390")},
    {.s=RF_STRING_STATIC_INIT("test_string391")},
    {.s=RF_STRING_STATIC_INIT("test_string392")},
    {.s=RF_STRING_STATIC_INIT("test_string393")},
    {.s=RF_STRING_STATIC_INIT("test_string394")},
    {.s=RF_STRING_STATIC_INIT("test_string395")},
    {.s=RF_STRING_STATIC_INIT("test_string396")},
    {.s=RF_STRING_STATIC_INIT("test_string397")},
    {.s=RF_STRING_STATIC_INIT("test_string398")},
    {.s=RF_STRING_STATIC_INIT("test_string399")},
    {.s=RF_STRING_STATIC_INIT("test_string400")},
    {.s=RF_STRING_STATIC_INIT("test_string401")},
    {.s=RF_STRING_STATIC_INIT("test_string402")},
    {.s=RF_STRING_STATIC_INIT("test_string403")},
    {.s=RF_STRING_STATIC_INIT("test_string404")},
    {.s=RF_STRING_STATIC_INIT("test_string405")},
    {.s=RF_STRING_STATIC_INIT("test_string406")},
    {.s=RF_STRING_STATIC_INIT("test_string407")},
    {.s=RF_STRING_STATIC_INIT("test_string408")},
    {.s=RF_STRING_STATIC_INIT("test_string409")},
    {.s=RF_STRING_STATIC_INIT("test_string410")},
    {.s=RF_STRING_STATIC_INIT("test_string411")},
    {.s=RF_STRING_STATIC_INIT("test_string412")},
    {.s=RF_STRING_STATIC_INIT("test_string413")},
    {.s=RF_STRING_STATIC_INIT("test_string414")},
    {.s=RF_STRING_STATIC_INIT("test_string415")},
    {.s=RF_STRING_STATIC_INIT("test_string416")},
    {.s=RF_STRING_STATIC_INIT("test_string417")},
    {.s=RF_STRING_STATIC_INIT("test_string418")},
    {.s=RF_STRING_STATIC_INIT("test_string419")},
    {.s=RF_STRING_STATIC_INIT("test_string420")},
    {.s=RF_STRING_STATIC_INIT("test_string421")},
    {.s=RF_STRING_STATIC_INIT("test_string422")},
    {.s=RF_STRING_STATIC_INIT("test_string423")},
    {.s=RF_STRING_STATIC_INIT("test_string424")},
    {.s=RF_STRING_STATIC_INIT("test_string425")},
    {.s=RF_STRING_STATIC_INIT("test_string426")},
    {.s=RF_STRING_STATIC_INIT("test_string427")},
    {.s=RF_STRING_STATIC_INIT("test_string428")},
    {.s=RF_STRING_STATIC_INIT("test_string429")},
    {.s=RF_STRING_STATIC_INIT("test_string430")},
    {.s=RF_STRING_STATIC_INIT("test_string431")},
    {.s=RF_STRING_STATIC_INIT("test_string432")},
    {.s=RF_STRING_STATIC_INIT("test_string433")},
    {.s=RF_STRING_STATIC_INIT("test_string434")},
    {.s=RF_STRING_STATIC_INIT("test_string435")},
    {.s=RF_STRING_STATIC_INIT("test_string436")},
    {.s=RF_STRING_STATIC_INIT("test_string437")},
    {.s=RF_STRING_STATIC_INIT("test_string438")},
    {.s=RF_STRING_STATIC_INIT("test_string439")},
    {.s=RF_STRING_STATIC_INIT("test_string440")},
    {.s=RF_STRING_STATIC_INIT("test_string441")},
    {.s=RF_STRING_STATIC_INIT("test_string442")},
    {.s=RF_STRING_STATIC_INIT("test_string443")},
    {.s=RF_STRING_STATIC_INIT("test_string444")},
    {.s=RF_STRING_STATIC_INIT("test_string445")},
    {.s=RF_STRING_STATIC_INIT("test_string446")},
    {.s=RF_STRING_STATIC_INIT("test_string447")},
    {.s=RF_STRING_STATIC_INIT("test_string448")},
    {.s=RF_STRING_STATIC_INIT("test_string449")},
    {.s=RF_STRING_STATIC_INIT("test_string450")},
    {.s=RF_STRING_STATIC_INIT("test_string451")},
    {.s=RF_STRING_STATIC_INIT("test_string452")},
    {.s=RF_STRING_STATIC_INIT("test_string453")},
    {.s=RF_STRING_STATIC_INIT("test_string454")},
    {.s=RF_STRING_STATIC_INIT("test_string455")},
    {.s=RF_STRING_STATIC_INIT("test_string456")},
    {.s=RF_STRING_STATIC_INIT("test_string457")},
    {.s=RF_STRING_STATIC_INIT("test_string458")},
    {.s=RF_STRING_STATIC_INIT("test_string459")},
    {.s=RF_STRING_STATIC_INIT("test_string460")},
    {.s=RF_STRING_STATIC_INIT("test_string461")},
    {.s=RF_STRING_STATIC_INIT("test_string462")},
    {.s=RF_STRING_STATIC_INIT("test_string463")},
    {.s=RF_STRING_STATIC_INIT("test_string464")},
    {.s=RF_STRING_STATIC_INIT("test_string465")},
    {.s=RF_STRING_STATIC_INIT("test_string466")},
    {.s=RF_STRING_STATIC_INIT("test_string467")},
    {.s=RF_STRING_STATIC_INIT("test_string468")},
    {.s=RF_STRING_STATIC_INIT("test_string469")},
    {.s=RF_STRING_STATIC_INIT("test_string470")},
    {.s=RF_STRING_STATIC_INIT("test_string471")},
    {.s=RF_STRING_STATIC_INIT("test_string472")},
    {.s=RF_STRING_STATIC_INIT("test_string473")},
    {.s=RF_STRING_STATIC_INIT("test_string474")},
    {.s=RF_STRING_STATIC_INIT("test_string475")},
    {.s=RF_STRING_STATIC_INIT("test_string476")},
    {.s=RF_STRING_STATIC_INIT("test_string477")},
    {.s=RF_STRING_STATIC_INIT("test_string478")},
    {.s=RF_STRING_STATIC_INIT("test_string479")},
    {.s=RF_STRING_STATIC_INIT("test_string480")},
    {.s=RF_STRING_STATIC_INIT("test_string481")},
    {.s=RF_STRING_STATIC_INIT("test_string482")},
    {.s=RF_STRING_STATIC_INIT("test_string483")},
    {.s=RF_STRING_STATIC_INIT("test_string484")},
    {.s=RF_STRING_STATIC_INIT("test_string485")},
    {.s=RF_STRING_STATIC_INIT("test_string486")},
    {.s=RF_STRING_STATIC_INIT("test_string487")},
    {.s=RF_STRING_STATIC_INIT("test_string488")},
    {.s=RF_STRING_STATIC_INIT("test_string489")},
    {.s=RF_STRING_STATIC_INIT("test_string490")},
    {.s=RF_STRING_STATIC_INIT("test_string491")},
    {.s=RF_STRING_STATIC_INIT("test_string492")},
    {.s=RF_STRING_STATIC_INIT("test_string493")},
    {.s=RF_STRING_STATIC_INIT("test_string494")},
    {.s=RF_STRING_STATIC_INIT("test_string495")},
    {.s=RF_STRING_STATIC_INIT("test_string496")},
    {.s=RF_STRING_STATIC_INIT("test_string497")},
    {.s=RF_STRING_STATIC_INIT("test_string498")},
    {.s=RF_STRING_STATIC_INIT("test_string499")},
    {.s=RF_STRING_STATIC_INIT("test_string500")},
    {.s=RF_STRING_STATIC_INIT("test_string501")},
    {.s=RF_STRING_STATIC_INIT("test_string502")},
    {.s=RF_STRING_STATIC_INIT("test_string503")},
    {.s=RF_STRING_STATIC_INIT("test_string504")},
    {.s=RF_STRING_STATIC_INIT("test_string505")},
    {.s=RF_STRING_STATIC_INIT("test_string506")},
    {.s=RF_STRING_STATIC_INIT("test_string507")},
    {.s=RF_STRING_STATIC_INIT("test_string508")},
    {.s=RF_STRING_STATIC_INIT("test_string509")},
    {.s=RF_STRING_STATIC_INIT("test_string510")},
    {.s=RF_STRING_STATIC_INIT("test_string511")},
    {.s=RF_STRING_STATIC_INIT("test_string512")},
    {.s=RF_STRING_STATIC_INIT("test_string513")},
    {.s=RF_STRING_STATIC_INIT("test_string514")},
    {.s=RF_STRING_STATIC_INIT("test_string515")},
    {.s=RF_STRING_STATIC_INIT("test_string516")},
    {.s=RF_STRING_STATIC_INIT("test_string517")},
    {.s=RF_STRING_STATIC_INIT("test_string518")},
    {.s=RF_STRING_STATIC_INIT("test_string519")},
    {.s=RF_STRING_STATIC_INIT("test_string520")},
    {.s=RF_STRING_STATIC_INIT("test_string521")},
    {.s=RF_STRING_STATIC_INIT("test_string522")},
    {.s=RF_STRING_STATIC_INIT("test_string523")},
    {.s=RF_STRING_STATIC_INIT("test_string524")},
    {.s=RF_STRING_STATIC_INIT("test_string525")},
    {.s=RF_STRING_STATIC_INIT("test_string526")},
    {.s=RF_STRING_STATIC_INIT("test_string527")},
    {.s=RF_STRING_STATIC_INIT("test_string528")},
    {.s=RF_STRING_STATIC_INIT("test_string529")},
    {.s=RF_STRING_STATIC_INIT("test_string530")},
    {.s=RF_STRING_STATIC_INIT("test_string531")},
    {.s=RF_STRING_STATIC_INIT("test_string532")},
    {.s=RF_STRING_STATIC_INIT("test_string533")},
    {.s=RF_STRING_STATIC_INIT("test_string534")},
    {.s=RF_STRING_STATIC_INIT("test_string535")},
    {.s=RF_STRING_STATIC_INIT("test_string536")},
    {.s=RF_STRING_STATIC_INIT("test_string537")},
    {.s=RF_STRING_STATIC_INIT("test_string538")},
    {.s=RF_STRING_STATIC_INIT("test_string539")},
    {.s=RF_STRING_STATIC_INIT("test_string540")},
};



START_TEST(test_string_table_add_many) {
    struct string_table st;
    const struct RFstring *ret;
    size_t i;
    size_t str_num = sizeof(recs_arr) / sizeof(struct st_test_record);

    string_table_init(&st);

    for (i = 0; i < str_num; ++i) {
        ck_assert_msg(
            string_table_add_str(&st, &recs_arr[i].s, &recs_arr[i].hash),
            "Failed to add string number %z to the string table", i);
    }

    for (i = 0; i < str_num; ++i) {
        ret = string_table_get_str(&st, recs_arr[i].hash);
        ck_assert_msg(ret, "Failed to get string %z by hash", i);
        ck_assert_msg(rf_string_equal(ret, &recs_arr[i].s),
                  "Failed to get string %z by hash", i);
    }

    string_table_deinit(&st);
} END_TEST

struct iter_cb_ctx {
    struct st_test_record *recs_arr;
    size_t size;
};

static void string_table_iterate_callback(const struct RFstring *s, void *user)
{
    unsigned int i;
    struct iter_cb_ctx *ctx = user;
    for (i = 0; i < ctx->size; ++i) {
        if (rf_string_equal(s, &recs_arr[i].s)) {
            return;
        }
    }
    ck_abort_msg("Could not find a string in the table during iteration");
}

START_TEST(test_string_table_iterate) {
    struct string_table st;
    struct iter_cb_ctx ctx;
    size_t i;
    ctx.size = sizeof(recs_arr) / sizeof(struct st_test_record);

    string_table_init(&st);

    for (i = 0; i < ctx.size; ++i) {
        ck_assert_msg(
            string_table_add_str(&st, &recs_arr[i].s, &recs_arr[i].hash),
            "Failed to add string number %z to the string table", i);
    }
    string_table_iterate(&st, string_table_iterate_callback, &ctx);

    string_table_deinit(&st);
} END_TEST

Suite *analyzer_stringtable_suite_create(void)
{
    Suite *s = suite_create("analyzer_string_table");

    TCase *st1 = tcase_create("analyzer_string_table_tests");
    tcase_add_checked_fixture(st1,
                              setup_base_tests,
                              teardown_base_tests);

    tcase_add_test(st1, test_string_table_add_some);
    tcase_add_test(st1, test_string_table_add_existing);
    tcase_add_test(st1, test_string_table_get_non_existing);
    tcase_add_test(st1, test_string_table_add_many);
    tcase_add_test(st1, test_string_table_iterate);

    suite_add_tcase(s, st1);
    return s;
}


