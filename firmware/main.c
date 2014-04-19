/*
  'SID GUTS' firmware 

  Copyright (c) 2014 ALMCo Ltd
  Parts based on code written by Alexis Kotlowy, released in Public Domain. 

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/
#include "uu.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

/* AVR Pins */
#define PIN_MULT_IN PIN_C0 
#define PIN_MULT_A PIN_C1
#define PIN_MULT_B PIN_C2 
#define PIN_MULT_C PIN_C3
#define PIN_MULT_D PIN_C4 
#define PIN_LED_I      PIN_B5 
#define PIN_LED_DATA   PIN_D0
#define PIN_LED_CLOCK  PIN_C5
#define PIN_LED_ENABLE PIN_D1

/* HW Related defines */
#define CCHAN_R 0
#define CCHAN_S 1
#define CCHAN_D 2
#define CCHAN_A 3
#define CCHAN_NONE 8

#define CCHAN_PWM 4
#define CCHAN_RES 5
#define CCHAN_FILT 6
#define CCHAN_SWITCH_RINGSYNC 9
#define CCHAN_CV 10
#define CCHAN_RINGSYNC_CV 11
#define CCHAN_SWITCH_FILTER 7
#define CCHAN_RINGSYNC 14 // to mod sel
#define CCHAN_SWITCH_WAVEFORM 13
#define CCHAN_WAVEFORM 12 

/* LED Flags */
#define LED_TRI   (1<<0)
#define LED_SAW   (1<<1)
#define LED_PULSE (1<<2)
#define LED_NOISE (1<<3)
#define LED_HI    (1<<4)
#define LED_MID   (1<<5)
#define LED_LO    (1<<6)
#define LED_RING  (1<<7)
#define LED_SYNC  (1<<8)

/* Switch flags */
#define SWITCH_FILTER (1<<2)
#define SWITCH_WAVEFORM (1<<3)
#define SWITCH_RINGSYNC (1<<4)

/* Filter flags */
#define FILTER_LP     0
#define FILTER_BP     1 
#define FILTER_HP     2
#define FILTER_NOTCH  3
#define FILTER_NONE   -1

/* Constants for Waveform */
#define WAVEFORM_NONE 0x0
#define WAVEFORM_TRI 0x10
#define WAVEFORM_SAW 0x20
#define WAVEFORM_PULSE 0x40
#define WAVEFORM_NOISE 0x80

/* OSC3 State */
#define STATE_NONE 0
#define STATE_RING 1
#define STATE_SYNC 2

/* SID Realted */
#define VOLUME 15
#define CHAN3_OFF (1<<7)

/* Freq LUT */
#define VOLTS_FREQ_MIN 1304
#define VOLTS_FREQ_MAX 41617

#define VOLTS_FREQ_N (614 + 1024)
#define VOLTS_FREQ_MAX_OFF 613
#define VOLTS_FREQ_MIN_OFF 0
#define VOLTS_FREQ_INIT_OFF (VOLTS_FREQ_MAX_OFF/2)

