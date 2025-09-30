#include "move.h"

static char *movement_string[] = {
    "MOVE_NONE",
    "MOVE_HOME",
    "MOVE_START",
    "MOVE_END",
    "MOVE_FRONT",
    "MOVE_LEFT",
    "MOVE_RIGHT",
    "MOVE_LEFT_90",
    "MOVE_RIGHT_90",
    "MOVE_LEFT_180",
    "MOVE_RIGHT_180",
    "MOVE_DIAGONAL",
    "MOVE_LEFT_TO_45",
    "MOVE_RIGHT_TO_45",
    "MOVE_LEFT_TO_135",
    "MOVE_RIGHT_TO_135",
    "MOVE_LEFT_45_TO_45",
    "MOVE_RIGHT_45_TO_45",
    "MOVE_LEFT_FROM_45",
    "MOVE_RIGHT_FROM_45",
    "MOVE_LEFT_FROM_45_180",
    "MOVE_RIGHT_FROM_45_180",
    "MOVE_BACK",
    "MOVE_BACK_WALL",
    "MOVE_BACK_STOP",
};

//! EL COLAB CALCULA MAL LOS SIGUIENTES GIROS:
//! MOVE_****_180 -> El .end tiene que ser igual que el .start
//! MOVE_****_TO_135 -> El .end tiene que ser el .start y el .start tiene que ser el .end del MOVE_****_FROM_45_180

static struct inplace_params turns_inplace[] = {
    [MOVE_BACK] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 13.460,
        .t_accel = 22,
        .t_max = 209,
        .sign = -1,
    },
    [MOVE_BACK_WALL] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 13.460,
        .t_accel = 22,
        .t_max = 209,
        .sign = -1,
    },
    [MOVE_BACK_STOP] = {
        .start = 0,
        .end = 0,
        .linear_speed = 0,
        .angular_accel = 612.5,
        .max_angular_speed = 13.460,
        .t_accel = 22,
        .t_max = 209,
        .sign = -1,
    },
};

static struct turn_params turns_explore[] = {
    [MOVE_LEFT] = {
        .start = 10.1633,
        .end = 10.1581,
        .linear_speed = 650,
        .max_angular_speed = 11.8182,
        .transition = 63.3458,
        .arc = 5.7460,
        .sign = -1,
    },
    [MOVE_RIGHT] = {
        .start = 10.1633,
        .end = 10.1581,
        .linear_speed = 650,
        .max_angular_speed = 11.8182,
        .transition = 63.3458,
        .arc = 5.7460,
        .sign = 1,
    },
};

static struct turn_params turns_normal[] = {
    [MOVE_LEFT_90] = {
        .start = -68.8257,
        .end = -68.8336,
        .linear_speed = 1696,
        .max_angular_speed = 12.5630,
        .transition = 63.3541,
        .arc = 131.4061,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = -68.8257,
        .end = -68.8336,
        .linear_speed = 1696,
        .max_angular_speed = 12.5630,
        .transition = 63.3541,
        .arc = 131.4061,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = -44.9865,
        .end = -44.9865,
        .linear_speed = 1350,
        .max_angular_speed = 15.0000,
        .transition = 63.3488,
        .arc = 202.0950,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = -44.9865,
        .end = -44.9865,
        .linear_speed = 1350,
        .max_angular_speed = 15.0000,
        .transition = 63.3488,
        .arc = 202.0950,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -73.0659,
        .end = 54.2105,
        .linear_speed = 1644,
        .max_angular_speed = 13.7000,
        .transition = 63.3433,
        .arc = 13.6123,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -73.0659,
        .end = 54.2105,
        .linear_speed = 1644,
        .max_angular_speed = 13.7000,
        .transition = 63.3433,
        .arc = 13.6123,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = -26.5873,
        .end = 47.9719,
        .linear_speed = 1118,
        .max_angular_speed = 14.9946,
        .transition = 63.3459,
        .arc = 95.0300,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = -26.5873,
        .end = 47.9719,
        .linear_speed = 1118,
        .max_angular_speed = 14.9946,
        .transition = 63.3459,
        .arc = 95.0300,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0126,
        .linear_speed = 955,
        .max_angular_speed = 15.0063,
        .transition = 63.3452,
        .arc = 19.3196,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0126,
        .linear_speed = 955,
        .max_angular_speed = 15.0063,
        .transition = 63.3452,
        .arc = 19.3196,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0687,
        .linear_speed = 1644,
        .max_angular_speed = 13.7000,
        .transition = 63.3433,
        .arc = 13.6123,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0687,
        .linear_speed = 1644,
        .max_angular_speed = 13.7000,
        .transition = 63.3433,
        .arc = 13.6123,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5873,
        .linear_speed = 1118,
        .max_angular_speed = 14.9946,
        .transition = 63.3459,
        .arc = 95.0300,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5873,
        .linear_speed = 1118,
        .max_angular_speed = 14.9946,
        .transition = 63.3459,
        .arc = 95.0300,
        .sign = 1,
    },
};

