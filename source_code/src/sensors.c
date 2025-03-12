#include <sensors.h>

static uint8_t aux_adc_channels[NUM_AUX_ADC_CHANNELS] = {
    ADC_CHANNEL4,
    ADC_CHANNEL5,
    ADC_CHANNEL6,
    ADC_CHANNEL7,
};
static volatile uint16_t aux_adc_raw[NUM_AUX_ADC_CHANNELS];

static uint8_t sensores[NUM_SENSORES] = {
    ADC_CHANNEL10, // DETECTA SENSOR_FRONT_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL13, // DETECTA SENSOR_FRONT_RIGHT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL12, // DETECTA SENSOR_SIDE_LEFT_WALL_ID - NO CAMBIAR
    ADC_CHANNEL11, // DETECTA SENSOR_SIDE_RIGHT_WALL_ID - NO CAMBIAR
};
static volatile uint16_t sensors_raw[NUM_SENSORES];
static volatile uint16_t sensors_off[NUM_SENSORES];
static volatile uint16_t sensors_on[NUM_SENSORES];

uint16_t sensors_offsets[NUM_SENSORES] = {SENSOR_FRONT_LEFT_WALL_OFFSET,
                                          SENSOR_FRONT_RIGHT_WALL_OFFSET,
                                          SENSOR_SIDE_LEFT_WALL_OFFSET,
                                          SENSOR_SIDE_RIGHT_WALL_OFFSET};

float sensors_distance_slope[NUM_SENSORES] = {SENSOR_FRONT_LEFT_WALL_SLOPE,
                                              SENSOR_FRONT_RIGHT_WALL_SLOPE,
                                              SENSOR_SIDE_LEFT_WALL_SLOPE,
                                              SENSOR_SIDE_RIGHT_WALL_SLOPE};
uint16_t sensors_distance_intercept[NUM_SENSORES] = {SENSOR_FRONT_LEFT_WALL_INTERCEPT,
                                                     SENSOR_FRONT_RIGHT_WALL_INTERCEPT,
                                                     SENSOR_SIDE_LEFT_WALL_INTERCEPT,
                                                     SENSOR_SIDE_RIGHT_WALL_INTERCEPT};
#ifdef ZOROBOT3_A
struct front_sensors_distance_calibration front_sensors_distance_calibrations[] = {
    [SENSOR_FRONT_LEFT_WALL_ID] = {
        .close_offset = 0,
        .close_slope = -0.0346,
        .close_intercept = 284,
        .close_low_raw = 665,
        .close_high_raw = 1553,

        .far_offset = 0,
        .far_slope = -0.082,
        .far_intercept = 558,
        .far_low_raw = 121,
        .far_high_raw = 214,

        .offset = 0,
        .slope = -0.0545,
        .intercept = 422,
        .low_raw = -1,
        .high_raw = -1,
    },
    [SENSOR_FRONT_RIGHT_WALL_ID] = {
        .close_offset = 0,
        .close_slope = -0.0378,
        .close_intercept = 305,
        .close_low_raw = 722,
        .close_high_raw = 1501,

        .far_offset = 0,
        .far_slope = -0.0806,
        .far_intercept = 544,
        .far_low_raw = 96,
        .far_high_raw = 215,

        .offset = 0,
        .slope = -0.0539,
        .intercept = 414,
        .low_raw = -1,
        .high_raw = -1,
    },
};

struct side_sensors_distance_calibration side_sensors_distance_calibrations[] = {
    [SENSOR_SIDE_LEFT_WALL_ID] = {
        .offset = 0,
        .slope = -0.0492,
        .intercept = 397,
        .low_linearized = -1,
        .high_linearized = -1,
    },
    [SENSOR_SIDE_RIGHT_WALL_ID] = {
        .offset = 0,
        .slope = -0.0486,
        .intercept = 397,
        .low_linearized = -1,
        .high_linearized = -1,
    },
};
#endif

#ifdef ZOROBOT3_B
struct front_sensors_distance_calibration front_sensors_distance_calibrations[] = {
    [SENSOR_FRONT_LEFT_WALL_ID] = {
        .close_offset = 0,
        .close_slope = -0.0515,
        .close_intercept = 441,
        .close_low_raw = 1640,
        .close_high_raw = 2930,

        .far_offset = 0,
        .far_slope = -0.0828,
        .far_intercept = 639,
        .far_low_raw = 270,
        .far_high_raw = 550,

        .offset = 0,
        .slope = -0.0537,
        .intercept = 456,
        .low_raw = -1,
        .high_raw = -1,
    },
    [SENSOR_FRONT_RIGHT_WALL_ID] = {
        .close_offset = 0,
        .close_slope = -0.0484,
        .close_intercept = 415,
        .close_low_raw = 1540,
        .close_high_raw = 2860,

        .far_offset = 0,
        .far_slope = -0.0845,
        .far_intercept = 647,
        .far_low_raw = 260,
        .far_high_raw = 530,

        .offset = 0,
        .slope = -0.0532,
        .intercept = 451,
        .low_raw = -1,
        .high_raw = -1,
    },
};