const unsigned int volts_to_freq[] PROGMEM = {
  461, 463, 464, 466, 467, 469, 471, 472, 474,
  475, 477, 479, 480, 482, 484, 485, 487,
  489, 490, 492, 494, 495, 497, 499, 500,
  502, 504, 505, 507, 509, 511, 512, 514,
  516, 517, 519, 521, 523, 525, 526, 528,
  530, 532, 534, 535, 537, 539, 541, 543,
  544, 546, 548, 550, 552, 554, 556, 558,
  559, 561, 563, 565, 567, 569, 571, 573,
  575, 577, 579, 581, 583, 585, 587, 589,
  591, 593, 595, 597, 599, 601, 603, 605,
  607, 609, 611, 613, 615, 617, 619, 621,
  623, 626, 628, 630, 632, 634, 636, 638,
  641, 643, 645, 647, 649, 652, 654, 656,
  658, 660, 663, 665, 667, 669, 672, 674,
  676, 679, 681, 683, 685, 688, 690, 692,
  695, 697, 700, 702, 704, 707, 709, 712,
  714, 716, 719, 721, 724, 726, 729, 731,
  734, 736, 739, 741, 744, 746, 749, 751,
  754, 756, 759, 761, 764, 767, 769, 772,
  774, 777, 780, 782, 785, 788, 790, 793,
  796, 798, 801, 804, 807, 809, 812, 815,
  818, 820, 823, 826, 829, 831, 834, 837,
  840, 843, 846, 849, 851, 854, 857, 860,
  863, 866, 869, 872, 875, 878, 881, 884,
  887, 890, 893, 896, 899, 902, 905, 908,
  911, 914, 917, 920, 923, 927, 930, 933,
  936, 939, 942, 946, 949, 952, 955, 959,
  962, 965, 968, 972, 975, 978, 982, 985,
  988, 992, 995, 998, 1002, 1005, 1008, 1012,
  1015, 1019, 1022, 1026, 1029, 1033, 1036, 1040,
  1043, 1047, 1050, 1054, 1057, 1061, 1065, 1068,
  1072, 1076, 1079, 1083, 1086, 1090, 1094, 1098,
  1101, 1105, 1109, 1113, 1116, 1120, 1124, 1128,
  1132, 1135, 1139, 1143, 1147, 1151, 1155, 1159,
  1163, 1167, 1171, 1174, 1178, 1182, 1186, 1190,
  1195, 1199, 1203, 1207, 1211, 1215, 1219, 1223,
  1227, 1231, 1236, 1240, 1244, 1248, 1253, 1257,
  1261, 1265, 1270, 1274, 1278, 1283, 1287, 1291,
  1296, 1300, 1304, 1309, 1313, 1318, 1322, 1327,
  1331, 1336, 1340, 1345, 1349, 1354, 1359, 1363,
  1368, 1372, 1377, 1382, 1386, 1391, 1396, 1401,
  1405, 1410, 1415, 1420, 1424, 1429, 1434, 1439,
  1444, 1449, 1454, 1459, 1464, 1469, 1474, 1479,
  1484, 1489, 1494, 1499, 1504, 1509, 1514, 1519,
  1524, 1529, 1535, 1540, 1545, 1550, 1556, 1561,
  1566, 1571, 1577, 1582, 1587, 1593, 1598, 1604,
  1609, 1615, 1620, 1626, 1631, 1637, 1642, 1648,
  1653, 1659, 1665, 1670, 1676, 1682, 1687, 1693,
  1699, 1704, 1710, 1716, 1722, 1728, 1734, 1739,
  1745, 1751, 1757, 1763, 1769, 1775, 1781, 1787,
  1793, 1799, 1805, 1812, 1818, 1824, 1830, 1836,
  1842, 1849, 1855, 1861, 1868, 1874, 1880, 1887,
  1893, 1899, 1906, 1912, 1919, 1925, 1932, 1938,
  1945, 1952, 1958, 1965, 1972, 1978, 1985, 1992,
  1998, 2005, 2012, 2019, 2026, 2033, 2039, 2046,
  2053, 2060, 2067, 2074, 2081, 2088, 2095, 2103,
  2110, 2117, 2124, 2131, 2138, 2146, 2153, 2160,
  2168, 2175, 2182, 2190, 2197, 2205, 2212, 2220,
  2227, 2235, 2242, 2250, 2257, 2265, 2273, 2281,
  2288, 2296, 2304, 2312, 2319, 2327, 2335, 2343,
  2351, 2359, 2367, 2375, 2383, 2391, 2399, 2407,
  2416, 2424, 2432, 2440, 2449, 2457, 2465, 2474,
  2482, 2490, 2499, 2507, 2516, 2524, 2533, 2541,
  2550, 2559, 2567, 2576, 2585, 2594, 2602, 2611,
  2620, 2629, 2638, 2647, 2656, 2665, 2674, 2683,
  2692, 2701, 2710, 2719, 2729, 2738, 2747, 2757,
  2766, 2775, 2785, 2794, 2804, 2813, 2823, 2832,
  2842, 2851, 2861, 2871, 2881, 2890, 2900, 2910,
  2920, 2930, 2940, 2950, 2960, 2970, 2980, 2990,
  3000, 3010, 3020, 3031, 3041, 3051, 3062, 3072,
  3082, 3093, 3103, 3114, 3124, 3135, 3146, 3156,
  3167, 3178, 3188, 3199, 3210, 3221, 3232, 3243,
  3254, 3265, 3276, 3287, 3298, 3309, 3321, 3332,
  3343, 3354, 3366, 3377, 3389, 3400, 3412, 3423,
  3435, 3447, 3458, 3470, 3482, 3494, 3505, 3517,
  3529, 3541, 3553, 3565, 3577, 3589, 3602, 3614,
  3626, 3638, 3651, 3663, 3676, 3688, 3700, 3713,
  3726, 3738, 3751, 3764, 3776, 3789, 3802, 3815,
  3828, 3841, 3854, 3867, 3880, 3893, 3906, 3920,
  3933, 3946, 3960, 3973, 3987, 4000, 4014, 4027,
  4041, 4055, 4068, 4082, 4096, 4110, 4124, 4138,
  4152, 4166, 4180, 4194, 4208, 4223, 4237, 4251,
  4266, 4280, 4295, 4309, 4324, 4339, 4353, 4368,
  4383, 4398, 4413, 4428, 4443, 4458, 4473, 4488,
  4503, 4518, 4534, 4549, 4565, 4580, 4596, 4611,
  4627, 4643, 4658, 4674, 4690, 4706, 4722, 4738,
  4754, 4770, 4786, 4802, 4819, 4835, 4851, 4868,
  4884, 4901, 4917, 4934, 4951, 4968, 4985, 5001,
  5018, 5035, 5052, 5070, 5087, 5104, 5121, 5139,
  5156, 5174, 5191, 5209, 5226, 5244, 5262, 5280,
  5298, 5316, 5334, 5352, 5370, 5388, 5406, 5425,
  5443, 5462, 5480, 5499, 5517, 5536, 5555, 5574,
  5592, 5611, 5630, 5650, 5669, 5688, 5707, 5727,
  5746, 5765, 5785, 5805, 5824, 5844, 5864, 5884,
  5904, 5924, 5944, 5964, 5984, 6004, 6025, 6045,
  6066, 6086, 6107, 6128, 6148, 6169, 6190, 6211,
  6232, 6253, 6275, 6296, 6317, 6339, 6360, 6382,
  6403, 6425, 6447, 6469, 6491, 6513, 6535, 6557,
  6579, 6601, 6624, 6646, 6669, 6691, 6714, 6737,
  6760, 6783, 6806, 6829, 6852, 6875, 6898, 6922,
  6945, 6969, 6992, 7016, 7040, 7064, 7088, 7112,
  7136, 7160, 7184, 7209, 7233, 7258, 7282, 7307,
  7332, 7357, 7381, 7407, 7432, 7457, 7482, 7507,
  7533, 7558, 7584, 7610, 7636, 7661, 7687, 7714,
  7740, 7766, 7792, 7819, 7845, 7872, 7898, 7925,
  7952, 7979, 8006, 8033, 8061, 8088, 8115, 8143,
  8170, 8198, 8226, 8254, 8282, 8310, 8338, 8366,
  8395, 8423, 8452, 8480, 8509, 8538, 8567, 8596,
  8625, 8654, 8684, 8713, 8743, 8772, 8802, 8832,
  8862, 8892, 8922, 8952, 8983, 9013, 9044, 9074,
  9105, 9136, 9167, 9198, 9229, 9260, 9292, 9323,
  9355, 9387, 9418, 9450, 9482, 9515, 9547, 9579,
  9612, 9644, 9677, 9710, 9743, 9776, 9809, 9842,
  9876, 9909, 9943, 9976, 10010, 10044, 10078, 10112,
  10147, 10181, 10215, 10250, 10285, 10320, 10355, 10390,
  10425, 10460, 10496, 10531, 10567, 10603, 10639, 10675,
  10711, 10748, 10784, 10821, 10857, 10894, 10931, 10968,
  11005, 11043, 11080, 11118, 11155, 11193, 11231, 11269,
  11307, 11346, 11384, 11423, 11461, 11500, 11539, 11578,
  11618, 11657, 11697, 11736, 11776, 11816, 11856, 11896,
  11936, 11977, 12018, 12058, 12099, 12140, 12181, 12223,
  12264, 12306, 12347, 12389, 12431, 12473, 12516, 12558,
  12601, 12643, 12686, 12729, 12772, 12816, 12859, 12903,
  12947, 12990, 13035, 13079, 13123, 13168, 13212, 13257,
  13302, 13347, 13392, 13438, 13483, 13529, 13575, 13621,
  13667, 13713, 13760, 13806, 13853, 13900, 13947, 13995,
  14042, 14090, 14138, 14185, 14234, 14282, 14330, 14379,
  14428, 14476, 14526, 14575, 14624, 14674, 14724, 14773,
  14824, 14874, 14924, 14975, 15026, 15077, 15128, 15179,
  15230, 15282, 15334, 15386, 15438, 15490, 15543, 15596,
  15648, 15701, 15755, 15808, 15862, 15915, 15969, 16024,
  16078, 16132, 16187, 16242, 16297, 16352, 16408, 16463,
  16519, 16575, 16631, 16688, 16744, 16801, 16858, 16915,
  16973, 17030, 17088, 17146, 17204, 17262, 17321, 17380,
  17438, 17498, 17557, 17616, 17676, 17736, 17796, 17857,
  17917, 17978, 18039, 18100, 18161, 18223, 18285, 18347,
  18409, 18471, 18534, 18597, 18660, 18723, 18787, 18850,
  18914, 18978, 19043, 19107, 19172, 19237, 19302, 19368,
  19433, 19499, 19565, 19632, 19698, 19765, 19832, 19899,
  19967, 20034, 20102, 20170, 20239, 20307, 20376, 20445,
  20515, 20584, 20654, 20724, 20794, 20865, 20935, 21006,
  21078, 21149, 21221, 21293, 21365, 21437, 21510, 21583,
  21656, 21730, 21803, 21877, 21951, 22026, 22100, 22175,
  22251, 22326, 22402, 22478, 22554, 22630, 22707, 22784,
  22861, 22939, 23017, 23095, 23173, 23251, 23330, 23409,
  23489, 23568, 23648, 23728, 23809, 23890, 23971, 24052,
  24133, 24215, 24297, 24380, 24462, 24545, 24629, 24712,
  24796, 24880, 24964, 25049, 25134, 25219, 25305, 25390,
  25476, 25563, 25649, 25736, 25824, 25911, 25999, 26087,
  26176, 26264, 26353, 26443, 26532, 26622, 26713, 26803,
  26894, 26985, 27077, 27169, 27261, 27353, 27446, 27539,
  27632, 27726, 27820, 27914, 28009, 28104, 28199, 28295,
  28391, 28487, 28583, 28680, 28778, 28875, 28973, 29071,
  29170, 29269, 29368, 29467, 29567, 29668, 29768, 29869,
  29970, 30072, 30174, 30276, 30379, 30482, 30585, 30689,
  30793, 30897, 31002, 31107, 31213, 31319, 31425, 31531,
  31638, 31745, 31853, 31961, 32069, 32178, 32287, 32397,
  32507, 32617, 32727, 32838, 32950, 33061, 33173, 33286,
  33399, 33512, 33626, 33740, 33854, 33969, 34084, 34199,
  34315, 34432, 34548, 34666, 34783, 34901, 35019, 35138,
  35257, 35377, 35497, 35617, 35738, 35859, 35980, 36102,
  36225, 36348, 36471, 36595, 36719, 36843, 36968, 37093,
  37219, 37345, 37472, 37599, 37726, 37854, 37983, 38111,
  38241, 38370, 38500, 38631, 38762, 38893, 39025, 39157,
  39290, 39423, 39557, 39691, 39826, 39961, 40096, 40232,
  40369, 40505, 40643, 40781, 40919, 41058, 41197, 41336,
  41477, 41617, 41758, 41900, 42042, 42184, 42327, 42471,
  42615, 42759, 42904, 43050, 43196, 43342, 43489, 43637,
  43785, 43933, 44082, 44231, 44381, 44532, 44683, 44834,
  44986, 45139, 45292, 45445, 45599, 45754, 45909, 46065,
  46221, 46378, 46535, 46693, 46851, 47010, 47169, 47329,
  47490, 47651, 47812, 47974, 48137, 48300, 48464, 48628,
  48793, 48958, 49124, 49291, 49458, 49626, 49794, 49963,
  50132, 50302, 50473, 50644, 50815, 50988, 51161, 51334,
  51508, 51683, 51858, 52034, 52210, 52387, 52565, 52743,
  52922, 53101, 53281, 53462, 53643, 53825, 54007, 54191,
  54374, 54559, 54744, 54929, 55115, 55302, 55490, 55678,
  55867, 56056, 56246, 56437, 56628, 56820, 57013, 57206,
  57400, 57595, 57790, 57986, 58182, 58380, 58578, 58776,
  58975, 59175, 59376, 59577, 59779, 59982, 60185, 60389,
  60594, 60799, 61006, 61212, 61420, 61628, 61837, 62047,
  62257, 62468, 62680, 62892, 63106, 63320, 63534, 63750,
  63966, 64183, 64400, 64619, 64838, 65058, 65278, 65499,
  65721, 65944, 66168, 66392, 66617, 66843, 67070, 67297,
  67525, 67754, 67984, 68214, 68446, 68678, 68911, 69144,
  69379, 69614, 69850, 70087, 70324, 70563, 70802, 71042,
  71283, 71524, 71767, 72010, 72254, 72499, 72745, 72992,
  73239, 73487, 73737, 73987, 74237, 74489, 74742, 74995,
  75249, 75504, 75760, 76017, 76275, 76534, 76793, 77053,
  77315, 77577, 77840, 78104, 78368, 78634, 78901, 79168,
  79437, 79706, 79976, 80247, 80519, 80792, 81066, 81341,
  81617, 81894, 82171, 82450, 82729, 83010, 83291, 83574,
  83857, 84141, 84426, 84713, 85000, 85288, 85577, 85867,
  86158, 86451, 86744, 87038, 87333, 87629, 87926, 88224,
  88523, 88823, 89124, 89427, 89730, 90034, 90339, 90645,
  90953, 91261, 91571, 91881, 92192, 92505, 92819, 93133,
  93449, 93766, 94084, 94403, 94723, 95044, 95366, 95689,
  96014, 96339, 96666, 96994, 97323, 97653, 97984, 98316,
  98649, 98984, 99319, 99656, 99994, 100333, 100673, 101014,
  101357, 101700, 102045, 102391, 102738, 103086, 103436, 103787,
  104138, 104492, 104846, 105201, 105558, 105916, 106275, 106635,
  106997, 107359, 107723, 108089, 108455, 108823, 109192, 109562,
  109933, 110306, 110680, 111055, 111432, 111809, 112189, 112569,
  112951, 113333, 113718, 114103, 114490, 114878, 115268, 115658,
  116051, 116444, 116839, 117235, 117632
};

