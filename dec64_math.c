/*
dec64_math.c
Elementary functions for DEC64.

dec64.com
2019-09-03
Public Domain

No warranty.

This file is a placeholder. It should be replaced with functions that are
faster and more accurate.
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "dec64.h"

#define D_2                           0x200LL
#define D_E              0x6092A113D8D574F0LL
#define D_HALF                        0x5FFLL
#define D_HALF_PI        0x37CE4F32BB21A6F0LL
#define D_NHALF_PI       0xC831B0CD44DE59F0LL
#define D_NPI            0x9063619A89BCB4F0LL
#define D_PI             0x6F9C9E6576434CF0LL
#define D_2PI            0x165286144ADA42F1LL

#define FAC              93

dec64 factorials[FAC] = {
    (1LL << 8) + 0,
    (1LL << 8) + 0,
    (2LL << 8) + 0,
    (6LL << 8) + 0,
    (24LL << 8) + 0,
    (120LL << 8) + 0,
    (720LL << 8) + 0,
    (5040LL << 8) + 0,
    (40320LL << 8) + 0,
    (362880LL << 8) + 0,
    (3628800LL << 8) + 0,
    (39916800LL << 8) + 0,
    (479001600LL << 8) + 0,
    (6227020800LL << 8) + 0,
    (87178291200LL << 8) + 0,
    (1307674368000LL << 8) + 0,
    (20922789888000LL << 8) + 0,
    (355687428096000LL << 8) + 0,
    (6402373705728000LL << 8) + 0,
    (12164510040883200LL << 8) + 1,
    (24329020081766400LL << 8) + 2,
    (5109094217170944LL << 8) + 4,
    (11240007277776077LL << 8) + 5,
    (25852016738884977LL << 8) + 6,
    (6204484017332394LL << 8) + 8,
    (15511210043330986LL << 8) + 9,
    (4032914611266056LL << 8) + 11,
    (10888869450418352LL << 8) + 12,
    (30488834461171386LL << 8) + 13,
    (8841761993739702LL << 8) + 15,
    (26525285981219106LL << 8) + 16,
    (8222838654177923LL << 8) + 18,
    (26313083693369353LL << 8) + 19,
    (8683317618811886LL << 8) + 21,
    (29523279903960414LL << 8) + 22,
    (10333147966386145LL << 8) + 24,
    (3719933267899012LL << 8) + 26,
    (13763753091226345LL << 8) + 27,
    (5230226174666011LL << 8) + 29,
    (20397882081197443LL << 8) + 30,
    (8159152832478977LL << 8) + 32,
    (33452526613163807LL << 8) + 33,
    (14050061177528799LL << 8) + 35,
    (6041526306337384LL << 8) + 37,
    (26582715747884488LL << 8) + 38,
    (11962222086548019LL << 8) + 40,
    (5502622159812089LL << 8) + 42,
    (25862324151116818LL << 8) + 43,
    (12413915592536073LL << 8) + 45,
    (6082818640342676LL << 8) + 47,
    (30414093201713378LL << 8) + 48,
    (15511187532873823LL << 8) + 50,
    (8065817517094388LL << 8) + 52,
    (4274883284060026LL << 8) + 54,
    (23084369733924138LL << 8) + 55,
    (12696403353658276LL << 8) + 57,
    (7109985878048635LL << 8) + 59,
    (4052691950487722LL << 8) + 61,
    (23505613312828786LL << 8) + 62,
    (13868311854568984LL << 8) + 64,
    (8320987112741390LL << 8) + 66,
    (5075802138772248LL << 8) + 68,
    (31469973260387938LL << 8) + 69,
    (19826083154044401LL << 8) + 71,
    (12688693218588416LL << 8) + 73,
    (8247650592082471LL << 8) + 75,
    (5443449390774431LL << 8) + 77,
    (3647111091818869LL << 8) + 79,
    (24800355424368306LL << 8) + 80,
    (17112245242814131LL << 8) + 82,
    (11978571669969892LL << 8) + 84,
    (8504785885678623LL << 8) + 86,
    (6123445837688609LL << 8) + 88,
    (4470115461512684LL << 8) + 90,
    (33078854415193864LL << 8) + 91,
    (24809140811395398LL << 8) + 93,
    (18854947016660503LL << 8) + 95,
    (14518309202828587LL << 8) + 97,
    (11324281178206298LL << 8) + 99,
    (8946182130782975LL << 8) + 101,
    (7156945704626380LL << 8) + 103,
    (5797126020747368LL << 8) + 105,
    (4753643337012842LL << 8) + 107,
    (3945523969720659LL << 8) + 109,
    (33142401345653533LL << 8) + 110,
    (28171041143805503LL << 8) + 112,
    (24227095383672732LL << 8) + 114,
    (21077572983795277LL << 8) + 116,
    (18548264225739844LL << 8) + 118,
    (16507955160908461LL << 8) + 120,
    (14857159644817615LL << 8) + 122,
    (1352001527678403LL << 8) + 124,
    (12438414054641307LL << 8) + 126
};

dec64 dec64_acos(dec64 slope) {
    dec64 result = dec64_subtract(
        D_HALF_PI,
        dec64_asin(slope)
    );
    return result;
}

dec64 dec64_asin(dec64 slope) {
    if (dec64_is_equal(slope, DEC64_ONE) == DEC64_TRUE) {
        return D_HALF_PI;
    }
    if (dec64_is_equal(slope, DEC64_NEGATIVE_ONE) == DEC64_TRUE) {
        return D_NHALF_PI;
    }
    if (
        dec64_is_nan(slope) == DEC64_TRUE ||
        dec64_is_less(DEC64_ONE, dec64_abs(slope)) == DEC64_TRUE
    ) {
        return DEC64_NAN;
    }
    dec64 bottom = D_2;
    dec64 factor = slope;
    dec64 x2 = dec64_multiply(slope, slope);
    dec64 result = factor;
    while (1) {
        factor = dec64_divide(
            dec64_multiply(
                dec64_multiply(dec64_add(DEC64_NEGATIVE_ONE, bottom), x2),
                factor
            ),
            bottom
        );
        dec64 progress = dec64_add(
            result,
            dec64_divide(factor, dec64_add(DEC64_ONE, bottom))
        );
        if (result == progress) {
            break;
        }
        result = progress;
        bottom = dec64_add(bottom, D_2);
    }
    return result;
}

dec64 dec64_atan(dec64 x) {
    int rev = 0, neg = 0;
    if (x < 0) {
        x = dec64_neg(x);
        neg = 1;
    }
    if (dec64_is_less(DEC64_ONE, x) == DEC64_TRUE) {
        x = dec64_divide(DEC64_ONE, x);
        rev = 1;
    }

    dec64 a = dec64_asin(
        dec64_divide(
            x,
            dec64_sqrt(dec64_add(DEC64_ONE, dec64_multiply(x, x)))
        )
    );
    if (rev) a = dec64_subtract(D_HALF_PI, a);
    if (neg) a = dec64_neg(a);
    return a;
}

dec64 dec64_atan2(dec64 y, dec64 x) {
    if (dec64_is_zero(x) == DEC64_TRUE) {
        if (dec64_is_zero(y) == DEC64_TRUE) {
            return DEC64_NAN;
        } else if (y < 0) {
            return D_NHALF_PI;
        } else {
            return D_HALF_PI;
        }
    } else {
        dec64 atan = dec64_atan(dec64_divide(y, x));
        if (x < 0) {
            if (y < 0) {
                return dec64_subtract(atan, D_HALF_PI);
            } else {
                return dec64_add(atan, D_HALF_PI);
            }
        } else {
            return atan;
        }
    }
}

dec64 dec64_cos(dec64 radians) {
    return dec64_sin(dec64_add(radians, D_HALF_PI));
}

#if 0
dec64 dec64_exp(dec64 exponent) {
    dec64 result = dec64_add(DEC64_ONE, exponent);
    dec64 divisor = D_2;
    dec64 term = exponent;
    int i = 1;
    for(;;i++) {
        term = dec64_divide(
            dec64_multiply(term, exponent),
            divisor
        );
        dec64 progress = dec64_add(result, term);
        if (result == progress) {
            break;
        }
        result = progress;
        divisor = dec64_add(DEC64_ONE, divisor);
    }
    //printf("\nexp taylor iters=%d\n", i);
    return result;
}
#else
#define DEC64_EXP_N 256
// exp(-i*log(10)/256), i=0..256
static const dec64 exptab[DEC64_EXP_N+1] = {
    2560000000000000240, 2537077391997084144, 2514360036321377008, 2491846095114035952,
    2469533746972670192, 2447421186803987696, 2425506625677761264, 2403788290682102256,
    2382264424780029680, 2360933286667324400, 2339793150631655920, 2318842306412969200,
    2298079059065124592, 2277501728818772976, 2257108650945461744, 2236898175622956272,
    2216868667801767664, 2197018507072874224, 2177346087536630512, 2157849817672847344,
    2138528120212035312, 2119379432007803376, 2100402203910397936, 2081594900641374704,
    2062956000669393648, 2044483996087124976, 2026177392489257456, 2008034708851600368,
    1990054477411266288, 1972235243547927792, 1954575565666136816, 1937074015078697968,
    1919729175891087088, 1902539644886902512, 1885504031414343920, 1868620957273707504,
    1851889056605887728, 1835306975781876976, 1818873373293257712, 1802586919643670512,
    1786446297241257968, 1770450200292069616, 1754597334694421488, 1738886417934201840,
    1723316178981115632, 1707885358185854960, 1692592707178193136, 1677436988765989872,
    1662416976835101168, 1647531456250185712, 1632779222756397808, 1618159082881963760,
    1603669853841627376, 1589310363440961008, 1575079449981535472, 1560975962166935792,
    1546998759009620208, 1533146709738614256, 1519418693708028656, 1505813600306398960,
    1492330328866835184, 1478967788577976816, 1465724898395745264, 1452600586955886064,
    1439593792487293936, 1426703462726115056, 1413928554830617584, 1401268035296823792,
    1388720879874899696, 1376286073486291440, 1363962610141604336, 1351749492859217392,
    1339645733584626672, 1327650353110509552, 1315762380997507568, 1303980855495714800,
    1292304823466872816, 1280733340307259888, 1269265469871272176, 1257900284395687664,
    1246636864424609776, 1235474298735082480, 1224411684263370736, 1213448126031902192,
    1202582737076862192, 1191814638376437488, 1181142958779703024, 1170566834936143856,
    1160085411225809648, 1149697839690094320, 1139403279963135216, 1129200899203825904,
    1119089872028439280, 1109069380443852784, 1099138613781371632, 1089296768631145456,
    1079543048777170672, 1069876665132877040, 1060296835677287920, 1050802785391755504,
    1041393746197260272, 1032068956892272880, 1022827663091171056, 1013669117163210992,
    1004592578172041456, 995597311815761392, 986682590367515376, 977847692616618480,
    969091903810209520, 960414515595427824, 951814825962105584, 943292139185975536,
    934845765772384752, 926475022400515056, 9181792318681008367, 9099577230366433519,
    9018098307771139823, 8937348959161452527, 8857322651827027183, 8778012911552347375,
    8699413322092952303, 8621517524656357103, 8544319217387618799, 8467812154859509487,
    8391990147567254255, 8316847061427793903, 8242376817283530479, 8168573390410516975,
    8095430810031051247, 8022943158830632687, 7951104572479244783, 7879909239156926191,
    7809351399083585519, 7739425344053031663, 7670125416971171567, 7601446011398345711,
    7533381571095761647, 7465926589575987951, 7399075609657472495, 7332823223023052271,
    7267164069782412527, 7202092838038467567, 7137604263457621487, 7073693128843878383,
    7010354263716765167, 6947582543893034223, 6885372891072111087, 6823720272425254895,
    6762619700188397039, 6702066231258624751, 6642054966794279663, 6582581051818636015,
    6523639674827127279, 6465226067398090735, 6407335503806996207, 6349963300644131055,
    6293104816435705583, 6236755451268354287, 6180910646416996847, 6125565883976032751,
    6070716686493837807, 6016358616610532847, 5962487276698997743, 5909098308509096687,
    5856187392815093231, 5803750249066218223, 5751782635040369647, 5700280346500912367,
    5649239216856550127, 5598655116824243951, 5548523954095149039, 5498841673003542255,
    5449604254198714095, 5400807714319799535, 5352448105673518831, 5304521515914805743,
    5257024067730294255, 5209951918524638959, 5163301260109646319, 5117068318396186607,
    5071249353088867567, 5025840657383438319, 4980838557666907631, 4936239413220341487,
    4892039615924327919, 4848235589967074031, 4804823791555120111, 4761800708626642159,
    4719162860567323119, 4676906797928766191, 4635029102149432303, 4593526385278073071,
    4552395289699642607, 4511632487863663343, 4471234682015022575, 4431198603927181295,
    4391521014637770223, 4352198704186555375, 4313228491355747567, 4274607223412639215,
    4236331775854544623, 4198399052156024559, 4160805983518373359, 4123549528621349871,
    4086626673377132271, 4050034430686473967, 4013769840197044463, 3977829968063933423,
    3942211906712299759, 3906912774602145519, 3871929715995196143, 3837259900723867887,
    3802900523962304495, 3768848805999463663, 3735101992014235375, 3701657351852573679,
    3668512179806624751, 3635663794395831535, 3603109538149999343, 3570846777394303727,
    3538872902036223215, 3507185325354380015, 3475781483789271279, 3444658836735874543,
    3413814866338109679, 3383247077285142511, 3352952996609510127, 3322930173487055855,
    3293176179038654959, 3263688606133715695, 3234465069195439599, 3205503204007825391,
    3176800667524402415, 3148355137678674159, 3120164313196261871, 3092225913408728303,
    3064537678069069039, 3037097367168856303, 3009902760757020143, 2982951658760251375,
    2956241880805013231, 2929771266041147119, 2903537672967057903, 2877538979256462831,
    2851773081586694383, 2826237895468538095, 2800931355077595887, 2775851413087158255,
    2750996040502573039, 2726363226497098479, 2701950978249223919, 2677757320781449455,
    2653780296800507119, 2630017966539014639, 2606468407598545647, 2583129714794105583,
    2560000000000000239
};

dec64 dec64_exp(dec64 x) {
    // x0 = n*10^e10, e^x0 = 10^(x0/log(10))
    // x = x0/log(10)
    // e10_ = ceil(x), x_ = x - e10_, -1 < x_ <= 0
    // e^x0 = 10^x = (10^x_) * 10^e10_ = e^(x_*log(10)) * 10^e10_
    dec64 log10 = (23025850929940457 << 8) | (-16 & 255);

    if (dec64_is_nan(x) == DEC64_TRUE) return DEC64_NULL;
    x = dec64_divide(x, log10);

    dec64 e10_ = dec64_ceiling(x);
    int64 e = dec64_normal(e10_) >> 8;
    if (e >= 200) return DEC64_NULL;
    if (e <= -200) return 0;
    dec64 y = dec64_subtract(x, e10_);
    dec64 y1 = dec64_normal(dec64_floor(dec64_multiply(y, -DEC64_EXP_N*256)));
    int tab_idx = y1 >> 8;
    tab_idx = tab_idx < 0 ? 0 : tab_idx > DEC64_EXP_N ? DEC64_EXP_N : tab_idx;
    y = dec64_multiply(dec64_add(y, dec64_divide(y1, DEC64_EXP_N*256)), log10);

    dec64 p = DEC64_ONE, s = DEC64_ONE;
    int n = 1, max_iters = 30;

    for(;n < max_iters; n++) {
        p = dec64_multiply(p, dec64_divide(y, n*256));
        dec64 progress = dec64_add(s, p);
        if (dec64_is_equal(progress, s) == DEC64_TRUE)
            break;
        s = progress;
    }
    //printf("exp taylor iters=%d\n", n);

    s = dec64_multiply(s, exptab[tab_idx]);
    if (dec64_is_nan(s) == DEC64_TRUE)
        return DEC64_NULL;
    return dec64_new(s >> 8, (signed char)s + e);
}
#endif

dec64 dec64_raise(dec64 coefficient, dec64 exponent) {
    if (dec64_is_zero(exponent) == DEC64_TRUE) {
        return DEC64_ONE;
    }

// Adjust for a negative exponent.

    if (exponent < 0) {
        coefficient = dec64_divide(DEC64_ONE, coefficient);
        exponent = dec64_neg(exponent);
    }
    if (dec64_is_nan(coefficient) == DEC64_TRUE) {
        return DEC64_NAN;
    }
    if (dec64_is_zero(coefficient) == DEC64_TRUE) {
        return 0;
    }

// If the exponent is an integer, then use the squaring method.

    if (exponent > 0 && dec64_exponent(exponent) == 0) {
        dec64 aux = DEC64_ONE;
        int64 n = dec64_coefficient(exponent);
        if (n <= 1) {
            return coefficient;
        }
        while (n > 1) {
            if ((n & 1) != 0) {
                aux = dec64_multiply(aux, coefficient);
            }
            coefficient = dec64_multiply(coefficient, coefficient);
            n /= 2;
        }
        return (n == 1)
            ? dec64_multiply(aux, coefficient)
            : aux;
    }

// Otherwise do it the hard way.

    return dec64_exp(dec64_multiply(
        dec64_log(coefficient),
        exponent
    ));
}

dec64 dec64_factorial(dec64 x) {
    dec64 n = dec64_normal(x);
    int64 c = dec64_coefficient(n);
    if (c >= 0 && c < FAC && dec64_exponent(n) == 0) {
        return factorials[c];
    }
    return DEC64_NAN;
}

#if 0
dec64 dec64_log(dec64 x) {
    if (x <= 0 || dec64_is_nan(x) == DEC64_TRUE) {
        return DEC64_NAN;
    }
    if (dec64_is_equal(x, DEC64_ONE) == DEC64_TRUE) {
        return DEC64_ZERO;
    }
    if (dec64_is_equal(x, D_HALF) == DEC64_TRUE) {
        return dec64_new(-6931471805599453, -16);
    }
    if (x == D_E) {
        return DEC64_ONE;
    }
    dec64 y, factor;
    if (dec64_is_less(x, DEC64_ONE) == DEC64_TRUE) {
        y = dec64_subtract(DEC64_ONE, x);
        factor = dec64_neg(y);
    } else {
        y = dec64_divide(dec64_add(DEC64_NEGATIVE_ONE, x), x);
        factor = y;
    }
    dec64 result = factor;
    dec64 divisor = D_2;

    int i = 1;
    for(;;i++) {
        factor = dec64_multiply(factor, y);
        dec64 f_scaled = dec64_divide(factor, divisor);
        dec64 progress = dec64_add(
            result,
            f_scaled
        );
        if (dec64_is_equal(result, progress) == DEC64_TRUE || progress == DEC64_NAN) {
            break;
        }
        result = progress;
        divisor = dec64_add(DEC64_ONE, divisor);
    }
    //printf("\nlog: taylor iters=%d\n", i);
    return result;
}
#else
// helper function that computes 10-th root of x; 0 < x < inf is assumed
dec64 __dec64_sqrt10(dec64 x, int max_iters)
{
    int i, rev = 0;
    if (dec64_is_less(DEC64_ONE, x) == DEC64_TRUE) {
        x = dec64_divide(DEC64_ONE, x);
        rev = 1;
    }

    dec64 y = DEC64_ONE;
    for(i = 1; i < max_iters; i++) {
        dec64 p = dec64_multiply(y, y);
        p = dec64_multiply(p, p);
        p = dec64_multiply(p, p);
        p = dec64_multiply(p, y);
        p = dec64_add(dec64_multiply(9 << 8, y), dec64_divide(x, p));
        p = dec64_new(p >> 8, (signed char)p - 1); // divide by 10
        if (dec64_is_equal(p, y) == DEC64_TRUE)
            break;
        y = p;
    }
    //printf("\nsqrt10: newton iters=%d", i);
    return rev ? dec64_divide(DEC64_ONE, y) : y;
}

dec64 dec64_log(dec64 x)
{
    dec64 log10 = (23025850929940457 << 8) | (-16 & 255);
    static signed char log10_prescale_tab[] =
    {
        0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7,
        8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 13,
        14, 14, 14, 15, 15, 16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 19
    };
    int e = (signed char)x;
    x >>= 8;
    if (e == -128 || x <= 0)
        return DEC64_NULL;
    int e_ = log10_prescale_tab[63 - __builtin_clzll(x)];
    e += e_;
    // by scaling coefficient by 10**-e_ bring x to 1
    // as close as possible; remember this scale in e
    x = dec64_new(x, -e_);
    // bring x even closer to 1 by taking 10-th root of it:
    // log(x) == 10*log(x**0.1)
    x = __dec64_sqrt10(x, 10);
    dec64 y = dec64_add(x, -256);
    dec64 s = y, p = y, d = 2*256;
    y = dec64_neg(y);
    int i = 1, max_iters = 30;
    for (; i < max_iters; i++) {
        p = dec64_multiply(p, y);
        dec64 progress = dec64_add(s, dec64_divide(p, d));
        if (dec64_is_equal(progress, s) == DEC64_TRUE)
            break;
        s = progress;
        d = dec64_add(d, DEC64_ONE);
    }
    //printf("\nlog: taylor iters=%d\n", i);
    if (dec64_is_nan(s) == DEC64_TRUE)
        return DEC64_NULL;
    return dec64_add(dec64_new(s >> 8, (signed char)s + 1),
                     dec64_multiply(e << 8, log10));
}
#endif

/*
    The seed variables contain the random number generator's state.
    They can be set by dec64_seed.
*/