static struct turn_params turns_medium[] = {
    [MOVE_LEFT_90] = {
        .start = -68.8257,
        .end = -68.8322,
        .linear_speed = 1932,
        .max_angular_speed = 14.3111,
        .transition = 63.3503,
        .arc = 131.4146,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = -68.8257,
        .end = -68.8322,
        .linear_speed = 1932,
        .max_angular_speed = 14.3111,
        .transition = 63.3503,
        .arc = 131.4146,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = -44.9842,
        .end = -44.9842,
        .linear_speed = 1575,
        .max_angular_speed = 17.5000,
        .transition = 63.3465,
        .arc = 202.1040,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = -44.9842,
        .end = -44.9842,
        .linear_speed = 1575,
        .max_angular_speed = 17.5000,
        .transition = 63.3465,
        .arc = 202.1040,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -73.0659,
        .end = 54.2077,
        .linear_speed = 1865,
        .max_angular_speed = 15.5417,
        .transition = 63.3541,
        .arc = 13.5959,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -73.0659,
        .end = 54.2077,
        .linear_speed = 1865,
        .max_angular_speed = 15.5417,
        .transition = 63.3541,
        .arc = 13.5959,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = -26.5962,
        .end = 47.9719,
        .linear_speed = 1305,
        .max_angular_speed = 17.5027,
        .transition = 63.3512,
        .arc = 95.0301,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = -26.5962,
        .end = 47.9719,
        .linear_speed = 1305,
        .max_angular_speed = 17.5027,
        .transition = 63.3512,
        .arc = 95.0301,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0122,
        .linear_speed = 1114,
        .max_angular_speed = 17.5047,
        .transition = 63.3420,
        .arc = 19.3279,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0122,
        .linear_speed = 1114,
        .max_angular_speed = 17.5047,
        .transition = 63.3420,
        .arc = 19.3279,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0715,
        .linear_speed = 1865,
        .max_angular_speed = 15.5417,
        .transition = 63.3541,
        .arc = 13.5959,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0715,
        .linear_speed = 1865,
        .max_angular_speed = 15.5417,
        .transition = 63.3541,
        .arc = 13.5959,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5962,
        .linear_speed = 1305,
        .max_angular_speed = 17.5027,
        .transition = 63.3512,
        .arc = 95.0301,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5962,
        .linear_speed = 1305,
        .max_angular_speed = 17.5027,
        .transition = 63.3512,
        .arc = 95.0301,
        .sign = 1,
    },
};

static struct turn_params turns_fast[] = {
    [MOVE_LEFT_90] = {
        .start = -68.8257,
        .end = -68.8335,
        .linear_speed = 2220,
        .max_angular_speed = 16.4444,
        .transition = 63.3588,
        .arc = 131.4018,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = -68.8257,
        .end = -68.8335,
        .linear_speed = 2220,
        .max_angular_speed = 16.4444,
        .transition = 63.3588,
        .arc = 131.4018,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = -44.9820,
        .end = -44.9820,
        .linear_speed = 1800,
        .max_angular_speed = 20.0000,
        .transition = 63.3510,
        .arc = 202.1040,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = -44.9820,
        .end = -44.9820,
        .linear_speed = 1800,
        .max_angular_speed = 20.0000,
        .transition = 63.3510,
        .arc = 202.1040,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -73.0659,
        .end = 54.2090,
        .linear_speed = 2146,
        .max_angular_speed = 17.8833,
        .transition = 63.3499,
        .arc = 13.6056,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -73.0659,
        .end = 54.2090,
        .linear_speed = 2146,
        .max_angular_speed = 17.8833,
        .transition = 63.3499,
        .arc = 13.6056,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = -26.5884,
        .end = 47.9719,
        .linear_speed = 1491,
        .max_angular_speed = 19.9973,
        .transition = 63.3526,
        .arc = 95.0214,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = -26.5884,
        .end = 47.9719,
        .linear_speed = 1491,
        .max_angular_speed = 19.9973,
        .transition = 63.3526,
        .arc = 95.0214,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0126,
        .linear_speed = 1273,
        .max_angular_speed = 20.0031,
        .transition = 63.3445,
        .arc = 19.3241,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0126,
        .linear_speed = 1273,
        .max_angular_speed = 20.0031,
        .transition = 63.3445,
        .arc = 19.3241,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0702,
        .linear_speed = 2146,
        .max_angular_speed = 17.8833,
        .transition = 63.3499,
        .arc = 13.6056,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0702,
        .linear_speed = 2146,
        .max_angular_speed = 17.8833,
        .transition = 63.3499,
        .arc = 13.6056,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5884,
        .linear_speed = 1491,
        .max_angular_speed = 19.9973,
        .transition = 63.3526,
        .arc = 95.0214,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5884,
        .linear_speed = 1491,
        .max_angular_speed = 19.9973,
        .transition = 63.3526,
        .arc = 95.0214,
        .sign = 1,
    },
};

static struct turn_params turns_super[] = {
    [MOVE_LEFT_90] = {
        .start = -68.8257,
        .end = -68.8497,
        .linear_speed = 2490,
        .max_angular_speed = 18.4444,
        .transition = 63.3581,
        .arc = 131.4222,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = -68.8257,
        .end = -68.8497,
        .linear_speed = 2490,
        .max_angular_speed = 18.4444,
        .transition = 63.3581,
        .arc = 131.4222,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = -44.9797,
        .end = -44.9797,
        .linear_speed = 2025,
        .max_angular_speed = 22.5000,
        .transition = 63.3521,
        .arc = 202.0950,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = -44.9797,
        .end = -44.9797,
        .linear_speed = 2025,
        .max_angular_speed = 22.5000,
        .transition = 63.3521,
        .arc = 202.0950,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -73.0659,
        .end = 54.1956,
        .linear_speed = 2404,
        .max_angular_speed = 20.0333,
        .transition = 63.3574,
        .arc = 13.6066,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -73.0659,
        .end = 54.1956,
        .linear_speed = 2404,
        .max_angular_speed = 20.0333,
        .transition = 63.3574,
        .arc = 13.6066,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = -26.5908,
        .end = 47.9719,
        .linear_speed = 1678,
        .max_angular_speed = 22.5054,
        .transition = 63.3445,
        .arc = 95.0419,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = -26.5908,
        .end = 47.9719,
        .linear_speed = 1678,
        .max_angular_speed = 22.5054,
        .transition = 63.3445,
        .arc = 95.0419,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0063,
        .linear_speed = 1432,
        .max_angular_speed = 22.5016,
        .transition = 63.3517,
        .arc = 19.3177,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0063,
        .linear_speed = 1432,
        .max_angular_speed = 22.5016,
        .transition = 63.3517,
        .arc = 19.3177,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0836,
        .linear_speed = 2404,
        .max_angular_speed = 20.0333,
        .transition = 63.3574,
        .arc = 13.6066,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0836,
        .linear_speed = 2404,
        .max_angular_speed = 20.0333,
        .transition = 63.3574,
        .arc = 13.6066,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5908,
        .linear_speed = 1678,
        .max_angular_speed = 22.5054,
        .transition = 63.3445,
        .arc = 95.0419,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5908,
        .linear_speed = 1678,
        .max_angular_speed = 22.5054,
        .transition = 63.3445,
        .arc = 95.0419,
        .sign = 1,
    },
};