typedef struct _SIDState 
{
  int      waveform;
  int      freq_chan_1;
  int      freq_chan_2;
  int      freq_chan_3;
  int      pulse_width;
  int      filter;
  int      resonance;
  int      filter_type;
  uint8_t  hp,bp,lp;
  uint8_t  chan_3_state;
  bool     gate_off;
} SIDstate;

SIDstate _sid;
int16_t _tune_offset = VOLTS_FREQ_INIT_OFF;

void cycle ();

void 
tuning_save()
{
  uu_interrupts_off();
  eeprom_write_word (1, _tune_offset);
  uu_interrupts_on();
}

void
settings_save()
{
  byte b;

  /* 1 byte for speed... */

  b = _sid.waveform 	    /* bits 8-4 */
    |(_sid.filter_type<<2)  /* Filter 4-2  */
    |_sid.chan_3_state;	    /* State  2-1  */

  uu_interrupts_off();
  eeprom_write_byte (0, b);
  uu_interrupts_on();
}

void
settings_load()
{
  byte b;

  b = eeprom_read_byte (0);

  if (b == 0xff) // default erased val.. waveform only goes to 0x80
    {
      _tune_offset = VOLTS_FREQ_INIT_OFF;
      return;
    }

  _sid.waveform = b & 0xf0;
  _sid.filter_type = (b & 0x0f) >> 2;
  _sid.chan_3_state = (b & 0x3);

  _tune_offset = eeprom_read_word (1);

  /* Safety on */
  if (_tune_offset < VOLTS_FREQ_MIN_OFF)
    _tune_offset = VOLTS_FREQ_MIN_OFF;
  if (_tune_offset > VOLTS_FREQ_MAX_OFF)
    _tune_offset = VOLTS_FREQ_MAX_OFF;

  switch (_sid.waveform) 
    {
    case WAVEFORM_NONE:
    case WAVEFORM_TRI:
    case WAVEFORM_SAW: 
    case WAVEFORM_PULSE: 
    case WAVEFORM_NOISE: 
      break;
    default:
      /* Something gone wrong - flash corrupt...? defaults */
      _sid.waveform = WAVEFORM_PULSE;
      break;
    }
}