static uint64 seed_0 = D_E;
static uint64 seed_1 = D_2PI;

dec64 dec64_random() {
/*
    Return a number between 0 and 1 containing 16 randomy digits.
    It uses xorshift128+.
*/
    while (1) {
        uint64 s1 = seed_0;
        uint64 s0 = seed_1;
        s1 ^= s1 << 23;
        s1 ^= s0 ^ (s0 >> 5) ^ (s1 >> 18);
        seed_0 = s0;
        seed_1 = s1;
        uint64 mantissa = (s1 + s0) >> 10;
/*
    mantissa contains an integer between 0 and 18014398509481983.
    If it is less than or equal to 9999999999999999 then we are done.
*/
        if (mantissa <= 9999999999999999LL) {
            return dec64_new(mantissa, -16);
        }
    }
}

dec64 dec64_root(dec64 index, dec64 radicand) {
    dec64 result;
    index = dec64_normal(index);
    if (
        dec64_is_nan(radicand) == DEC64_TRUE
        || dec64_is_zero(index) == DEC64_TRUE
        || index < 0
        || dec64_exponent(index) != 0
        || (
            radicand < 0
            && (dec64_coefficient(index) & 1) == 0
        )
    ) {
        return DEC64_NAN;
    }
    if (dec64_is_zero(radicand) == DEC64_TRUE) {
        return DEC64_ZERO;
    }
    if (index == DEC64_ONE) {
        return radicand;
    }
    if (index == D_2) {
        return dec64_sqrt(radicand);
    }
    dec64 index_minus_one = dec64_add(DEC64_NEGATIVE_ONE, index);
    result = DEC64_ONE;
    dec64 prosult = DEC64_NAN;
    while (1) {
        dec64 progress = dec64_divide(
            dec64_add(
                dec64_multiply(result, index_minus_one),
                dec64_divide(
                    radicand,
                    dec64_raise(result, index_minus_one)
                )
            ),
            index
        );
        if (progress == result) {
            return result;
        }
        if (progress == prosult) {
            return dec64_divide(dec64_add(progress, result), D_2);
        }
        prosult = result;
        result = progress;
    }
}