struct side_sensors_distance_calibration side_sensors_distance_calibrations[] = {
    [SENSOR_SIDE_LEFT_WALL_ID] = {
        .offset = 0,
        .slope = -0.0563,
        .intercept = 447,
        .low_linearized = -1,
        .high_linearized = -1,
    },
    [SENSOR_SIDE_RIGHT_WALL_ID] = {
        .offset = 0,
        .slope = -0.0529,
        .intercept = 435,
        .low_linearized = -1,
        .high_linearized = -1,
    },
};
#endif

#define LOG_LINEARIZATION_TABLE_STEP 4
#define LOG_LINEARIZATION_TABLE_SIZE (ADC_RESOLUTION / LOG_LINEARIZATION_TABLE_STEP)

/**
 * @brief Tabla para linealización de sensores en base a la función de logaritmo.
 * Contiene los valores de logaritmo de los valores entre `1` y `ADC_RESOLUTION - 1`.
 *
 * ? It uses steps of size `LOG_LINEARIZATION_TABLE_STEP`.
 * ? Usa pasos de tamaño `LOG_LINEARIZATION_TABLE_STEP`.
 * ? Para evitar resultados inestables, calcula `ln(0)` como `1`.
 */
const float ln_linearization[LOG_LINEARIZATION_TABLE_SIZE] = {
    1., 1.3863, 2.0794, 2.4849, 2.7726, 2.9957, 3.1781, 3.3322, 3.4657, 3.5835, 3.6889, 3.7842, 3.8712, 3.9512, 4.0254, 4.0943, 4.1589, 4.2195, 4.2767, 4.3307, 4.382, 4.4308, 4.4773, 4.5218, 4.5643, 4.6052, 4.6444, 4.6821, 4.7185, 4.7536, 4.7875, 4.8203, 4.852, 4.8828, 4.9127, 4.9416, 4.9698, 4.9972, 5.0239, 5.0499, 5.0752, 5.0999, 5.124, 5.1475, 5.1705, 5.193, 5.2149, 5.2364, 5.2575, 5.2781, 5.2983, 5.3181, 5.3375, 5.3566, 5.3753, 5.3936, 5.4116, 5.4293, 5.4467, 5.4638, 5.4806, 5.4972, 5.5134, 5.5294, 5.5452, 5.5607, 5.5759, 5.591, 5.6058, 5.6204, 5.6348, 5.649, 5.663, 5.6768, 5.6904, 5.7038, 5.717, 5.7301, 5.743, 5.7557, 5.7683, 5.7807, 5.793, 5.8051, 5.8171, 5.8289, 5.8406, 5.8522, 5.8636, 5.8749, 5.8861, 5.8972, 5.9081, 5.9189, 5.9296, 5.9402, 5.9506, 5.961, 5.9713, 5.9814, 5.9915, 6.0014, 6.0113, 6.021, 6.0307, 6.0403, 6.0497, 6.0591, 6.0684, 6.0776, 6.0868, 6.0958, 6.1048, 6.1137, 6.1225, 6.1312, 6.1399, 6.1485, 6.157, 6.1654, 6.1738, 6.1821, 6.1903, 6.1985, 6.2066, 6.2146, 6.2226, 6.2305, 6.2383, 6.2461, 6.2538, 6.2615, 6.2691, 6.2766, 6.2841, 6.2916, 6.2989, 6.3063, 6.3135, 6.3208, 6.3279, 6.3351, 6.3421, 6.3491, 6.3561, 6.363, 6.3699, 6.3767, 6.3835, 6.3902, 6.3969, 6.4036, 6.4102, 6.4167, 6.4232, 6.4297, 6.4362, 6.4425, 6.4489, 6.4552, 6.4615, 6.4677, 6.4739, 6.48, 6.4862, 6.4922, 6.4983, 6.5043, 6.5103, 6.5162, 6.5221, 6.528, 6.5338, 6.5396, 6.5453, 6.5511, 6.5568, 6.5624, 6.5681, 6.5737, 6.5793, 6.5848, 6.5903, 6.5958, 6.6012, 6.6067, 6.612, 6.6174, 6.6227, 6.628, 6.6333, 6.6386, 6.6438, 6.649, 6.6542, 6.6593, 6.6644, 6.6695, 6.6746, 6.6796, 6.6846, 6.6896, 6.6946, 6.6995, 6.7044, 6.7093, 6.7142, 6.719, 6.7238, 6.7286, 6.7334, 6.7382, 6.7429, 6.7476, 6.7523, 6.7569, 6.7616, 6.7662, 6.7708, 6.7754, 6.7799, 6.7845, 6.789, 6.7935, 6.7979, 6.8024, 6.8068, 6.8112, 6.8156, 6.82, 6.8244, 6.8287, 6.833, 6.8373, 6.8416, 6.8459, 6.8501, 6.8544, 6.8586, 6.8628, 6.8669, 6.8711, 6.8752, 6.8794, 6.8835, 6.8876, 6.8916, 6.8957, 6.8997, 6.9037, 6.9078, 6.9117, 6.9157, 6.9197, 6.9236, 6.9276, 6.9315, 6.9354, 6.9393, 6.9431, 6.947, 6.9508, 6.9546, 6.9584, 6.9622, 6.966, 6.9698, 6.9735, 6.9773, 6.981, 6.9847, 6.9884, 6.9921, 6.9958, 6.9994, 7.0031, 7.0067, 7.0103, 7.0139, 7.0175, 7.0211, 7.0246, 7.0282, 7.0317, 7.0353, 7.0388, 7.0423, 7.0458, 7.0493, 7.0527, 7.0562, 7.0596, 7.063, 7.0665, 7.0699, 7.0733, 7.0767, 7.08, 7.0834, 7.0867, 7.0901, 7.0934, 7.0967, 7.1, 7.1033, 7.1066, 7.1099, 7.1131, 7.1164, 7.1196, 7.1229, 7.1261, 7.1293, 7.1325, 7.1357, 7.1389, 7.142, 7.1452, 7.1483, 7.1515, 7.1546, 7.1577, 7.1608, 7.1639, 7.167, 7.1701, 7.1732, 7.1763, 7.1793, 7.1824, 7.1854, 7.1884, 7.1914, 7.1944, 7.1974, 7.2004, 7.2034, 7.2064, 7.2093, 7.2123, 7.2152, 7.2182, 7.2211, 7.224, 7.2269, 7.2298, 7.2327, 7.2356, 7.2385, 7.2414, 7.2442, 7.2471, 7.2499, 7.2528, 7.2556, 7.2584, 7.2612, 7.264, 7.2668, 7.2696, 7.2724, 7.2752, 7.2779, 7.2807, 7.2834, 7.2862, 7.2889, 7.2917, 7.2944, 7.2971, 7.2998, 7.3025, 7.3052, 7.3079, 7.3106, 7.3132, 7.3159, 7.3185, 7.3212, 7.3238, 7.3265, 7.3291, 7.3317, 7.3343, 7.3369, 7.3395, 7.3421, 7.3447, 7.3473, 7.3499, 7.3524, 7.355, 7.3576, 7.3601, 7.3626, 7.3652, 7.3677, 7.3702, 7.3727, 7.3753, 7.3778, 7.3803, 7.3827, 7.3852, 7.3877, 7.3902, 7.3926, 7.3951, 7.3976, 7.4, 7.4025, 7.4049, 7.4073, 7.4097, 7.4122, 7.4146, 7.417, 7.4194, 7.4218, 7.4242, 7.4265, 7.4289, 7.4313, 7.4337, 7.436, 7.4384, 7.4407, 7.4431, 7.4454, 7.4478, 7.4501, 7.4524, 7.4547, 7.457, 7.4593, 7.4616, 7.4639, 7.4662, 7.4685, 7.4708, 7.4731, 7.4753, 7.4776, 7.4799, 7.4821, 7.4844, 7.4866, 7.4889, 7.4911, 7.4933, 7.4955, 7.4978, 7.5, 7.5022, 7.5044, 7.5066, 7.5088, 7.511, 7.5132, 7.5153, 7.5175, 7.5197, 7.5219, 7.524, 7.5262, 7.5283, 7.5305, 7.5326, 7.5348, 7.5369, 7.539, 7.5412, 7.5433, 7.5454, 7.5475, 7.5496, 7.5517, 7.5538, 7.5559, 7.558, 7.5601, 7.5622, 7.5642, 7.5663, 7.5684, 7.5704, 7.5725, 7.5746, 7.5766, 7.5787, 7.5807, 7.5827, 7.5848, 7.5868, 7.5888, 7.5909, 7.5929, 7.5949, 7.5969, 7.5989, 7.6009, 7.6029, 7.6049, 7.6069, 7.6089, 7.6109, 7.6128, 7.6148, 7.6168, 7.6187, 7.6207, 7.6227, 7.6246, 7.6266, 7.6285, 7.6305, 7.6324, 7.6343, 7.6363, 7.6382, 7.6401, 7.642, 7.644, 7.6459, 7.6478, 7.6497, 7.6516, 7.6535, 7.6554, 7.6573, 7.6592, 7.6611, 7.6629, 7.6648, 7.6667, 7.6686, 7.6704, 7.6723, 7.6742, 7.676, 7.6779, 7.6797, 7.6816, 7.6834, 7.6852, 7.6871, 7.6889, 7.6907, 7.6926, 7.6944, 7.6962, 7.698, 7.6998, 7.7017, 7.7035, 7.7053, 7.7071, 7.7089, 7.7107, 7.7124, 7.7142, 7.716, 7.7178, 7.7196, 7.7213, 7.7231, 7.7249, 7.7267, 7.7284, 7.7302, 7.7319, 7.7337, 7.7354, 7.7372, 7.7389, 7.7407, 7.7424, 7.7441, 7.7459, 7.7476, 7.7493, 7.751, 7.7528, 7.7545, 7.7562, 7.7579, 7.7596, 7.7613, 7.763, 7.7647, 7.7664, 7.7681, 7.7698, 7.7715, 7.7732, 7.7749, 7.7765, 7.7782, 7.7799, 7.7816, 7.7832, 7.7849, 7.7866, 7.7882, 7.7899, 7.7915, 7.7932, 7.7948, 7.7965, 7.7981, 7.7998, 7.8014, 7.803, 7.8047, 7.8063, 7.8079, 7.8095, 7.8112, 7.8128, 7.8144, 7.816, 7.8176, 7.8192, 7.8208, 7.8224, 7.824, 7.8256, 7.8272, 7.8288, 7.8304, 7.832, 7.8336, 7.8352, 7.8368, 7.8383, 7.8399, 7.8415, 7.8431, 7.8446, 7.8462, 7.8478, 7.8493, 7.8509, 7.8524, 7.854, 7.8555, 7.8571, 7.8586, 7.8602, 7.8617, 7.8633, 7.8648, 7.8663, 7.8679, 7.8694, 7.8709, 7.8725, 7.874, 7.8755, 7.877, 7.8785, 7.88, 7.8816, 7.8831, 7.8846, 7.8861, 7.8876, 7.8891, 7.8906, 7.8921, 7.8936, 7.8951, 7.8966, 7.898, 7.8995, 7.901, 7.9025, 7.904, 7.9054, 7.9069, 7.9084, 7.9099, 7.9113, 7.9128, 7.9143, 7.9157, 7.9172, 7.9186, 7.9201, 7.9215, 7.923, 7.9244, 7.9259, 7.9273, 7.9288, 7.9302, 7.9316, 7.9331, 7.9345, 7.9359, 7.9374, 7.9388, 7.9402, 7.9417, 7.9431, 7.9445, 7.9459, 7.9473, 7.9487, 7.9501, 7.9516, 7.953, 7.9544, 7.9558, 7.9572, 7.9586, 7.96, 7.9614, 7.9628, 7.9642, 7.9655, 7.9669, 7.9683, 7.9697, 7.9711, 7.9725, 7.9738, 7.9752, 7.9766, 7.978, 7.9793, 7.9807, 7.9821, 7.9834, 7.9848, 7.9862, 7.9875, 7.9889, 7.9902, 7.9916, 7.9929, 7.9943, 7.9956, 7.997, 7.9983, 7.9997, 8.001, 8.0024, 8.0037, 8.005, 8.0064, 8.0077, 8.009, 8.0104, 8.0117, 8.013, 8.0143, 8.0157, 8.017, 8.0183, 8.0196, 8.0209, 8.0222, 8.0236, 8.0249, 8.0262, 8.0275, 8.0288, 8.0301, 8.0314, 8.0327, 8.034, 8.0353, 8.0366, 8.0379, 8.0392, 8.0404, 8.0417, 8.043, 8.0443, 8.0456, 8.0469, 8.0481, 8.0494, 8.0507, 8.052, 8.0533, 8.0545, 8.0558, 8.0571, 8.0583, 8.0596, 8.0609, 8.0621, 8.0634, 8.0646, 8.0659, 8.0671, 8.0684, 8.0697, 8.0709, 8.0722, 8.0734, 8.0746, 8.0759, 8.0771, 8.0784, 8.0796, 8.0809, 8.0821, 8.0833, 8.0846, 8.0858, 8.087, 8.0883, 8.0895, 8.0907, 8.0919, 8.0932, 8.0944, 8.0956, 8.0968, 8.098, 8.0993, 8.1005, 8.1017, 8.1029, 8.1041, 8.1053, 8.1065, 8.1077, 8.1089, 8.1101, 8.1113, 8.1125, 8.1137, 8.1149, 8.1161, 8.1173, 8.1185, 8.1197, 8.1209, 8.1221, 8.1233, 8.1244, 8.1256, 8.1268, 8.128, 8.1292, 8.1304, 8.1315, 8.1327, 8.1339, 8.1351, 8.1362, 8.1374, 8.1386, 8.1397, 8.1409, 8.1421, 8.1432, 8.1444, 8.1455, 8.1467, 8.1479, 8.149, 8.1502, 8.1513, 8.1525, 8.1536, 8.1548, 8.1559, 8.1571, 8.1582, 8.1594, 8.1605, 8.1617, 8.1628, 8.1639, 8.1651, 8.1662, 8.1674, 8.1685, 8.1696, 8.1708, 8.1719, 8.173, 8.1741, 8.1753, 8.1764, 8.1775, 8.1786, 8.1798, 8.1809, 8.182, 8.1831, 8.1842, 8.1854, 8.1865, 8.1876, 8.1887, 8.1898, 8.1909, 8.192, 8.1931, 8.1942, 8.1953, 8.1964, 8.1975, 8.1986, 8.1997, 8.2008, 8.2019, 8.203, 8.2041, 8.2052, 8.2063, 8.2074, 8.2085, 8.2096, 8.2107, 8.2118, 8.2128, 8.2139, 8.215, 8.2161, 8.2172, 8.2182, 8.2193, 8.2204, 8.2215, 8.2226, 8.2236, 8.2247, 8.2258, 8.2268, 8.2279, 8.229, 8.23, 8.2311, 8.2322, 8.2332, 8.2343, 8.2354, 8.2364, 8.2375, 8.2385, 8.2396, 8.2406, 8.2417, 8.2428, 8.2438, 8.2449, 8.2459, 8.247, 8.248, 8.2491, 8.2501, 8.2511, 8.2522, 8.2532, 8.2543, 8.2553, 8.2563, 8.2574, 8.2584, 8.2595, 8.2605, 8.2615, 8.2626, 8.2636, 8.2646, 8.2657, 8.2667, 8.2677, 8.2687, 8.2698, 8.2708, 8.2718, 8.2728, 8.2738, 8.2749, 8.2759, 8.2769, 8.2779, 8.2789, 8.28, 8.281, 8.282, 8.283, 8.284, 8.285, 8.286, 8.287, 8.288, 8.289, 8.29, 8.291, 8.292, 8.293, 8.294, 8.295, 8.296, 8.297, 8.298, 8.299, 8.3, 8.301, 8.302, 8.303, 8.304, 8.305, 8.306, 8.307, 8.308, 8.3089, 8.3099, 8.3109, 8.3119, 8.3129, 8.3139, 8.3148, 8.3158, 8.3168};