static struct turn_params turns_haki[] = {
    [MOVE_LEFT_90] = {
        .start = -68.8257,
        .end = -68.8486,
        .linear_speed = 2745,
        .max_angular_speed = 20.3333,
        .transition = 63.3683,
        .arc = 131.4032,
        .sign = -1,
    },
    [MOVE_RIGHT_90] = {
        .start = -68.8257,
        .end = -68.8486,
        .linear_speed = 2745,
        .max_angular_speed = 20.3333,
        .transition = 63.3683,
        .arc = 131.4032,
        .sign = 1,
    },
    [MOVE_LEFT_180] = {
        .start = -44.9775,
        .end = -44.9775,
        .linear_speed = 2250,
        .max_angular_speed = 25.0000,
        .transition = 63.3600,
        .arc = 202.0950,
        .sign = -1,
    },
    [MOVE_RIGHT_180] = {
        .start = -44.9775,
        .end = -44.9775,
        .linear_speed = 2250,
        .max_angular_speed = 25.0000,
        .transition = 63.3600,
        .arc = 202.0950,
        .sign = 1,
    },
    [MOVE_LEFT_TO_45] = {
        .start = -73.0659,
        .end = 54.2020,
        .linear_speed = 2647,
        .max_angular_speed = 22.0583,
        .transition = 63.3559,
        .arc = 13.6056,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_45] = {
        .start = -73.0659,
        .end = 54.2020,
        .linear_speed = 2647,
        .max_angular_speed = 22.0583,
        .transition = 63.3559,
        .arc = 13.6056,
        .sign = 1,
    },
    [MOVE_LEFT_TO_135] = {
        .start = -26.5995,
        .end = 47.9719,
        .linear_speed = 1864,
        .max_angular_speed = 25.0000,
        .transition = 63.3480,
        .arc = 95.0454,
        .sign = -1,
    },
    [MOVE_RIGHT_TO_135] = {
        .start = -26.5995,
        .end = 47.9719,
        .linear_speed = 1864,
        .max_angular_speed = 25.0000,
        .transition = 63.3480,
        .arc = 95.0454,
        .sign = 1,
    },
    [MOVE_LEFT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0069,
        .linear_speed = 1591,
        .max_angular_speed = 25.0000,
        .transition = 63.3536,
        .arc = 19.3147,
        .sign = -1,
    },
    [MOVE_RIGHT_45_TO_45] = {
        .start = 39.0156,
        .end = 39.0069,
        .linear_speed = 1591,
        .max_angular_speed = 25.0000,
        .transition = 63.3536,
        .arc = 19.3147,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0772,
        .linear_speed = 2647,
        .max_angular_speed = 22.0583,
        .transition = 63.3559,
        .arc = 13.6056,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45] = {
        .start = 54.2133,
        .end = -73.0772,
        .linear_speed = 2647,
        .max_angular_speed = 22.0583,
        .transition = 63.3559,
        .arc = 13.6056,
        .sign = 1,
    },
    [MOVE_LEFT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5995,
        .linear_speed = 1864,
        .max_angular_speed = 25.0000,
        .transition = 63.3480,
        .arc = 95.0454,
        .sign = -1,
    },
    [MOVE_RIGHT_FROM_45_180] = {
        .start = 47.9719,
        .end = -26.5995,
        .linear_speed = 1864,
        .max_angular_speed = 25.0000,
        .transition = 63.3480,
        .arc = 95.0454,
        .sign = 1,
    },
};

static struct kinematics kinematics_settings[] = {
    [SPEED_EXPLORE] = {
        .linear_speed = 650,
        .linear_accel = {
            .break_accel = 5000,
            .accel_hard = 5000,
            .speed_hard = 0,
            .accel_soft = 0,
        },
        .fan_speed = 30,
        .turns = turns_explore,
    },
    [SPEED_NORMAL] = {
        .linear_speed = 3000,
        .linear_accel = {
            .break_accel = 12000,
            .accel_hard = 12000,
            .speed_hard = 0,
            .accel_soft = 0,
        },
        .fan_speed = 60,
        .turns = turns_normal,
    },
    [SPEED_MEDIUM] = {
        .linear_speed = 5000,
        .linear_accel = {
            .break_accel = 15000,
            .accel_hard = 15000,
            .speed_hard = 0,
            .accel_soft = 0,
        },
        .fan_speed = 65,
        .turns = turns_medium,
    },
    [SPEED_FAST] = {
        .linear_speed = 5500,
        .linear_accel = {
            .break_accel = 20000,
            .accel_hard = 20000,
            .speed_hard = 3500,
            .accel_soft = 15000,
        },
        .fan_speed = 75,
        .turns = turns_fast,
    },
    [SPEED_SUPER] = {
        .linear_speed = 6000,
        .linear_accel = {
            .break_accel = 25000,
            .accel_hard = 25000,
            .speed_hard = 4000,
            .accel_soft = 20000,
        },
        .fan_speed = 85,
        .turns = turns_super,
    },
    [SPEED_HAKI] = {
        .linear_speed = 6500,
        .linear_accel = {
            .break_accel = 30000,
            .accel_hard = 30000,
            .speed_hard = 4500,
            .accel_soft = 25000,
        },
        .fan_speed = 90,
        .turns = turns_haki,
    }};

static struct kinematics kinematics;

static int32_t current_cell_start_mm = 0;
static bool current_cell_wall_lost = false;
static int32_t current_cell_absolute_start_mm = 0;

static bool wall_lost_toggle_state = false;
static bool cell_change_toggle_state = false;