void dec64_seed(uint64 part_0, uint64 part_1) {
/*
    Seed the dec64_random function. It takes any 128 bits as the seed value.
    The seed must contain at least one 1 bit.
*/
    seed_0 = part_0;
    seed_1 = part_1;
    if ((seed_0 | seed_1) == 0) {
        seed_1 = 1;
    }
}

dec64 dec64_sin(dec64 radians) {
    radians = dec64_modulo(radians, D_2PI);
    while (dec64_is_less(D_PI, radians) == DEC64_TRUE) {
        radians = dec64_subtract(radians, D_PI);
        radians = dec64_subtract(radians, D_PI);
    }
    while (dec64_is_less(radians, D_NPI) == DEC64_TRUE) {
        radians = dec64_add(radians, D_PI);
        radians = dec64_add(radians, D_PI);
    }
    int neg = 0;
    if (radians < 0) {
        radians = dec64_neg(radians);
        neg = -1;
    }
    if (dec64_is_less(D_HALF_PI, radians) == DEC64_TRUE) {
        radians = dec64_subtract(D_PI, radians);
    }
    dec64 result;
    if (radians == D_HALF_PI) {
        result = DEC64_ONE;
    } else {
        dec64 x2 = dec64_multiply(radians, radians);
        dec64 order = DEC64_ONE;
        dec64 term = radians;
        result = term;
        while (1) {
            term = dec64_multiply(term, x2);
            order = dec64_add(DEC64_ONE, order);
            term = dec64_divide(term, order);
            order = dec64_add(DEC64_ONE, order);
            term = dec64_divide(term, order);
            dec64 progress = dec64_subtract(result, term);

            term = dec64_multiply(term, x2);
            order = dec64_add(DEC64_ONE, order);
            term = dec64_divide(term, order);
            order = dec64_add(DEC64_ONE, order);
            term = dec64_divide(term, order);
            progress = dec64_add(progress, term);

            if (progress == result) {
                break;
            }
            result = progress;
        }
    }
    if (neg) {
        result = dec64_neg(result);
    }
    return result;
}

dec64 dec64_sqrt(dec64 radicand) {
    if (dec64_is_nan(radicand) == DEC64_FALSE && radicand >= 0) {
        if (dec64_coefficient(radicand) == 0) {
            return DEC64_ZERO;
        }
        dec64 result = radicand;
        while (1) {
            dec64 progress = dec64_divide(
                dec64_add(
                    result,
                    dec64_divide(radicand, result)
                ),
                D_2
            );
            if (progress == result) {
                return result;
            }
            result = progress;
        }
        return result;
    } else {
        return DEC64_NAN;
    }
}

dec64 dec64_tan(dec64 radians) {
    return dec64_divide(
        dec64_sin(radians),
        dec64_cos(radians)
    );
}