void leds_set_mask(uint32_t mask)
{
  byte shift_mask = mask & 0xFF;

  if (mask & LED_SYNC)
    uu_pin_digital_write(PIN_LED_I, HIGH);
  else
    uu_pin_digital_write(PIN_LED_I, LOW);

  uu_pin_digital_write(PIN_LED_ENABLE, LOW);
  uu_pin_shift_out(PIN_LED_DATA, PIN_LED_CLOCK, MSBFIRST, shift_mask);  
  uu_pin_digital_write(PIN_LED_ENABLE, HIGH);
}

int analog_read()
{
  uint8_t low, high;
  
  /* switch to ADMUX for (1<<6) - AVcc with external capacitor on AREF pin  */
  ADMUX |= 0; // AREF, Internal Vref turned off  & chan 0 
  
  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
  /* Start conversion */
  ADCSRA |= (1<<ADSC);
  /* Wait.. */
  while (uu_bit_is_set(ADCSRA, ADSC));
  /* Collect */
  low  = ADCL;
  high = ADCH;

  return (high << 8) | low;
}

void select_chan(int chan)
{
  uu_pin_digital_write (PIN_MULT_A, (chan & 0x01));
  uu_pin_digital_write (PIN_MULT_B, ((chan >> 1) & 0x01));
  uu_pin_digital_write (PIN_MULT_C, ((chan >> 2) & 0x01));
  uu_pin_digital_write (PIN_MULT_D, ((chan >> 3) & 0x01));

  /* Datasheet assumes max 16 chan sampling freq at 173khz
   *  1/173000 = .00000578 = 5.78 usec
   *
   * - However - 10us is not enough, needs to be at least 500+
   *  or otherwsie inacuracys in CV reading etc ? 
   *  TODO: Figure out why - ADC? _delay_us() out? Multiplexer datasheet wrong ?
   *
   *  11 select chan calls take ~ 10.5ms, we have 20ms per cycle (minus code)
   */

  _delay_us(500);
}