volatile uint16_t sensors_filtered[NUM_SENSORES];
volatile uint16_t sensors_linearized[NUM_SENSORES];
volatile uint16_t sensors_distance[NUM_SENSORES];
int16_t sensors_distance_offset[NUM_SENSORES] = {0, 0, 0, 0};

static volatile int16_t last_front_sensors_angle_error = 0;

uint8_t *get_aux_adc_channels(void) {
  return aux_adc_channels;
}

uint8_t get_aux_adc_channels_num(void) {
  return NUM_AUX_ADC_CHANNELS;
}

volatile uint16_t *get_aux_adc_raw(void) {
  return aux_adc_raw;
}

uint16_t get_aux_raw(uint8_t pos) {
  return aux_adc_raw[pos];
}

/**
 * @brief Set an specific emitter ON.
 *
 * @param[in] emitter Emitter type.
 */
static void set_emitter_on(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_FRONT_LEFT_WALL_ID:
      gpio_set(GPIOA, GPIO0);
      break;
    case SENSOR_FRONT_RIGHT_WALL_ID:
      gpio_set(GPIOA, GPIO3);
      break;
    case SENSOR_SIDE_RIGHT_WALL_ID:
      gpio_set(GPIOA, GPIO1);
      break;
    case SENSOR_SIDE_LEFT_WALL_ID:
      gpio_set(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}

/**
 * @brief Set an specific emitter OFF.
 *
 * @param[in] emitter Emitter type.
 */
static void set_emitter_off(uint8_t emitter) {
  switch (emitter) {
    case SENSOR_FRONT_LEFT_WALL_ID:
      gpio_clear(GPIOA, GPIO0);
      break;
    case SENSOR_FRONT_RIGHT_WALL_ID:
      gpio_clear(GPIOA, GPIO3);
      break;
    case SENSOR_SIDE_RIGHT_WALL_ID:
      gpio_clear(GPIOA, GPIO1);
      break;
    case SENSOR_SIDE_LEFT_WALL_ID:
      gpio_clear(GPIOA, GPIO2);
      break;
    default:
      break;
  }
}

void get_sensors_raw(uint16_t *on, uint16_t *off) {
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    on[i] = sensors_on[i];
    off[i] = sensors_off[i];
  }
}