static float calc_straight_to_speed_distance(int32_t from_speed, int32_t to_speed) {
  return abs((to_speed * to_speed - from_speed * from_speed) / (2 * kinematics.linear_accel.break_accel));
}

#ifndef MMSIM_ENABLED
static void enter_next_cell(void) {
  current_cell_start_mm = -SENSING_POINT_DISTANCE;
  current_cell_absolute_start_mm = get_encoder_avg_millimeters();
  current_cell_wall_lost = false;
  toggle_status_led();
  cell_change_toggle_state = !cell_change_toggle_state;
}
#endif

static bool check_wall_loss_correction(struct walls initial_walls) {
  if (current_cell_wall_lost) {
    return false;
  }
  struct walls current_walls = get_walls();
  bool wall_lost = false;
  if (initial_walls.left && !current_walls.left) {
    wall_lost = true;
  } else if (initial_walls.right && !current_walls.right) {
    wall_lost = true;
  } /* else {
    if (last_check_walls_loss.left) {
      count_check_walls_left++;
      if (count_check_walls_left > 10 && !current_walls.left) {
        wall_lost = true;
        count_check_walls_left = 0;
      }
    } else {
      count_check_walls_left = 0;
    }
    if (last_check_walls_loss.right) {
      count_check_walls_right++;
      if (count_check_walls_right > 10 && !current_walls.right) {
        wall_lost = true;
        count_check_walls_right = 0;
      }
    } else {
      count_check_walls_right = 0;
    }
  }
  last_check_walls_loss = current_walls; */
  if (wall_lost) {
    current_cell_wall_lost = true;
    wall_lost_toggle_state = !wall_lost_toggle_state;
  }
  return wall_lost;
}

static void move_home(void) {
#ifdef MMSIM_ENABLED
  API_moveForward();
  API_turnRight();
  API_turnRight();
  return;
#endif

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);

  struct walls initial_walls = get_walls();
  if (initial_walls.front && (!initial_walls.left || !initial_walls.right)) {
    set_side_sensors_close_correction(false);
  }

  move_straight_until_front_distance(MIDDLE_MAZE_DISTANCE, 300, true);

  disable_sensors_correction();
  move_inplace_turn(MOVE_BACK);
  reset_control_errors();
  move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, false, true);
  set_starting_position();
}

static void move_end(void) {
#ifdef MMSIM_ENABLED
  API_moveForward();
  API_turnRight();
  API_turnRight();
  return;
#endif

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(false);

  struct walls initial_walls = get_walls();
  if (initial_walls.front && (!initial_walls.left || !initial_walls.right)) {
    set_side_sensors_close_correction(false);
  }

  move_straight(MIDDLE_MAZE_DISTANCE, 300, false, true);

  disable_sensors_correction();
  move_inplace_turn(MOVE_BACK);
  reset_control_errors();
  move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH, -100, false, true);
}

/**
 * @brief Movimiento frontal relativo a la celda actual; avanza a la siguiente celda
 *
 */
static void move_front(void) {
#ifdef MMSIM_ENABLED
  API_moveForward();
#else

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  struct walls initial_walls = get_walls();
  if (initial_walls.left || initial_walls.right) {
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
  } else {
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
  }
  move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, kinematics.linear_speed, true, false);
  enter_next_cell();
#endif
}

static void move_side(enum movement movement) {
#ifdef MMSIM_ENABLED
  switch (movement) {
    case MOVE_LEFT:
      API_turnLeft();
      API_moveForward();
      break;
    case MOVE_RIGHT:
      API_turnRight();
      API_moveForward();
      break;
  }
#else

  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);

  int32_t end_distance_offset = 0;
  int32_t start_distance_offset = 0;
  bool enable_end_distance_offset = true;
  bool enable_start_distance_offset = true;

  switch (movement) {
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
    case MOVE_LEFT_FROM_45:
    case MOVE_RIGHT_FROM_45:
    case MOVE_LEFT_FROM_45_180:
    case MOVE_RIGHT_FROM_45_180:
      enable_start_distance_offset = false;
      enable_end_distance_offset = false;
      set_side_sensors_close_correction(false);
      set_side_sensors_far_correction(false);
      break;
    default:
      set_side_sensors_close_correction(false);
      set_side_sensors_far_correction(false);
      break;
  }

  struct walls walls = get_walls();
  if (enable_end_distance_offset) {
    if (kinematics.turns[movement].sign > 0) {
      if (walls.left) {
        end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID);
      }
    } else {
      if (walls.right) {
        end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID);
      }
    }
  }

  if (enable_start_distance_offset) {
    if (walls.front) {
      start_distance_offset = get_front_wall_distance() - (CELL_DIMENSION - (WALL_WIDTH / 2));
    }
  }

  if (kinematics.turns[movement].start > 0) {
    if (abs(start_distance_offset) > kinematics.turns[movement].start / 2) {
      start_distance_offset = start_distance_offset > 0 ? kinematics.turns[movement].start / 2 : -kinematics.turns[movement].start / 2;
    }
    set_RGB_color(0, 0, 255);
    move_straight(kinematics.turns[movement].start - current_cell_start_mm + start_distance_offset, kinematics.turns[movement].linear_speed, false, false);
    set_RGB_color(0, 0, 0);
  }

  disable_sensors_correction();
  // reset_control_errors(); //! Esto se había puesto por un problema en la acumulación de error según aumenta el número de giros realizados
  move_arc_turn(kinematics.turns[movement]);

  set_front_sensors_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  switch (movement) {
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
      set_front_sensors_diagonal_correction(true);
      break;
    default:
      set_front_sensors_diagonal_correction(false);
      break;
  }

  if (kinematics.turns[movement].end > 0) {
    if (abs(end_distance_offset) > kinematics.turns[movement].end / 2) {
      end_distance_offset = end_distance_offset > 0 ? kinematics.turns[movement].end / 2 : -kinematics.turns[movement].end / 2;
    }
    set_RGB_color(0, 0, 255);
    move_straight(kinematics.turns[movement].end + end_distance_offset, kinematics.turns[movement].linear_speed, false, false);
    set_RGB_color(0, 0, 0);
  }
  enter_next_cell();