int read_chan_analog(int chan)
{
  select_chan(chan);

  return analog_read(); 
}

bool read_chan_digital(int chan)
{
  select_chan(chan);

  return (analog_read() > 512);
}

byte switches_read_mask()
{
  byte button_mask = 0;

  if (read_chan_digital(CCHAN_SWITCH_FILTER))
    button_mask |= SWITCH_FILTER;
    
  if (read_chan_digital(CCHAN_SWITCH_WAVEFORM))
    button_mask |= SWITCH_WAVEFORM;

  if (read_chan_digital(CCHAN_SWITCH_RINGSYNC))
    button_mask |= SWITCH_RINGSYNC;

  return button_mask;
}


void SID_poke (uint8_t port, uint8_t data) 
{
  PORTB = (port & 0x0F) | 0x10;        // Lower bit, keep CS deactive
  PORTD = ((port | 0x20) & 0xF0) >> 2; // Upper bit, reset high
  PORTD |= 0x80;                       // Clock in Address
  _delay_us(10);
  PORTD &= ~(0x80);

  PORTB = (data & 0x0F) | 0x10;  // Lower bit, keep CS deactive
  PORTD = (data & 0xF0) >> 2;    // Upper bit
  PORTB &= ~(0x10);   // Activate /CS
  _delay_us(10);
  PORTB |= 0x10;      // Deactivate /CS
}  

void soundcheck()
{
  int waveforms[] = { WAVEFORM_PULSE, WAVEFORM_SAW, WAVEFORM_TRI, WAVEFORM_NOISE };
  int filters[] = { FILTER_LP, FILTER_BP, FILTER_HP, FILTER_NOTCH };
  int i,j, k;

  /* Test Waveforms */
  for (k=0;k<1;k++)
    {
      unsigned int n,f,d,e;

      SID_poke(4,WAVEFORM_PULSE|(0<<2)|(0<<1)|1);

      /* Set pulse width middle */
      SID_poke(2,uu_bit_low_byte(2056));
      SID_poke(3,uu_bit_high_byte(206));

      /* low pass */
      SID_poke(24, (CHAN3_OFF|(0<<6)|(0<<5)|(1<<4)|VOLUME));

      /* resonance */
      SID_poke(23,(8<<4)|9 /* 9 may be safer */);
      f = 1024 << 1;
      SID_poke(21,uu_bit_low_byte(f));     // Set filter value - 11bits
      SID_poke(22, uu_bit_high_byte(f << 5));

      for (j=0;j<4;j++)
	{
	  switch (waveforms[j])
	    {
	    case WAVEFORM_TRI:
	      leds_set_mask(LED_TRI);
	      break;
	    case WAVEFORM_SAW:
	      leds_set_mask(LED_SAW);
	      break;
	    case WAVEFORM_NOISE:
	      leds_set_mask(LED_NOISE);
	      break;
	    case WAVEFORM_PULSE:
	      leds_set_mask(LED_PULSE);
	      break;
	    }

	  for(i=0;i<12;i++) 
	    {
	      f = pgm_read_word(&volts_to_freq[(1024/12)*i]);

	      SID_poke(4,waveforms[j]|(0<<2)|(0<<1)|1);
	      SID_poke(0,f);   // Send frequency to chanel
	      SID_poke(1,f>>8);
	      for(d=0;d<65000;d++)   
		_delay_us(10);
	    }
	}
    }

  /* SWEEP NOISE */
  SID_poke(4,WAVEFORM_NOISE|(0<<2)|(0<<1)|1);
  SID_poke(0, 0x9999);
  SID_poke(1, 0x9999>>8);

  for (k=0;k<1;k++)
    for (j=0;j<4;j++)
      {
	switch (filters[j])
	  {
	  case FILTER_NOTCH:
	    SID_poke(24, (CHAN3_OFF|(1<<6)|(0<<5)|(1<<4)|VOLUME));
	    leds_set_mask(LED_LO|LED_HI);
	    break;
	  case FILTER_HP:
	    SID_poke(24, (CHAN3_OFF|(1<<6)|(0<<5)|(0<<4)|VOLUME));
	    leds_set_mask(LED_HI);
	    break;
	  case FILTER_LP:
	    SID_poke(24, (CHAN3_OFF|(0<<6)|(0<<5)|(1<<4)|VOLUME));
	    leds_set_mask(LED_LO);
	    break;
	  case FILTER_BP:
	    SID_poke(24, (CHAN3_OFF|(0<<6)|(1<<5)|(0<<4)|VOLUME));
	    leds_set_mask(LED_MID);
	    break;
	  }
	
	for (i=0;i<1024;i++)
	  {
	    int f = i << 1;
	    SID_poke(21,uu_bit_low_byte(f));     // Set filter value - 11bits
	    SID_poke(22, uu_bit_high_byte(f << 5));
	    _delay_us(10000);
	  }
      }
}