uint8_t *get_sensors(void) {
  return sensores;
}

uint8_t get_sensors_num(void) {
  return NUM_SENSORES;
}

/**
 * @brief Máquina de estados de valores de sensores
 *
 */
void sm_emitter_adc(void) {
  gpio_clear(GPIOA, GPIO0);
  gpio_clear(GPIOA, GPIO3);
  gpio_clear(GPIOA, GPIO1);
  gpio_clear(GPIOA, GPIO2);
  return;

  static uint8_t emitter_status = 1;
  static uint8_t sensor_index = SENSOR_FRONT_LEFT_WALL_ID;

  switch (emitter_status) {
    case 1:
      sensors_off[sensor_index] = adc_read_injected(ADC2, (sensor_index + 1));
      set_emitter_on(sensor_index);
      emitter_status = 2;
      break;
    case 2:
      adc_start_conversion_injected(ADC2);
      emitter_status = 3;
      break;
    case 3:
      sensors_on[sensor_index] = adc_read_injected(ADC2, (sensor_index + 1));
      set_emitter_off(sensor_index);
      emitter_status = 4;
      break;
    case 4:
      adc_start_conversion_injected(ADC2);
      emitter_status = 1;
      if (sensor_index == (NUM_SENSORES - 1))
        sensor_index = 0;
      else
        sensor_index++;
      break;
    default:
      break;
  }
}