#endif
}

static void move_back(enum movement movement) {
#ifdef MMSIM_ENABLED
  API_turnRight();
  API_turnRight();
  if (movement != MOVE_BACK_STOP) {
    API_moveForward();
  }
#else
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  struct walls initial_walls = get_walls();
  if (initial_walls.front) {
    move_straight(10, 300, false, false);
    current_cell_start_mm += 10;
    set_side_sensors_close_correction(false);
    set_side_sensors_far_correction(false);
    set_front_sensors_correction(true);
  }

  if (initial_walls.front) {
    move_straight_until_front_distance(MIDDLE_MAZE_DISTANCE, 300, true);
  } else {
    move_straight(MIDDLE_MAZE_DISTANCE * 1.75f - current_cell_start_mm, 300, false, true);
  }

  disable_sensors_correction();

  move_inplace_turn(movement);

  switch (movement) {
    case MOVE_BACK_WALL:
    case MOVE_BACK_STOP:
      set_check_motors_saturated_enabled(false);
      move_straight((CELL_DIMENSION - WALL_WIDTH) / 2 - ROBOT_BACK_LENGTH + 10, -100, false, true);
      set_check_motors_saturated_enabled(true);
      set_starting_position();
      break;
    case MOVE_BACK:
      move_straight((MIDDLE_MAZE_DISTANCE)-ROBOT_BACK_LENGTH, -100, false, true);
      current_cell_start_mm = (MIDDLE_MAZE_DISTANCE - ROBOT_BACK_LENGTH) / 2;
      break;
    default:
      break;
  }
  reset_control_errors(); //? Aquí sí reseteamos al estar en una "posición inicial estática"

  if (movement != MOVE_BACK_STOP) {
    set_front_sensors_correction(false);
    set_front_sensors_diagonal_correction(false);
    set_side_sensors_close_correction(true);
    set_side_sensors_far_correction(true);
    move_straight(CELL_DIMENSION - SENSING_POINT_DISTANCE - current_cell_start_mm, kinematics.linear_speed, true, false);
    enter_next_cell();
  }
#endif
}

bool get_cell_change_toggle_state(void) {
  return cell_change_toggle_state;
}

bool get_wall_lost_toggle_state(void) {
  return wall_lost_toggle_state;
}

char *get_movement_string(enum movement movement) {
  return movement_string[movement];
}

void configure_kinematics(enum speed_strategy speed) {
  kinematics = kinematics_settings[speed];
}

struct kinematics get_kinematics(void) {
  return kinematics;
}

uint16_t get_floodfill_linear_speed(void) {
  return kinematics_settings[SPEED_HAKI].turns[MOVE_LEFT_90].linear_speed;
}

uint16_t get_floodfill_max_linear_speed(void) {
  return kinematics_settings[SPEED_HAKI].linear_speed;
}

uint16_t get_floodfill_accel(void) {
  return kinematics_settings[SPEED_HAKI].linear_accel.accel_hard;
}

void set_starting_position(void) {
  current_cell_start_mm = ROBOT_BACK_LENGTH + WALL_WIDTH / 2;
}

int32_t get_current_cell_travelled_distance(void) {
#ifndef MMSIM_ENABLED
  return get_encoder_avg_millimeters() - current_cell_absolute_start_mm;
#else
  return 0;
#endif
}

/**
 * @brief Movimiento frontal paramétrico
 *
 * @param distance
 * @param speed
 * @param stop
 */
void move_straight(int32_t distance, int32_t speed, bool check_wall_loss, bool stop) {
#ifndef MMSIM_ENABLED
  int32_t current_distance = get_encoder_avg_micrometers();
  float stop_distance = 0;
  struct walls initial_walls = get_walls();
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  if (speed >= 0) {
    while (is_race_started() && !is_motor_saturated() && get_encoder_avg_micrometers() <= current_distance + (distance - stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (check_wall_loss && check_wall_loss_correction(initial_walls) /* && distance_left < 90 */) { // TODO: resetear distancia solo cuando el error es pequeño.
        // int32_t left_distance = (distance * MICROMETERS_PER_MILLIMETER) - (get_encoder_avg_micrometers() - current_distance);
        // while (true) {
        //   set_race_started(false);
        //   set_target_linear_speed(0);
        //   printf("%ld\n", left_distance);
        // }
        current_distance = get_encoder_avg_micrometers();
        distance = WALL_LOSS_TO_SENSING_POINT_DISTANCE;
      }

      if (stop) {
        stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
      }
    }
  } else {
    while (is_race_started() && !is_motor_saturated() && get_encoder_avg_micrometers() >= current_distance - (distance + stop_distance) * MICROMETERS_PER_MILLIMETER) {
      if (stop) {
        stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
      }
    }
  }
  if (stop) {
    set_target_linear_speed(0);
    set_ideal_angular_speed(0.0);
    while (is_race_started() && !is_motor_saturated() && get_ideal_linear_speed() != 0) {
    }
  }
#endif
}

/**
 * @brief Movimiento frontal paramétrico hasta una distancia determinada de la pared frontal
 *
 * @param distance
 * @param speed
 * @param stop
 */
void move_straight_until_front_distance(uint32_t distance, int32_t speed, bool stop) {
#ifndef MMSIM_ENABLED
  float stop_distance = 0;
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  while (is_race_started() && (get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) + get_sensor_distance(SENSOR_FRONT_RIGHT_WALL_ID)) / 2 > (distance + stop_distance)) {
    // while (is_race_started() && !is_motor_saturated() && get_sensor_distance(SENSOR_FRONT_LEFT_WALL_ID) > (distance + stop_distance)) {
    if (stop) {
      stop_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), 0);
    }
  }
  if (stop) {
    set_target_linear_speed(0);
    while (is_race_started() && !is_motor_saturated() && (get_ideal_linear_speed() != 0 || (is_front_sensors_correction_enabled() && get_front_sensors_angle_error() != 0))) {
    }
    if (is_front_sensors_correction_enabled()) {
      uint32_t timeout = get_clock_ticks() + 1000;
      uint16_t count = 0;
      while (count < 250 && get_clock_ticks() < timeout) {
        if (get_front_sensors_angle_error() == 0) {
          count++;
        } else {
          count = 0;
        }
      }
    }
  }