void setup () 
{
  int c, i = 0;

  PORTD = 0;
  PORTB = 0x10;   // PB4 is high to disable Chip Select
  DDRD  = 0xFE;   // Keep RxD as an input, everything else output
  DDRB  = 0xFF;

  uu_init(0);

  uu_pin_mode(PIN_LED_I, OUTPUT);

  uu_pin_mode(PIN_MULT_IN, INPUT);

  uu_pin_mode(PIN_MULT_A, OUTPUT);
  uu_pin_mode(PIN_MULT_B, OUTPUT);
  uu_pin_mode(PIN_MULT_C, OUTPUT);
  uu_pin_mode(PIN_MULT_D, OUTPUT);

  uu_pin_mode(PIN_LED_DATA, OUTPUT);
  uu_pin_mode(PIN_LED_CLOCK, OUTPUT);
  uu_pin_mode(PIN_LED_ENABLE, OUTPUT);

  /* Initialise Timer 0 OC0A to 1MHz on Pin 6. */
  TIMSK0 = 0;
  TCNT0  = 0;     /* Reset timer */
  OCR0A  = 0;     /* Invert clock after this many clock cycles + 1 */
  TCCR0A = (1<<COM0A0) | (2<<WGM00);
  TCCR0B = (2<<CS00); /* Prescaler */

  _sid.waveform = WAVEFORM_NONE;
  _sid.freq_chan_1 = -1;
  _sid.freq_chan_2 = 0;
  _sid.freq_chan_3 = 0;
  _sid.pulse_width = 0;
  _sid.filter = -1;
  _sid.resonance = -1;
  _sid.filter_type = FILTER_LP;
  _sid.hp = _sid.bp = 0; _sid.lp = 1;
  _sid.chan_3_state = STATE_NONE;
  _sid.gate_off = FALSE;

  settings_load();

  /* Give swinsid a chance to initialise... seems racey, not sure why */
  for (i=0;i<2000;i++)
    _delay_us(1000);

  /* Clear all regs */
  for(c=0; c<25;c++)
    SID_poke(c,0);

  /* Write some default values to the SID */
  SID_poke(24,15);    /* Turn up the volume */
  SID_poke(5,0);      /* Fast Attack, Decay */
  SID_poke(6,0xF0);   /* Full volume on sustain, quick release */
  SID_poke(4,0x21);   /* Enable gate, sawtooth waveform. */

  leds_set_mask(0);

  /* fire test here if waveform switch held down */
  while (read_chan_digital(CCHAN_SWITCH_WAVEFORM))
    {
      i++;
      if (i > 1000)
	{
	  while (1)
	    soundcheck();
	  SID_poke(4,0x21); 
	}
    }

  /* set up Timer 1 for processing input & output at 50hz (like real SID to avoid excessive noise) */
  TCCR1A = 0;                                     /* normal operation */
  TCCR1B = _BV(WGM12) | _BV(CS10) | _BV (CS12);   /* CTC, scale to clock / 1024 */
  OCR1A =  319;                                   /* compare A register value (319 * clock speed / 1024) = 50hz / 20ms */
  TIMSK1 = _BV (OCIE1A);                          /* interrupt on Compare A Match */
}