uint16_t get_sensor_raw(uint8_t pos, bool on) {
  if (pos < NUM_SENSORES) {
    return on ? sensors_on[pos] : sensors_off[pos];
  } else {
    return 0;
  }
}

uint16_t get_sensor_raw_filter(uint8_t pos) {
  if (pos < NUM_SENSORES) {
    if (sensors_on[pos] > sensors_off[pos]) {
      return sensors_on[pos] - sensors_off[pos];
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

void front_sensors_calibration(void) {

  uint16_t distance_left;
  uint16_t distance_right;
  bool done_left;
  bool done_right;

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  sensors_distance_offset[SENSOR_FRONT_LEFT_WALL_ID] = 0;
  sensors_distance_offset[SENSOR_FRONT_RIGHT_WALL_ID] = 0;

  front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset = 0;
  front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset = 0;
  front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset = 0;
  front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset = 0;

  // configure_kinematics(SPEED_EXPLORE);
  // set_race_started(true);
  // move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_FRONT_LENGTH, -100, false, true);
  // delay(1000);
  // set_race_started(false);

  while (!get_menu_mode_btn()) {
    blink_RGB_color(50, 0, 50, 200);
    while (get_menu_mode_btn()) {
      blink_RGB_color(50, 0, 50, 200);
    }
  }
  delay(1000);

  done_left = false;
  done_right = false;
  while (!done_left || !done_right) {
    set_RGB_rainbow();
    set_info_led(0, !done_left);
    set_info_led(7, !done_right);
    if (!done_left) {
      distance_left = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID];
      for (uint8_t i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
        delay(2);
        distance_left = (distance_left + sensors_distance[SENSOR_FRONT_LEFT_WALL_ID]) / 2;
      }
      int16_t diff_left = distance_left - (CELL_DIMENSION - WALL_WIDTH) / 2;
      if (diff_left < -1) {
        front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset -= 5;
      } else if (diff_left > 1) {
        front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset += 5;
      } else {
        done_left = true;
      }
      // printf("L: %d %d\n", distance_left, front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset);
    }
    if (!done_right) {
      distance_right = sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
      for (uint8_t i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
        delay(2);
        distance_right = (distance_right + sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID]) / 2;
      }
      int16_t diff_right = distance_right - (CELL_DIMENSION - WALL_WIDTH) / 2;
      if (diff_right < -1) {
        front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset -= 5;
      } else if (diff_right > 1) {
        front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset += 5;
      } else {
        done_right = true;
      }
      // printf("R: %d %d\n", distance_right, front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset);
    }
  }
  set_info_led(0, !done_left);
  set_info_led(7, !done_right);
  set_RGB_color(0, 0, 0);

  // set_race_started(true);
  // move_straight(CELL_DIMENSION / 2, -100, false, true);
  // delay(1000);
  // set_race_started(false);
  while (!get_menu_mode_btn()) {
    blink_RGB_color(50, 0, 50, 200);
    while (get_menu_mode_btn()) {
      blink_RGB_color(50, 0, 50, 200);
    }
  }
  delay(1000);

  done_left = false;
  done_right = false;
  while (!done_left || !done_right) {
    set_RGB_rainbow();
    set_info_led(0, !done_left);
    set_info_led(7, !done_right);
    if (!done_left) {
      set_info_led(0, true);
      distance_left = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID];
      for (uint8_t i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
        delay(2);
        distance_left = (distance_left + sensors_distance[SENSOR_FRONT_LEFT_WALL_ID]) / 2;
      }
      int16_t diff_left = distance_left - (CELL_DIMENSION - WALL_WIDTH / 2);
      if (diff_left < 0) {
        front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset -= 5;
      } else if (diff_left > 0) {
        front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset += 5;
      } else {
        set_info_led(0, false);
        done_left = true;
      }
      // printf("L: %d %d\n", distance_left, front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset);
    }
    if (!done_right) {
      set_info_led(7, true);
      distance_right = sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
      for (uint8_t i = 0; i < SENSOR_FRONT_CALIBRATION_READINGS; i++) {
        delay(2);
        distance_right = (distance_right + sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID]) / 2;
      }
      int16_t diff_right = distance_right - (CELL_DIMENSION - WALL_WIDTH / 2);
      if (diff_right < 0) {
        front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset -= 5;
      } else if (diff_right > 0) {
        front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset += 5;
      } else {
        set_info_led(7, false);
        done_right = true;
      }
      // printf("R: %d %d\n", distance_right, front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset);
    }
  }
  set_info_led(0, !done_left);
  set_info_led(7, !done_right);
  set_RGB_color(0, 0, 0);

  int16_t eeprom_data[4] = {
      front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset,
      front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset,
      front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset,
      front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset,
  };
  eeprom_set_data(DATA_INDEX_FRONT_SENSORS_CALIBRATION, eeprom_data, NUM_SENSORES);
  eeprom_set_data(DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset, NUM_SENSORES);
}