#endif
}

void run_straight(float distance, float start_offset, float end_offset, uint16_t cells, bool has_begin, int32_t speed, int32_t final_speed, int8_t next_turn_sign) {
#ifdef MMSIM_ENABLED
  for (uint16_t i = 0; i < cells; i++) {
    API_moveForward();
  }
  return;
#endif
#ifndef MMSIM_ENABLED
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);
  set_side_sensors_close_correction(true);
  set_side_sensors_far_correction(true);

  int32_t current_distance = get_encoder_avg_micrometers();
  float slow_distance = 0;

  uint16_t current_cell = 1;
  int32_t current_cell_distance_left;
  if (has_begin) {
    current_cell_distance_left = CELL_DIMENSION - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2);
  } else {
    current_cell_distance_left = CELL_DIMENSION + start_offset;
  }

  struct walls cell_walls = get_walls();
  struct walls current_walls;
  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  distance += end_offset;
  bool last_cell_wall_lost = (has_begin && cells <= 2) || cells <= 1;
  while (is_race_started() && !is_motor_saturated() && (get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER || !last_cell_wall_lost)) {
    current_walls = get_walls();

    if (!(has_begin && current_cell == 1) && check_wall_loss_correction(cell_walls)) {
      current_distance = get_encoder_avg_micrometers();
      distance = WALL_LOSS_TO_SENSING_POINT_DISTANCE + CELL_DIMENSION * (cells - current_cell) + end_offset;
      if (cells - current_cell == 0) {
        current_cell_distance_left = distance;
        last_cell_wall_lost = true;
        set_RGB_color_while(255, 0, 0, 33);
      } else {
        current_cell_distance_left = WALL_LOSS_TO_SENSING_POINT_DISTANCE;
        set_RGB_color_while(0, 255, 0, 33);
      }
    }

    if (get_encoder_avg_micrometers() - current_distance >= (current_cell_distance_left * MICROMETERS_PER_MILLIMETER)) {
      current_distance = get_encoder_avg_micrometers();
      distance = CELL_DIMENSION * (cells - current_cell) + end_offset;
      current_cell++;
      current_cell_distance_left = CELL_DIMENSION;
      cell_walls = current_walls;
      if (next_turn_sign == 1 && !cell_walls.right) {
        last_cell_wall_lost = true;
      } else if (next_turn_sign == -1 && !cell_walls.left) {
        last_cell_wall_lost = true;
      } else if (next_turn_sign == 0) {
        last_cell_wall_lost = true;
      }
      enter_next_cell();
    }

    if (final_speed != speed) {
      slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed) + 20;
    }
    if (slow_distance > 0 && ((current_distance + distance * MICROMETERS_PER_MILLIMETER) - get_encoder_avg_micrometers()) <= slow_distance * MICROMETERS_PER_MILLIMETER) {
      set_target_linear_speed(final_speed);
    }
  }
  if (current_cell < cells && end_offset == 0) {
    enter_next_cell();
  }
#endif
}

void run_side(enum movement movement, struct turn_params turn, struct turn_params next_turn) {
#ifndef MMSIM_ENABLED
  set_front_sensors_correction(false);
  set_front_sensors_diagonal_correction(false);

  float end_distance_offset = 0.0f;
  float start_distance_offset = 0.0f;
  bool enable_end_distance_offset = true;
  bool enable_start_distance_offset = true;

  switch (movement) {
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
    case MOVE_LEFT_FROM_45:
    case MOVE_RIGHT_FROM_45:
    case MOVE_LEFT_FROM_45_180:
    case MOVE_RIGHT_FROM_45_180:
      enable_start_distance_offset = false;
      enable_end_distance_offset = false;
      set_side_sensors_close_correction(false);
      set_side_sensors_far_correction(false);
      break;
    default:
      set_side_sensors_close_correction(false);
      set_side_sensors_far_correction(false);
      break;
  }

  struct walls walls = get_walls();
  // if (enable_end_distance_offset) {
  //   if (turn.sign > 0) {
  //     if (walls.left) {
  //       end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_LEFT_WALL_ID);
  //     }
  //   } else {
  //     if (walls.right) {
  //       end_distance_offset = MIDDLE_MAZE_DISTANCE - get_sensor_distance(SENSOR_SIDE_RIGHT_WALL_ID);
  //     }
  //   }
  // }

  // if (enable_start_distance_offset) {
  //   if (walls.front) {
  //     start_distance_offset = get_front_wall_distance() - (CELL_DIMENSION - (WALL_WIDTH / 2));
  //   }
  // }

  if (turn.start > 0) {
    if (abs(start_distance_offset) > turn.start / 2) {
      start_distance_offset = start_distance_offset > 0 ? turn.start / 2 : -turn.start / 2;
    }
    move_straight(turn.start - current_cell_start_mm + start_distance_offset, turn.linear_speed, false, false);
  }

  disable_sensors_correction();
  // reset_control_errors(); //! Esto se había puesto por un problema en la acumulación de error según aumenta el número de giros realizados
  move_arc_turn(turn);

  set_front_sensors_correction(false);
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  switch (movement) {
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
      set_front_sensors_diagonal_correction(true);
      break;
    default:
      set_front_sensors_diagonal_correction(false);
      break;
  }

  if (turn.end > 0) {
    if (abs(end_distance_offset) > turn.end / 2) {
      end_distance_offset = end_distance_offset > 0 ? turn.end / 2 : -turn.end / 2;
    }
    move_straight(turn.end + end_distance_offset, next_turn.linear_speed, false, false);
  }
  enter_next_cell();
#endif
}