void cycle () 
{
  static bool last_gate = 0;
  static byte switch_ignore_mask = 0;
  static bool want_tune = FALSE;
  static bool first_run = TRUE;

  unsigned int f;
  uint8_t      c;
  int          i, waveform, state;
  bool         sync_waveform = FALSE, sync_filter = FALSE, reset_osc1 = FALSE;

  uint8_t      filter_mask = 0;
  int          led_mask    = 0;
  byte         switch_mask = 0;

#define CHECK_SWITCH(key) \
        (switch_mask & (key) && !(switch_ignore_mask & (key)))

  switch_mask = switches_read_mask();

  state = _sid.chan_3_state;

  /* Tuning */
  if ((CHECK_SWITCH(SWITCH_FILTER) && CHECK_SWITCH(SWITCH_RINGSYNC))
      || (CHECK_SWITCH(SWITCH_FILTER) && (switch_ignore_mask & SWITCH_RINGSYNC))
      || (CHECK_SWITCH(SWITCH_RINGSYNC) && (switch_ignore_mask & SWITCH_FILTER)))
    {
      want_tune = !want_tune;

      if (want_tune == FALSE)
	/* save tune setting */
	tuning_save();
      else
	state = STATE_NONE; /* turn off any ring or sync state 
			      accidentilly set by pressing ringmod switch */

      switch_ignore_mask |= (SWITCH_FILTER|SWITCH_RINGSYNC);
    }

  if (want_tune)
    {
      /* We dont CHECK_SWITCH as we want continuous handling */
      if (switch_mask & SWITCH_FILTER)
	{
	  _tune_offset--;
	  if (_tune_offset < VOLTS_FREQ_MIN_OFF)
	    _tune_offset = VOLTS_FREQ_MIN_OFF;
	}

      if (switch_mask & SWITCH_RINGSYNC)
	{
	  _tune_offset++;

	  if (_tune_offset > VOLTS_FREQ_MAX_OFF)
	    _tune_offset = VOLTS_FREQ_MAX_OFF;
	}
    }

  /* 
   *  - Waveform & Filter switches held should turn off the waveform... 
   *  - Waveform display is then blank, no LED lit.
   *  - Can be turned on again by pressing waveform butten
   */
  if ((CHECK_SWITCH(SWITCH_WAVEFORM) && CHECK_SWITCH(SWITCH_RINGSYNC))
      || (CHECK_SWITCH(SWITCH_WAVEFORM) && (switch_ignore_mask & SWITCH_RINGSYNC))
      || (CHECK_SWITCH(SWITCH_RINGSYNC) && (switch_ignore_mask & SWITCH_WAVEFORM)))

    {
      switch_ignore_mask |= SWITCH_WAVEFORM;

      if (_sid.gate_off != TRUE)
	{
	  SID_poke(6,0x00);   // No volume of sustain.. gate is not enough
	  _sid.gate_off = TRUE;
	  sync_waveform = TRUE; // So gate is toggled.
	}
    }
   
  if (CHECK_SWITCH(SWITCH_FILTER))
    {
      if (!want_tune)
	{
	  _sid.filter_type++;
	  
	  if (_sid.filter_type > FILTER_NOTCH)
	    _sid.filter_type = FILTER_LP;

	  sync_filter = TRUE;
	}

      switch_ignore_mask |= SWITCH_FILTER;
    }

  switch (_sid.filter_type)
    {
    case FILTER_NOTCH:
      led_mask |= (LED_HI|LED_LO);
      filter_mask = (CHAN3_OFF|(1<<6)|(0<<5)|(1<<4)|VOLUME);
      break;
    case FILTER_HP:
      led_mask |= LED_HI;
      filter_mask = (CHAN3_OFF|(1<<6)|(0<<5)|(0<<4)|VOLUME);
      break;
    case FILTER_LP:
      led_mask |= LED_LO;
      filter_mask = (CHAN3_OFF|(0<<6)|(0<<5)|(1<<4)|VOLUME);
      break;
    case FILTER_BP:
      led_mask |= LED_MID;
      filter_mask = (CHAN3_OFF|(0<<6)|(1<<5)|(0<<4)|VOLUME);
      break;
    }

  if (first_run)
    {
      first_run = FALSE;
      sync_filter = TRUE;
      sync_waveform = TRUE;
    }

  /* Initial waveform */
  if (_sid.waveform == WAVEFORM_NONE)
    {
      _sid.waveform = waveform = WAVEFORM_PULSE;
      sync_waveform = TRUE;
    }
  else 
    waveform = _sid.waveform;

  if (CHECK_SWITCH(SWITCH_WAVEFORM))
    {
      if (_sid.gate_off)
	{
	  SID_poke(6,0xF0);   /* Full volume on sustain back on */
	  _sid.gate_off = FALSE;
	  sync_waveform = TRUE;
	}
      else
	{
	  waveform = waveform*2;
	  reset_osc1 = TRUE; 	/* More safety on */
	}

      if (waveform>WAVEFORM_NOISE)
	waveform = WAVEFORM_TRI;

      switch_ignore_mask |= SWITCH_WAVEFORM;
    }

  /* extra delay here a below can pickup switch reading :/ */
  i = read_chan_analog(CCHAN_WAVEFORM);

  if (i > 50)
    {
      if (i < 350)
	{
	  waveform = WAVEFORM_NOISE;
	}
      else if (i < 550)
	{
	  waveform = WAVEFORM_TRI;
	}
      else if (i < 775)
	{
	  waveform = WAVEFORM_SAW;
	}
      else 
	{
	  waveform = WAVEFORM_PULSE;
	}

      if (waveform != _sid.waveform) 
	reset_osc1 = TRUE;
    }

  /* Waveform changed make sure we update */
  if (waveform != _sid.waveform) 
    {
      _sid.waveform = waveform;
      sync_waveform = TRUE;
    }

  /* update LED */
  if (_sid.gate_off == FALSE)
    {
      switch (_sid.waveform)
	{
	case WAVEFORM_PULSE:
	  led_mask |= LED_PULSE;
	  break;
	case WAVEFORM_TRI:
	  led_mask |= LED_TRI;
	  break;
	case WAVEFORM_SAW:
	  led_mask |= LED_SAW;
	  break;
	case WAVEFORM_NOISE:
	  led_mask |= LED_NOISE;
	  break;
	}
    }

  /* CV */
  i = read_chan_analog(CCHAN_CV);

  if (i != _sid.freq_chan_1 || want_tune) /* always set in tuning mode  */
    {
      _sid.freq_chan_1 = i;
      f = pgm_read_word(&volts_to_freq[i + _tune_offset]);
      SID_poke(0,f);   /* Send frequency to chanel */
      SID_poke(1,f>>8);
    }

  /*  Pulse width */
  i = (read_chan_analog(CCHAN_PWM) << 2); /* 12 bit value */

  /* 40 * 4 - cuts off so cant be heard */
  if (i<160) i = 160;
  if (i>4095) i = 4095;

  if (i != _sid.pulse_width)
    {
      SID_poke(2,uu_bit_low_byte(i));    /* Set pulse width low */
      SID_poke(3,uu_bit_high_byte(i));   /* Set pulse width high */
      _sid.pulse_width = i;
    }

  /* Filter */
  i = read_chan_analog(CCHAN_FILT) << 1;

  if (i<0) i = 0;

  if (i != _sid.filter)
    {
      SID_poke(21,uu_bit_low_byte(i) & 7);     // Set filter value - 11bits
      SID_poke(22, uu_bit_high_byte(i << 5));

      _sid.filter = i;
    }

  /* Resonance 4bit */
  i = (read_chan_analog(CCHAN_RES) >> 6);
  if (i<0) i = 0;
  if (i>15) i = 15;

  if (i != _sid.resonance) 
    {
      SID_poke(23,(i<<4)|9);  /* Set resonance and all channels on */
      _sid.resonance = i;
    }

  if (sync_filter)
    {
      SID_poke(24, filter_mask);
    }

  /* Modulation Osc */
  if (CHECK_SWITCH(SWITCH_RINGSYNC))
    {
      if (!want_tune && !_sid.gate_off)
	{
	  state++;

	  if (state > STATE_SYNC)
	    {
	      sync_waveform = TRUE;
	      state = STATE_NONE;
	    }
	}
      switch_ignore_mask |= SWITCH_RINGSYNC;
    }

  i = read_chan_analog(CCHAN_RINGSYNC);

  if (i > 300)
    {
      if (i < 750)
	{
	  state = STATE_SYNC;
	}
      else
	{
	  state = STATE_RING;
	}

      if (state != _sid.chan_3_state)
	reset_osc1 = TRUE; 		/* Safety on */
    }
  else
    {
      if (i > 100 && state != STATE_NONE)
	{
	  state = STATE_NONE;
	  sync_waveform = TRUE;
	  reset_osc1 = TRUE; 
	}
    }

  if (state == STATE_SYNC && !_sid.gate_off)
    led_mask |= LED_SYNC;

  if (state == STATE_RING && !_sid.gate_off)
    {
      led_mask |= LED_RING;

      led_mask &= ~(LED_PULSE|LED_SAW|LED_NOISE);
      led_mask |= LED_TRI;
    }

  if (state != STATE_NONE)
    {

      i = read_chan_analog(CCHAN_RINGSYNC_CV);
      if (i != _sid.freq_chan_3)
	{
	  _sid.freq_chan_3 = i;

	  f = pgm_read_word(&volts_to_freq[i]);
	  /* freq of oscillator 3 */
	  SID_poke(14,f); 
	  SID_poke(15,f>>8);
	}
    }

  if (reset_osc1) /* hack to avoid odd lockup with osc1 turning off */
    SID_poke(4,_sid.waveform|0);

  if (sync_waveform || _sid.chan_3_state != state)
    {
      int ring, sync, gate;

      ring = (state == STATE_RING) ? 1 : 0;
      sync = (state == STATE_SYNC) ? 1 : 0;
      gate = (_sid.gate_off == TRUE) ? 0 : 1;

      if (state == STATE_NONE && state != _sid.chan_3_state)
	  /* Make sure oscillator goes off - for swinsid */
	  SID_poke(18,_sid.waveform|0);

      if (ring) /* must be triangle for ring sinc */
	{
	  SID_poke(4,WAVEFORM_TRI|(ring<<2)|(sync<<1)|gate);
	  SID_poke(18,_sid.waveform|gate);
	}
      else
	SID_poke(4,_sid.waveform|(ring<<2)|(sync<<1)|gate);
    }

  _sid.chan_3_state = state;

  if (switch_mask == 0) 	// Nothing pressed so we clear the mask
    switch_ignore_mask = 0; 	// Maybe a time out here to debounce better?
  else
    settings_save(); 		/* something pressed so save settings */

  if (want_tune)
    led_mask |= (LED_HI|LED_LO|LED_MID|LED_SYNC|LED_RING);

  leds_set_mask(led_mask);

}

ISR(TIMER1_COMPA_vect)
{
  cycle();
}

int main(void)
{
  setup();
  while (TRUE);
}