void side_sensors_calibration(void) {
  int32_t left_temp = 0;
  int16_t left_offset = 0;
  int32_t right_temp = 0;
  int16_t right_offset = 0;
  int i;

  sensors_distance_offset[SENSOR_SIDE_LEFT_WALL_ID] = 0;
  sensors_distance_offset[SENSOR_SIDE_RIGHT_WALL_ID] = 0;
  delay(5);

  for (i = 0; i < SENSOR_SIDE_CALIBRATION_READINGS; i++) {
    left_temp += sensors_distance[SENSOR_SIDE_LEFT_WALL_ID];
    right_temp += sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID];
    set_leds_wave(35);
    delay(5);
  }
  set_info_leds();
  left_offset = (int16_t)((left_temp / SENSOR_SIDE_CALIBRATION_READINGS) - MIDDLE_MAZE_DISTANCE);
  right_offset = (int16_t)((right_temp / SENSOR_SIDE_CALIBRATION_READINGS) - MIDDLE_MAZE_DISTANCE);
  // printf("l: %d r: %d\n", left_offset, right_offset);
  sensors_distance_offset[SENSOR_SIDE_LEFT_WALL_ID] = left_offset;
  sensors_distance_offset[SENSOR_SIDE_RIGHT_WALL_ID] = right_offset;

  delay(500);
  clear_info_leds();
  eeprom_set_data(DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset, NUM_SENSORES);
}