void run_diagonal(float distance, float end_offset, uint16_t cells, int32_t speed, int32_t final_speed) {
#ifndef MMSIM_ENABLED
  set_front_sensors_correction(false);
  if (cells > 1) {
    set_front_sensors_diagonal_correction(true);
  } else {
    set_front_sensors_diagonal_correction(false);
  }
  set_side_sensors_close_correction(false);
  set_side_sensors_far_correction(false);

  uint16_t current_cell = 1;
  int32_t current_distance = get_encoder_avg_micrometers();
  float slow_distance = 0;
  float remaining_distance = 0;

  set_ideal_angular_speed(0.0);
  set_target_linear_speed(speed);
  distance += end_offset;
  while (is_race_started() && !is_motor_saturated() && get_encoder_avg_micrometers() <= current_distance + distance * MICROMETERS_PER_MILLIMETER) {
    remaining_distance = distance * MICROMETERS_PER_MILLIMETER - (get_encoder_avg_micrometers() - current_distance);
    if (remaining_distance < CELL_DIAGONAL * 0.5f * MICROMETERS_PER_MILLIMETER) {
      set_front_sensors_diagonal_correction(false);
    }

    if (get_encoder_avg_micrometers() - current_distance >= (CELL_DIAGONAL * MICROMETERS_PER_MILLIMETER)) {
      current_distance = get_encoder_avg_micrometers();
      distance = CELL_DIAGONAL * (cells - current_cell) + end_offset;
      current_cell++;
      enter_next_cell();
    }

    if (final_speed != speed) {
      slow_distance = calc_straight_to_speed_distance(get_ideal_linear_speed(), final_speed) + 20;
    }
    if (slow_distance > 0 && ((current_distance + distance * MICROMETERS_PER_MILLIMETER) - get_encoder_avg_micrometers()) <= slow_distance * MICROMETERS_PER_MILLIMETER) {
      set_target_linear_speed(final_speed);
    }
  }
  if (current_cell < cells && remaining_distance < 10) {
    enter_next_cell();
  }
#endif
}

/**
 * @brief Movimiento de giro paramétrico
 *
 * @param turn_type
 */
void move_arc_turn(struct turn_params turn) {
#ifndef MMSIM_ENABLED
  int32_t start = get_encoder_avg_micrometers();
  int32_t current;
  float travelled;
  float angular_speed;
  float factor;
  while (true && is_race_started() && !is_motor_saturated()) {
    set_RGB_color(255, 0, 0);
    current = get_encoder_avg_micrometers();
    travelled = (float)(current - start) / MICROMETERS_PER_MILLIMETER;
    if (travelled >= 2 * turn.transition + turn.arc) {
      break;
    }
    angular_speed = turn.sign * turn.max_angular_speed;
    if (travelled < turn.transition) {
      factor = travelled / turn.transition;
      angular_speed *= sin(factor * PI / 2);
    } else if (travelled >= turn.transition + turn.arc) {
      factor = (travelled - turn.arc) / turn.transition;
      angular_speed *= sin(factor * PI / 2);
    }
    set_ideal_angular_speed(angular_speed);
  }
  set_ideal_angular_speed(0);
  set_RGB_color(0, 0, 0);
#endif
}

void move_inplace_turn(enum movement movement) {
#ifndef MMSIM_ENABLED
  struct inplace_params turn = turns_inplace[movement];
  set_target_linear_speed(turn.linear_speed);

  uint32_t ms_start = get_clock_ticks();
  uint32_t ms_current = ms_start;
  float angular_speed = 0;
  int8_t sign = turn.sign == 0 ? ((int8_t)(rand() % 2) * 2 - 1) : turn.sign;
  while (true) {
    ms_current = get_clock_ticks();
    if (ms_current - ms_start <= turn.t_accel) {
      angular_speed = turn.angular_accel * (ms_current - ms_start) / 1000;
    } else if (ms_current - ms_start <= (turn.t_accel + turn.t_max)) {
      angular_speed = turn.max_angular_speed;
    } else if (ms_current - ms_start <= (uint32_t)(turn.t_accel + turn.t_max + turn.t_accel)) {
      angular_speed = turn.max_angular_speed - (turn.angular_accel * (ms_current - ms_start - turn.t_accel - turn.t_max) / 1000);
    } else {
      set_ideal_angular_speed(0);
      break;
    }
    set_ideal_angular_speed(angular_speed * sign);
  }
  set_ideal_angular_speed(0);
#endif
}

/**
 * @brief Movimiento de giro en el sitio paramétrico
 *
 * @param angle
 * @param rads
 */
void move_inplace_angle(float angle, float rads) {
#ifndef MMSIM_ENABLED
  lsm6dsr_set_gyro_z_degrees(0);
  float current_angle = lsm6dsr_get_gyro_z_degrees();
  float target_angle = current_angle + angle;
  if (target_angle > 360.0) {
    target_angle = 360.0 - target_angle;
  } else if (target_angle < -360) {
    target_angle = 360.0 + target_angle;
  }
  set_target_linear_speed(0.0);
  if (angle >= 0) {
    set_ideal_angular_speed(rads);
    while (is_race_started() && !is_motor_saturated() && lsm6dsr_get_gyro_z_degrees() <= target_angle) {
    }
  } else {
    set_ideal_angular_speed(-rads);
    while (is_race_started() && !is_motor_saturated() && lsm6dsr_get_gyro_z_degrees() >= target_angle) {
    }
  }
  set_ideal_angular_speed(0.0);
#endif
}