void sensors_load_eeprom(void) {
  int16_t *data = eeprom_get_data();
  if (data != NULL) {
    for (uint8_t i = DATA_INDEX_SENSORS_OFFSETS; i < (DATA_INDEX_SENSORS_OFFSETS + NUM_SENSORES); i++) {
      if (data[i] != 0) {
        sensors_distance_offset[i - DATA_INDEX_SENSORS_OFFSETS] = data[i];
        // printf("sensor %d offset: %d\n", i - DATA_INDEX_SENSORS_OFFSETS, sensors_distance_offset[i - DATA_INDEX_SENSORS_OFFSETS]);
      }
    }
    {
      uint8_t i = DATA_INDEX_FRONT_SENSORS_CALIBRATION;
      front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].close_offset = data[i++];
      front_sensors_distance_calibrations[SENSOR_FRONT_LEFT_WALL_ID].far_offset = data[i++];
      front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].close_offset = data[i++];
      front_sensors_distance_calibrations[SENSOR_FRONT_RIGHT_WALL_ID].far_offset = data[i++];
    }
  }
}

bool left_wall_detection(void) {
  return sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] < SENSOR_SIDE_DETECTION;
}

bool right_wall_detection(void) {
  return sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] < SENSOR_SIDE_DETECTION;
}

bool front_wall_detection(void) {
  return (sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] < SENSOR_FRONT_DETECTION) ||
         (sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID] < SENSOR_FRONT_DETECTION);
}

/**
 * @brief Aplica las magias necesarias para obtener valores correctos de los sensores.
 * · Resta el valor de offset a las lecturas en caso necesario.
 * · Aplica un filtro pasa bajo a los valores de los sensores.
 * · Calcula el valor linealizado de los sensores.
 * · Obtiene la distancia de las paredes en mm
 *
 */
void update_sensors_magics(void) {
  for (uint8_t sensor = 0; sensor < NUM_SENSORES; sensor++) {
    if (sensors_on[sensor] > sensors_off[sensor]) {
      sensors_raw[sensor] = sensors_on[sensor] - sensors_off[sensor];

      uint16_t new_sensor_distance = 0;
      if (sensor == SENSOR_FRONT_LEFT_WALL_ID || sensor == SENSOR_FRONT_RIGHT_WALL_ID) {
        // printf("%d: ", sensor);

        struct front_sensors_distance_calibration front_sensor_distance_calibration = front_sensors_distance_calibrations[sensor];

        uint16_t front_sensor_before = sensors_filtered[sensor];
        uint16_t front_ln_index = 0;
        uint16_t front_linearized_value = 0;

        if (sensors_raw[sensor] + front_sensor_distance_calibration.close_offset >= front_sensor_distance_calibration.close_low_raw && sensors_raw[sensor] + front_sensor_distance_calibration.close_offset <= front_sensor_distance_calibration.close_high_raw) {
          sensors_raw[sensor] += front_sensor_distance_calibration.close_offset;
          sensors_filtered[sensor] = SENSOR_LOW_PASS_FILTER_ALPHA * sensors_raw[sensor] + (1 - SENSOR_LOW_PASS_FILTER_ALPHA) * front_sensor_before;
          front_ln_index = sensors_filtered[sensor] / LOG_LINEARIZATION_TABLE_STEP;
          front_linearized_value = (uint16_t)(ln_linearization[front_ln_index] * 1000);
          sensors_linearized[sensor] = front_linearized_value;
          new_sensor_distance = ((uint16_t)((front_sensor_distance_calibration.close_slope * front_linearized_value) + front_sensor_distance_calibration.close_intercept)) + ROBOT_FRONT_LENGTH - sensors_distance_offset[sensor];

        } else if (sensors_raw[sensor] + front_sensor_distance_calibration.far_offset >= front_sensor_distance_calibration.far_low_raw && sensors_raw[sensor] + front_sensor_distance_calibration.far_offset <= front_sensor_distance_calibration.far_high_raw) {
          sensors_raw[sensor] += front_sensor_distance_calibration.far_offset;
          sensors_filtered[sensor] = SENSOR_LOW_PASS_FILTER_ALPHA * sensors_raw[sensor] + (1 - SENSOR_LOW_PASS_FILTER_ALPHA) * front_sensor_before;
          front_ln_index = sensors_filtered[sensor] / LOG_LINEARIZATION_TABLE_STEP;
          front_linearized_value = (uint16_t)(ln_linearization[front_ln_index] * 1000);
          sensors_linearized[sensor] = front_linearized_value;
          new_sensor_distance = ((uint16_t)((front_sensor_distance_calibration.far_slope * front_linearized_value) + front_sensor_distance_calibration.far_intercept)) + ROBOT_FRONT_LENGTH - sensors_distance_offset[sensor];

        } else {
          sensors_raw[sensor] += front_sensor_distance_calibration.offset;
          sensors_filtered[sensor] = SENSOR_LOW_PASS_FILTER_ALPHA * sensors_raw[sensor] + (1 - SENSOR_LOW_PASS_FILTER_ALPHA) * front_sensor_before;
          front_ln_index = sensors_filtered[sensor] / LOG_LINEARIZATION_TABLE_STEP;
          front_linearized_value = (uint16_t)(ln_linearization[front_ln_index] * 1000);
          sensors_linearized[sensor] = front_linearized_value;
          new_sensor_distance = ((uint16_t)((front_sensor_distance_calibration.slope * front_linearized_value) + front_sensor_distance_calibration.intercept)) + ROBOT_FRONT_LENGTH - sensors_distance_offset[sensor];
        }

      } else if (sensor == SENSOR_SIDE_LEFT_WALL_ID || sensor == SENSOR_SIDE_RIGHT_WALL_ID) {
        struct side_sensors_distance_calibration side_sensor_distance_calibration = side_sensors_distance_calibrations[sensor];

        uint16_t side_sensor_before = sensors_filtered[sensor];
        uint16_t side_ln_index = 0;
        uint16_t side_linearized_value = 0;

        sensors_raw[sensor] += side_sensor_distance_calibration.offset;
        sensors_filtered[sensor] = SENSOR_LOW_PASS_FILTER_ALPHA * sensors_raw[sensor] + (1 - SENSOR_LOW_PASS_FILTER_ALPHA) * side_sensor_before;
        side_ln_index = sensors_filtered[sensor] / LOG_LINEARIZATION_TABLE_STEP;
        if (side_ln_index == 0) {
          side_ln_index = 1;
        }
        side_linearized_value = (uint16_t)(ln_linearization[side_ln_index] * 1000);
        sensors_linearized[sensor] = side_linearized_value;
        new_sensor_distance = ((uint16_t)((side_sensor_distance_calibration.slope * side_linearized_value) + side_sensor_distance_calibration.intercept)) + ROBOT_MIDDLE_WIDTH - sensors_distance_offset[sensor];
      }

      sensors_distance[sensor] = 0.1f * new_sensor_distance + (1 - 0.1f) * sensors_distance[sensor];
    }
  }
}

void update_side_sensors_leds(void) {
  int16_t side_error_leds = get_side_sensors_close_error();
  if (abs(side_error_leds) < 2) {
    clear_info_leds();
  } else if (side_error_leds >= 20) {
    set_info_led(0, true);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 10) {
    set_info_led(0, false);
    set_info_led(1, true);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 5) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, true);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds >= 0) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, true);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds <= -20) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, true);
  } else if (side_error_leds <= -10) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, false);
    set_info_led(6, true);
    set_info_led(7, false);
  } else if (side_error_leds <= -5) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, false);
    set_info_led(5, true);
    set_info_led(6, false);
    set_info_led(7, false);
  } else if (side_error_leds <= 0) {
    set_info_led(0, false);
    set_info_led(1, false);
    set_info_led(2, false);
    set_info_led(3, false);
    set_info_led(4, true);
    set_info_led(5, false);
    set_info_led(6, false);
    set_info_led(7, false);
  }
}

uint16_t get_sensor_filtered(uint8_t pos) {
  return sensors_filtered[pos];
}

uint16_t get_sensor_linearized(uint8_t pos) {
  return sensors_linearized[pos];
}

uint16_t get_sensor_distance(uint8_t pos) {
  return sensors_distance[pos];
}

uint16_t get_front_wall_distance(void) {
  return (sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] + sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID]) / 2;
}

struct walls get_walls(void) {
  struct walls walls;
  walls.front = front_wall_detection();
  walls.left = left_wall_detection();
  walls.right = right_wall_detection();
  return walls;
}

int16_t get_side_sensors_close_error(void) {
  int16_t left_error = sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] - MIDDLE_MAZE_DISTANCE;
  int16_t right_error = sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] - MIDDLE_MAZE_DISTANCE;
  if (left_error > 0 && right_error < 0) {
    return right_error;
  } else if (right_error > 0 && left_error < 0) {
    return -left_error;
  }
  return 0;
}

int16_t get_side_sensors_far_error(void) {
  int16_t left_error = sensors_distance[SENSOR_SIDE_LEFT_WALL_ID] - MIDDLE_MAZE_DISTANCE;
  int16_t right_error = sensors_distance[SENSOR_SIDE_RIGHT_WALL_ID] - MIDDLE_MAZE_DISTANCE;
  // printf("\t\t%4d - %4d\n", left_error, right_error);

  if ((left_error > 70) && (right_error < 50)) {
    return right_error;
  }
  if ((right_error > 70) && (left_error < 50)) {
    return -left_error;
  }

  return 0;
}

int16_t get_front_sensors_angle_error(void) {
  if (!front_wall_detection()) {
    last_front_sensors_angle_error = 0;
    return 0;
  }
  int16_t error = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] - sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID];
  // error = 0.1 * error + (1 - 0.1) * last_front_sensors_angle_error;
  // last_front_sensors_angle_error = error;
  return abs(error) < 2 ? 0 : error;
}

int16_t get_front_sensors_diagonal_error(void) {
  int16_t left_error = sensors_distance[SENSOR_FRONT_LEFT_WALL_ID] - 385;
  int16_t right_error = sensors_distance[SENSOR_FRONT_RIGHT_WALL_ID] - 385;
  // printf("\t\t%4d - %4d\n", left_error, right_error);

  if (right_error < 0) {
    return right_error;
  }
  if (left_error < 0) {
    return -left_error;
  }

  return 0;
}