void move(enum movement movement) {
#ifndef MMSIM_ENABLED
  set_check_motors_saturated_enabled(true);
#endif
  switch (movement) {
    case MOVE_HOME:
      move_home();
      break;
    case MOVE_START:
      set_starting_position();
      move_front();
      break;
    case MOVE_END:
      move_end();
      break;
    case MOVE_FRONT:
      move_front();
      break;
    case MOVE_LEFT:
    case MOVE_RIGHT:
    case MOVE_LEFT_90:
    case MOVE_RIGHT_90:
    case MOVE_LEFT_180:
    case MOVE_RIGHT_180:
    case MOVE_LEFT_TO_45:
    case MOVE_RIGHT_TO_45:
    case MOVE_LEFT_TO_135:
    case MOVE_RIGHT_TO_135:
    case MOVE_LEFT_45_TO_45:
    case MOVE_RIGHT_45_TO_45:
    case MOVE_LEFT_FROM_45:
    case MOVE_RIGHT_FROM_45:
    case MOVE_LEFT_FROM_45_180:
    case MOVE_RIGHT_FROM_45_180:
      move_side(movement);
      break;
    case MOVE_BACK:
    case MOVE_BACK_WALL:
    case MOVE_BACK_STOP:
      move_back(movement);
      break;
    default:
      break;
  }
}

void move_run_sequence(enum movement *sequence_movements) {
  float distance = 0;
  float start_offset = 0;
  float end_offset = 0;
  bool running_diagonal = false;
  bool straight_has_begin = true;
  uint16_t straight_cells = 0;
  struct turn_params turn_params;
  struct turn_params next_turn_params;

  for (uint16_t i = 0; i < (MAZE_CELLS + 3); i++) {
    switch (sequence_movements[i]) {
      case MOVE_START:
        distance += CELL_DIMENSION - (ROBOT_BACK_LENGTH + WALL_WIDTH / 2);
        straight_cells++;
        straight_has_begin = true;
        break;
      case MOVE_FRONT:
        distance += CELL_DIMENSION;
        straight_cells++;
        break;
      case MOVE_DIAGONAL:
        running_diagonal = true;
        distance += CELL_DIAGONAL;
        straight_cells++;
        break;
      case MOVE_HOME:
        if (distance > 0) {
          if (running_diagonal) {
            run_diagonal(distance, 0, straight_cells, kinematics.linear_speed, 500);
          } else {
            run_straight(distance, 0, 0, straight_cells, straight_has_begin, kinematics.linear_speed, 500, 0);
          }
          distance = 0;
          start_offset = 0;
          end_offset = 0;
          straight_cells = 0;
          straight_has_begin = false;
          running_diagonal = false;
        }
        move(MOVE_HOME);
        break;
      case MOVE_LEFT:
      case MOVE_RIGHT:
      case MOVE_LEFT_90:
      case MOVE_RIGHT_90:
      case MOVE_LEFT_180:
      case MOVE_RIGHT_180:
      case MOVE_LEFT_TO_45:
      case MOVE_RIGHT_TO_45:
      case MOVE_LEFT_TO_135:
      case MOVE_RIGHT_TO_135:
      case MOVE_LEFT_45_TO_45:
      case MOVE_RIGHT_45_TO_45:
      case MOVE_LEFT_FROM_45:
      case MOVE_RIGHT_FROM_45:
      case MOVE_LEFT_FROM_45_180:
      case MOVE_RIGHT_FROM_45_180:

        turn_params = kinematics.turns[sequence_movements[i]];
        if (turn_params.start < 0) {
          end_offset = turn_params.start;
        }

        enum speed_strategy speed_strategy = menu_run_get_speed();
        while (distance + end_offset < calc_straight_to_speed_distance(get_ideal_linear_speed(), turn_params.linear_speed)) {
          if (speed_strategy <= SPEED_NORMAL) {
            turn_params = kinematics_settings[SPEED_NORMAL].turns[sequence_movements[i]];
            break;
          }
          turn_params = kinematics_settings[--speed_strategy].turns[sequence_movements[i]];
          if (turn_params.start < 0) {
            end_offset = turn_params.start;
          }
        }
        if ((i + 1) < (MAZE_CELLS + 3)) {
          switch (sequence_movements[i + 1]) {
            case MOVE_START:
            case MOVE_FRONT:
            case MOVE_DIAGONAL:
            case MOVE_HOME:
              next_turn_params = turn_params;
              break;
            default:
              next_turn_params = kinematics.turns[sequence_movements[i + 1]];
              break;
          }
        } else {
          next_turn_params = turn_params;
        }

        // Resetea el offset para evitar desplazamientos en otras celdas al realizar un giro de 180º no seguido de una recta
        switch (sequence_movements[i]) {
          case MOVE_LEFT_180:
          case MOVE_RIGHT_180:
            if ((i + 1) < (MAZE_CELLS + 3) && sequence_movements[i + 1] != MOVE_FRONT) {
              end_offset = 0;
            }
            break;
          default:
            // Nothing to do
            break;
        }

        if (distance > 0 || end_offset > 0) {
          if (running_diagonal) {
            run_diagonal(distance, end_offset, straight_cells, kinematics.linear_speed, turn_params.linear_speed);
          } else {
            run_straight(distance, start_offset, end_offset, straight_cells, straight_has_begin, kinematics.linear_speed, turn_params.linear_speed, turn_params.sign);
          }
          end_offset = 0;
          start_offset = 0;
          straight_cells = 0;
          straight_has_begin = false;
          running_diagonal = false;
        }
        if (turn_params.end < 0) {
          distance = turn_params.end;
          start_offset = turn_params.end;
        } else {
          distance = 0;
        }

        // Resetea el offset para evitar desplazamientos en otras celdas al realizar un giro de 180º no seguido de una recta
        switch (sequence_movements[i]) {
          case MOVE_LEFT_180:
          case MOVE_RIGHT_180:
            if ((i + 1) < (MAZE_CELLS + 3) && sequence_movements[i + 1] != MOVE_FRONT) {
              distance = 0;
            }
            break;
          default:
            // Nothing to do
            break;
        }

        // TODO: Obtener las kinematics de giro a partir de la velocidad máxima de la recta
        run_side(sequence_movements[i], turn_params, next_turn_params);
        break;
      default:
        i = (MAZE_CELLS + 3);
        break;
    }
  }
}