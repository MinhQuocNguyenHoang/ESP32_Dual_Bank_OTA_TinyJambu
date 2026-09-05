#ifndef GRADIENT_BOOSTING_MODEL_H
#define GRADIENT_BOOSTING_MODEL_H

// Auto-generated Gradient Boosting Trees for Embedded IoT
const float GB_INIT_VAL = 105.941667f;
const float GB_LEARNING_RATE = 0.100000f;

static inline float predict_gb_tree_0(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[1] <= 1.386281f) {
    if (x[2] <= -0.425420f) {
    return 7.625000f;
    } else {
    return 2.158333f;
    }
    } else {
    return 17.658333f;
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[4] <= -1.657338f) {
    return -18.341667f;
    } else {
    return -10.641667f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -2.491667f;
    } else {
    return -5.441667f;
    }
    }
    }
}

static inline float predict_gb_tree_1(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[1] <= 1.386281f) {
    if (x[2] <= -0.425420f) {
    return 6.862500f;
    } else {
    return 1.942500f;
    }
    } else {
    return 15.892500f;
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[4] <= -0.156040f) {
    return -4.362500f;
    } else {
    return -1.192500f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -9.577500f;
    } else {
    return -16.507500f;
    }
    }
    }
}

static inline float predict_gb_tree_2(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -1.964750f;
    } else {
    return -4.461250f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -14.856750f;
    } else {
    return -8.619750f;
    }
    }
    } else {
    if (x[4] <= 1.488630f) {
    if (x[2] <= -0.425420f) {
    return 6.176250f;
    } else {
    return 1.748250f;
    }
    } else {
    return 14.303250f;
    }
    }
}

static inline float predict_gb_tree_3(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[2] <= -1.442673f) {
    return 12.872925f;
    } else {
    if (x[2] <= -0.590583f) {
    return 6.441958f;
    } else {
    return 2.312936f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[1] <= -0.058812f) {
    return -3.563342f;
    } else {
    return -0.876775f;
    }
    } else {
    if (x[3] <= 1.767393f) {
    return -7.757775f;
    } else {
    return -13.371075f;
    }
    }
    }
}

static inline float predict_gb_tree_4(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[4] <= 1.488630f) {
    if (x[2] <= -0.590583f) {
    return 5.797763f;
    } else {
    return 2.081643f;
    }
    } else {
    return 11.585633f;
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -1.546269f;
    } else {
    return -3.658791f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -6.981997f;
    } else {
    return -12.033968f;
    }
    }
    }
}

static inline float predict_gb_tree_5(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -6.283798f;
    } else {
    return -10.830571f;
    }
    } else {
    if (x[3] <= 0.013940f) {
    return -0.150252f;
    } else {
    return -2.911546f;
    }
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 10.427069f;
    } else {
    if (x[2] <= -0.590583f) {
    return 5.217986f;
    } else {
    return 2.643234f;
    }
    }
    }
}

static inline float predict_gb_tree_6(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -5.655418f;
    } else {
    return -9.747514f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -1.592954f;
    } else {
    return -3.701757f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 3.517182f;
    } else {
    return 1.009318f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 9.384362f;
    } else {
    return 5.446188f;
    }
    }
    }
}

static inline float predict_gb_tree_7(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.981957f) {
    if (x[4] <= -0.156040f) {
    return -2.390802f;
    } else {
    return -0.106045f;
    }
    } else {
    if (x[3] <= 1.767393f) {
    return -5.089876f;
    } else {
    return -8.772762f;
    }
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 8.445926f;
    } else {
    if (x[2] <= -0.590583f) {
    return 4.248019f;
    } else {
    return 2.152585f;
    }
    }
    }
}

static inline float predict_gb_tree_8(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.981957f) {
    if (x[3] <= 0.013940f) {
    return -0.095440f;
    } else {
    return -2.151722f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -7.895486f;
    } else {
    return -4.580889f;
    }
    }
    } else {
    if (x[2] <= -1.442673f) {
    return 7.601333f;
    } else {
    if (x[2] <= -0.590583f) {
    return 3.823217f;
    } else {
    return 1.937327f;
    }
    }
    }
}

static inline float predict_gb_tree_9(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 2.557407f;
    } else {
    return 0.713965f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 6.841200f;
    } else {
    return 4.094445f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[4] <= -0.486136f) {
    return -2.877329f;
    } else {
    return -1.124107f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -4.122800f;
    } else {
    return -7.105937f;
    }
    }
    }
}

static inline float predict_gb_tree_10(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[3] <= -1.303720f) {
    return 6.157080f;
    } else {
    if (x[0] <= -0.953668f) {
    return 3.685001f;
    } else {
    return 1.897219f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[3] <= 0.013940f) {
    return -0.065389f;
    } else {
    return -1.765698f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -3.710520f;
    } else {
    return -6.395344f;
    }
    }
    }
}

static inline float predict_gb_tree_11(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[2] <= 0.824563f) {
    if (x[4] <= -0.486136f) {
    return -2.413026f;
    } else {
    return -0.891804f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -3.339468f;
    } else {
    return -5.755809f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 2.111944f;
    } else {
    return 0.550977f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 5.541372f;
    } else {
    return 3.316501f;
    }
    }
    }
}

static inline float predict_gb_tree_12(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[0] <= -1.678304f) {
    return 4.987235f;
    } else {
    if (x[2] <= -0.590583f) {
    return 2.557770f;
    } else {
    return 1.257158f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -5.180228f;
    } else {
    return -3.005521f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.316523f;
    } else {
    return -1.740886f;
    }
    }
    }
}

static inline float predict_gb_tree_13(const float* x) {
    if (x[1] <= 0.202841f) {
    if (x[3] <= 0.797331f) {
    if (x[4] <= -0.486136f) {
    return -1.997635f;
    } else {
    return -0.723492f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -4.662206f;
    } else {
    return -2.704969f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[2] <= -1.442673f) {
    return 4.488511f;
    } else {
    return 2.729074f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 1.710003f;
    } else {
    return 0.448847f;
    }
    }
    }
}

static inline float predict_gb_tree_14(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[0] <= -0.953668f) {
    if (x[2] <= -1.442673f) {
    return 4.039660f;
    } else {
    return 2.456166f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 1.539003f;
    } else {
    return 0.226685f;
    }
    }
    } else {
    if (x[2] <= 1.661639f) {
    if (x[4] <= -0.486136f) {
    return -2.116172f;
    } else {
    return -0.912780f;
    }
    } else {
    return -4.195985f;
    }
    }
}

static inline float predict_gb_tree_15(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[1] <= -0.831769f) {
    if (x[0] <= 1.481369f) {
    return -2.222855f;
    } else {
    return -3.776387f;
    }
    } else {
    if (x[4] <= -0.156040f) {
    return -1.076419f;
    } else {
    return -0.019093f;
    }
    }
    } else {
    if (x[1] <= 1.386281f) {
    if (x[2] <= -0.590583f) {
    return 1.880280f;
    } else {
    return 0.935215f;
    }
    } else {
    return 3.635694f;
    }
    }
}

static inline float predict_gb_tree_16(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[1] <= 1.386281f) {
    if (x[4] <= 0.823386f) {
    return 1.015123f;
    } else {
    return 2.022522f;
    }
    } else {
    return 3.272125f;
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -0.199132f;
    } else {
    return -1.171652f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -3.398748f;
    } else {
    return -2.000569f;
    }
    }
    }
}

static inline float predict_gb_tree_17(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.652281f) {
    return -0.473119f;
    } else {
    return -1.361447f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -3.058873f;
    } else {
    return -1.800512f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[1] <= 1.386281f) {
    return 1.820269f;
    } else {
    return 2.944912f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 1.142816f;
    } else {
    return 0.294689f;
    }
    }
    }
}

static inline float predict_gb_tree_18(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 1.028534f;
    } else {
    return 0.149678f;
    }
    } else {
    if (x[1] <= 1.386281f) {
    return 1.638242f;
    } else {
    return 2.650421f;
    }
    }
    } else {
    if (x[2] <= 1.661639f) {
    if (x[0] <= 0.652281f) {
    return -0.598009f;
    } else {
    return -1.422882f;
    }
    } else {
    return -2.752986f;
    }
    }
}

static inline float predict_gb_tree_19(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.925681f;
    } else {
    return 0.250252f;
    }
    } else {
    if (x[1] <= 1.386281f) {
    return 1.474418f;
    } else {
    return 2.385379f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.390929f;
    } else {
    return -1.083014f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -2.477687f;
    } else {
    return -1.478173f;
    }
    }
    }
}

static inline float predict_gb_tree_20(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 1.326976f;
    } else {
    return 2.146841f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.833113f;
    } else {
    return 0.131058f;
    }
    }
    } else {
    if (x[3] <= 1.767393f) {
    if (x[0] <= 0.652281f) {
    return -0.499115f;
    } else {
    return -1.152534f;
    }
    } else {
    return -2.229918f;
    }
    }
}

static inline float predict_gb_tree_21(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[1] <= 1.386281f) {
    if (x[2] <= -0.590583f) {
    return 1.030868f;
    } else {
    return 0.502390f;
    }
    } else {
    return 1.932157f;
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -0.121925f;
    } else {
    return -0.705434f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -1.215102f;
    } else {
    return -2.006927f;
    }
    }
    }
}

static inline float predict_gb_tree_22(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[3] <= -0.811539f) {
    if (x[0] <= -1.678304f) {
    return 1.738941f;
    } else {
    return 1.091192f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.673138f;
    } else {
    return 0.109334f;
    }
    }
    } else {
    if (x[0] <= 1.481369f) {
    if (x[4] <= -0.486136f) {
    return -0.941254f;
    } else {
    return -0.407836f;
    }
    } else {
    return -1.806234f;
    }
    }
}

static inline float predict_gb_tree_23(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[4] <= 1.488630f) {
    if (x[2] <= -0.590583f) {
    return 0.839564f;
    } else {
    return 0.413027f;
    }
    } else {
    return 1.565047f;
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[4] <= -1.657338f) {
    return -1.625611f;
    } else {
    return -0.999467f;
    }
    } else {
    if (x[2] <= 0.160158f) {
    return -0.008130f;
    } else {
    return -0.476298f;
    }
    }
    }
}

static inline float predict_gb_tree_24(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.433585f) {
    return -0.157353f;
    } else {
    return -0.519806f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -1.463050f;
    } else {
    return -0.899520f;
    }
    }
    } else {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.543195f;
    } else {
    return 0.161919f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 1.408542f;
    } else {
    return 0.898116f;
    }
    }
    }
}

static inline float predict_gb_tree_25(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[1] <= 0.822167f) {
    if (x[0] <= -0.519540f) {
    return 0.253967f;
    } else {
    return 0.488876f;
    }
    } else {
    if (x[4] <= 1.488630f) {
    return 0.808305f;
    } else {
    return 1.267688f;
    }
    }
    } else {
    if (x[0] <= 0.652281f) {
    if (x[3] <= 0.013940f) {
    return -0.007545f;
    } else {
    return -0.285564f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -1.316745f;
    } else {
    return -0.702374f;
    }
    }
    }
}

static inline float predict_gb_tree_26(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -0.071894f;
    } else {
    return -0.418429f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.739331f;
    } else {
    return -1.185070f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[0] <= -0.519540f) {
    return 0.228570f;
    } else {
    return 0.439988f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 1.140919f;
    } else {
    return 0.727474f;
    }
    }
    }
}

static inline float predict_gb_tree_27(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 0.654727f;
    } else {
    return 1.026827f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.395989f;
    } else {
    return 0.125572f;
    }
    }
    } else {
    if (x[3] <= 0.797331f) {
    if (x[4] <= -0.486136f) {
    return -0.483100f;
    } else {
    return -0.169872f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -1.066563f;
    } else {
    return -0.665397f;
    }
    }
    }
}

static inline float predict_gb_tree_28(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[0] <= 0.981957f) {
    if (x[4] <= -0.486136f) {
    return -0.434790f;
    } else {
    return -0.215504f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.959907f;
    } else {
    return -0.598858f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[2] <= -1.442673f) {
    return 0.924145f;
    } else {
    return 0.507774f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.002614f;
    } else {
    return 0.239821f;
    }
    }
    }
}

static inline float predict_gb_tree_29(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[0] <= -0.953668f) {
    if (x[4] <= 1.488630f) {
    return 0.538477f;
    } else {
    return 0.831730f;
    }
    } else {
    if (x[4] <= 0.306609f) {
    return 0.002353f;
    } else {
    return 0.269065f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[0] <= 1.481369f) {
    return -0.538972f;
    } else {
    return -0.863916f;
    }
    } else {
    if (x[4] <= -0.486136f) {
    return -0.391311f;
    } else {
    return -0.193954f;
    }
    }
    }
}

static inline float predict_gb_tree_30(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.485075f;
    } else {
    return -0.777524f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.082561f;
    } else {
    return -0.282159f;
    }
    }
    } else {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 0.484629f;
    } else {
    return 0.748557f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.292104f;
    } else {
    return 0.087323f;
    }
    }
    }
}

static inline float predict_gb_tree_31(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[0] <= 0.981957f) {
    if (x[4] <= -0.486136f) {
    return -0.323964f;
    } else {
    return -0.156323f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.699772f;
    } else {
    return -0.436567f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[1] <= 1.386281f) {
    return 0.377783f;
    } else {
    return 0.673701f;
    }
    } else {
    if (x[3] <= -0.399663f) {
    return 0.169961f;
    } else {
    return 0.001879f;
    }
    }
    }
}

static inline float predict_gb_tree_32(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[2] <= 0.824563f) {
    if (x[4] <= -0.486136f) {
    return -0.291568f;
    } else {
    return -0.140690f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.392911f;
    } else {
    return -0.629795f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[2] <= -1.442673f) {
    return 0.606331f;
    } else {
    return 0.340005f;
    }
    } else {
    if (x[4] <= 0.349912f) {
    return 0.001691f;
    } else {
    return 0.152965f;
    }
    }
    }
}

static inline float predict_gb_tree_33(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.126621f;
    } else {
    return -0.262411f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -0.566815f;
    } else {
    return -0.353620f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.210858f;
    } else {
    return 0.034762f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.545698f;
    } else {
    return 0.364387f;
    }
    }
    }
}

static inline float predict_gb_tree_34(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[4] <= -0.854546f) {
    if (x[4] <= -1.657338f) {
    return -0.510134f;
    } else {
    return -0.318258f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.055040f;
    } else {
    return -0.188864f;
    }
    }
    } else {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.189772f;
    } else {
    return 0.058789f;
    }
    } else {
    if (x[4] <= 1.488630f) {
    return 0.327949f;
    } else {
    return 0.491128f;
    }
    }
    }
}

static inline float predict_gb_tree_35(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[2] <= -0.849588f) {
    if (x[2] <= -1.442673f) {
    return 0.442016f;
    } else {
    return 0.295154f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.207559f;
    } else {
    return 0.112959f;
    }
    }
    } else {
    if (x[4] <= -0.486136f) {
    if (x[2] <= 1.661639f) {
    return -0.251858f;
    } else {
    return -0.459120f;
    }
    } else {
    if (x[3] <= 0.013940f) {
    return -0.002142f;
    } else {
    return -0.101764f;
    }
    }
    }
}

static inline float predict_gb_tree_36(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[1] <= -0.058812f) {
    return -0.091588f;
    } else {
    return -0.001927f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.226672f;
    } else {
    return -0.413208f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[4] <= 0.393167f) {
    return 0.186803f;
    } else {
    return 0.101663f;
    }
    } else {
    if (x[4] <= 1.488630f) {
    return 0.265638f;
    } else {
    return 0.397814f;
    }
    }
    }
}

static inline float predict_gb_tree_37(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.140346f;
    } else {
    return 0.042383f;
    }
    } else {
    if (x[2] <= -1.442673f) {
    return 0.358033f;
    } else {
    return 0.239075f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.433585f) {
    return -0.039665f;
    } else {
    return -0.136384f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.371888f;
    } else {
    return -0.238579f;
    }
    }
    }
}

static inline float predict_gb_tree_38(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[2] <= -0.849588f) {
    if (x[4] <= 1.488630f) {
    return 0.215167f;
    } else {
    return 0.322229f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.154088f;
    } else {
    return 0.082361f;
    }
    }
    } else {
    if (x[0] <= 0.652281f) {
    if (x[2] <= 0.160158f) {
    return -0.001871f;
    } else {
    return -0.073626f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.334699f;
    } else {
    return -0.185256f;
    }
    }
    }
}

static inline float predict_gb_tree_39(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[4] <= -0.156040f) {
    return -0.066264f;
    } else {
    return -0.001683f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.301229f;
    } else {
    return -0.166731f;
    }
    }
    } else {
    if (x[0] <= -0.953668f) {
    if (x[0] <= -1.678304f) {
    return 0.290006f;
    } else {
    return 0.193650f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.138679f;
    } else {
    return 0.074125f;
    }
    }
    }
}

static inline float predict_gb_tree_40(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.103849f;
    } else {
    return 0.030498f;
    }
    } else {
    if (x[1] <= 1.386281f) {
    return 0.174285f;
    } else {
    return 0.261006f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[0] <= 1.481369f) {
    return -0.179522f;
    } else {
    return -0.271106f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.028526f;
    } else {
    return -0.098152f;
    }
    }
    }
}

static inline float predict_gb_tree_41(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -0.014646f;
    } else {
    return -0.088337f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.161570f;
    } else {
    return -0.243995f;
    }
    }
    } else {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 0.156857f;
    } else {
    return 0.234905f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.114426f;
    } else {
    return 0.059995f;
    }
    }
    }
}

static inline float predict_gb_tree_42(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[2] <= -0.849588f) {
    if (x[4] <= 1.488630f) {
    return 0.141171f;
    } else {
    return 0.211415f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.084743f;
    } else {
    return 0.025181f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[4] <= -1.657338f) {
    return -0.219596f;
    } else {
    return -0.145413f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.035160f;
    } else {
    return -0.101944f;
    }
    }
    }
}

static inline float predict_gb_tree_43(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.190273f;
    } else {
    return 0.110782f;
    }
    } else {
    if (x[0] <= -0.147430f) {
    return 0.048499f;
    } else {
    return 0.000350f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.044638f;
    } else {
    return -0.091750f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -0.197636f;
    } else {
    return -0.130872f;
    }
    }
    }
}

static inline float predict_gb_tree_44(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.171246f;
    } else {
    return 0.099704f;
    }
    } else {
    if (x[0] <= -0.147430f) {
    return 0.043649f;
    } else {
    return 0.000315f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.040175f;
    } else {
    return -0.082575f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -0.177873f;
    } else {
    return -0.117785f;
    }
    }
    }
}

static inline float predict_gb_tree_45(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[2] <= -1.442673f) {
    return 0.154121f;
    } else {
    return 0.089733f;
    }
    } else {
    if (x[3] <= -0.399663f) {
    return 0.039284f;
    } else {
    return 0.000283f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.036157f;
    } else {
    return -0.074317f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.106006f;
    } else {
    return -0.160085f;
    }
    }
    }
}

static inline float predict_gb_tree_46(const float* x) {
    if (x[2] <= -0.110109f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.054686f;
    } else {
    return 0.016044f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.138709f;
    } else {
    return 0.097032f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[0] <= 1.481369f) {
    return -0.095405f;
    } else {
    return -0.144077f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.014692f;
    } else {
    return -0.054167f;
    }
    }
    }
}

static inline float predict_gb_tree_47(const float* x) {
    if (x[1] <= -0.058812f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.652281f) {
    return -0.029098f;
    } else {
    return -0.061469f;
    }
    } else {
    if (x[3] <= 1.767393f) {
    return -0.085865f;
    } else {
    return -0.129669f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[4] <= 1.488630f) {
    return 0.073174f;
    } else {
    return 0.124838f;
    }
    } else {
    if (x[0] <= -0.147430f) {
    return 0.031819f;
    } else {
    return 0.000187f;
    }
    }
    }
}

static inline float predict_gb_tree_48(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[0] <= -0.953668f) {
    if (x[3] <= -1.303720f) {
    return 0.112354f;
    } else {
    return 0.080012f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.043968f;
    } else {
    return 0.012839f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.077278f;
    } else {
    return -0.116702f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.011777f;
    } else {
    return -0.044222f;
    }
    }
    }
}

static inline float predict_gb_tree_49(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.849588f) {
    if (x[1] <= 1.386281f) {
    return 0.072010f;
    } else {
    return 0.101119f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.039571f;
    } else {
    return 0.006663f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.652281f) {
    return -0.023389f;
    } else {
    return -0.050900f;
    }
    } else {
    if (x[3] <= 1.767393f) {
    return -0.069551f;
    } else {
    return -0.105032f;
    }
    }
    }
}

static inline float predict_gb_tree_50(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[3] <= 0.013940f) {
    return -0.000551f;
    } else {
    return -0.021050f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.054203f;
    } else {
    return -0.094529f;
    }
    }
    } else {
    if (x[1] <= 0.822167f) {
    if (x[3] <= -0.390634f) {
    return 0.023485f;
    } else {
    return 0.043348f;
    }
    } else {
    if (x[4] <= 1.488630f) {
    return 0.064809f;
    } else {
    return 0.091007f;
    }
    }
    }
}

static inline float predict_gb_tree_51(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[4] <= -0.486136f) {
    if (x[4] <= -1.657338f) {
    return -0.085076f;
    } else {
    return -0.048782f;
    }
    } else {
    if (x[2] <= 0.160158f) {
    return -0.000496f;
    } else {
    return -0.018945f;
    }
    }
    } else {
    if (x[2] <= -0.849588f) {
    if (x[2] <= -1.442673f) {
    return 0.081906f;
    } else {
    return 0.058328f;
    }
    } else {
    if (x[3] <= -0.390634f) {
    return 0.021137f;
    } else {
    return 0.039013f;
    }
    }
    }
}

static inline float predict_gb_tree_52(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[4] <= -0.486136f) {
    if (x[1] <= -1.712747f) {
    return -0.076568f;
    } else {
    return -0.043904f;
    }
    } else {
    if (x[1] <= -0.058812f) {
    return -0.017050f;
    } else {
    return -0.000446f;
    }
    }
    } else {
    if (x[2] <= -0.849588f) {
    if (x[1] <= 1.386281f) {
    return 0.052496f;
    } else {
    return 0.073716f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.035112f;
    } else {
    return 0.019023f;
    }
    }
    }
}

static inline float predict_gb_tree_53(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.068912f;
    } else {
    return -0.047907f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.006836f;
    } else {
    return -0.025889f;
    }
    }
    } else {
    if (x[2] <= -0.849588f) {
    if (x[3] <= -1.303720f) {
    return 0.066344f;
    } else {
    return 0.047246f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.026558f;
    } else {
    return 0.007781f;
    }
    }
    }
}

static inline float predict_gb_tree_54(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.043116f;
    } else {
    return -0.062020f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.003416f;
    } else {
    return -0.023300f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[3] <= -1.303720f) {
    return 0.059710f;
    } else {
    return 0.042521f;
    }
    } else {
    if (x[2] <= -0.590583f) {
    return 0.028945f;
    } else {
    return 0.015404f;
    }
    }
    }
}

static inline float predict_gb_tree_55(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -0.005811f;
    } else {
    return -0.020970f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.055818f;
    } else {
    return -0.038804f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[1] <= 1.386281f) {
    return 0.038269f;
    } else {
    return 0.053739f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.021685f;
    } else {
    return 0.006403f;
    }
    }
    }
}

static inline float predict_gb_tree_56(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[4] <= -0.486136f) {
    if (x[1] <= -1.712747f) {
    return -0.050236f;
    } else {
    return -0.029515f;
    }
    } else {
    if (x[4] <= -0.156040f) {
    return -0.011034f;
    } else {
    return -0.000137f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[3] <= -0.390634f) {
    return 0.012459f;
    } else {
    return 0.023882f;
    }
    } else {
    if (x[2] <= -1.442673f) {
    return 0.048365f;
    } else {
    return 0.034442f;
    }
    }
    }
}

static inline float predict_gb_tree_57(const float* x) {
    if (x[2] <= -0.110109f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.017699f;
    } else {
    return 0.005147f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 0.043528f;
    } else {
    return 0.030998f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.433585f) {
    return -0.004671f;
    } else {
    return -0.016845f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.031972f;
    } else {
    return -0.045213f;
    }
    }
    }
}

static inline float predict_gb_tree_58(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[3] <= 0.013940f) {
    return -0.000147f;
    } else {
    return -0.008855f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.040692f;
    } else {
    return -0.024122f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[4] <= 1.488630f) {
    return 0.027898f;
    } else {
    return 0.039176f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.019724f;
    } else {
    return 0.010071f;
    }
    }
    }
}

static inline float predict_gb_tree_59(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[2] <= -0.849588f) {
    if (x[3] <= -1.303720f) {
    return 0.035258f;
    } else {
    return 0.025108f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.017751f;
    } else {
    return 0.009064f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.036622f;
    } else {
    return -0.026363f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.002079f;
    } else {
    return -0.013512f;
    }
    }
    }
}

static inline float predict_gb_tree_60(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 0.022598f;
    } else {
    return 0.031732f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.013099f;
    } else {
    return 0.003787f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[0] <= 1.481369f) {
    return -0.023727f;
    } else {
    return -0.032960f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.005236f;
    } else {
    return -0.015706f;
    }
    }
    }
}

static inline float predict_gb_tree_61(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[1] <= 1.386281f) {
    return 0.017502f;
    } else {
    return 0.028559f;
    }
    } else {
    if (x[4] <= 0.349912f) {
    return 0.000148f;
    } else {
    return 0.007313f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.006666f;
    } else {
    return -0.014135f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.021354f;
    } else {
    return -0.029664f;
    }
    }
    }
}

static inline float predict_gb_tree_62(const float* x) {
    if (x[1] <= -0.058812f) {
    if (x[1] <= -0.831769f) {
    if (x[4] <= -1.657338f) {
    return -0.026698f;
    } else {
    return -0.019219f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.006000f;
    } else {
    return -0.012722f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[4] <= 1.488630f) {
    return 0.015752f;
    } else {
    return 0.025703f;
    }
    } else {
    if (x[3] <= -0.399663f) {
    return 0.006582f;
    } else {
    return 0.000133f;
    }
    }
    }
}

static inline float predict_gb_tree_63(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[2] <= -1.442673f) {
    return 0.023133f;
    } else {
    return 0.014177f;
    }
    } else {
    if (x[4] <= 0.349912f) {
    return 0.000120f;
    } else {
    return 0.005924f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.017297f;
    } else {
    return -0.024028f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.005400f;
    } else {
    return -0.011449f;
    }
    }
    }
}

static inline float predict_gb_tree_64(const float* x) {
    if (x[1] <= 0.202841f) {
    if (x[4] <= -0.854546f) {
    if (x[0] <= 1.481369f) {
    return -0.015567f;
    } else {
    return -0.021625f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.002139f;
    } else {
    return -0.008295f;
    }
    }
    } else {
    if (x[2] <= -0.849588f) {
    if (x[2] <= -1.442673f) {
    return 0.020820f;
    } else {
    return 0.015595f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.008426f;
    } else {
    return 0.002397f;
    }
    }
    }
}

static inline float predict_gb_tree_65(const float* x) {
    if (x[1] <= -0.058812f) {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.019463f;
    } else {
    return -0.014010f;
    }
    } else {
    if (x[4] <= -0.486136f) {
    return -0.009475f;
    } else {
    return -0.004338f;
    }
    }
    } else {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.007584f;
    } else {
    return 0.001228f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 0.018738f;
    } else {
    return 0.014035f;
    }
    }
    }
}

static inline float predict_gb_tree_66(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[0] <= -0.953668f) {
    if (x[4] <= 1.488630f) {
    return 0.012632f;
    } else {
    return 0.016864f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.006825f;
    } else {
    return 0.002035f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.017516f;
    } else {
    return -0.012609f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.001770f;
    } else {
    return -0.006775f;
    }
    }
    }
}

static inline float predict_gb_tree_67(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.590583f) {
    return 0.007640f;
    } else {
    return 0.003907f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.015177f;
    } else {
    return 0.011369f;
    }
    }
    } else {
    if (x[4] <= -0.486136f) {
    if (x[2] <= 1.661639f) {
    return -0.009599f;
    } else {
    return -0.015765f;
    }
    } else {
    if (x[4] <= -0.156040f) {
    return -0.003477f;
    } else {
    return -0.000041f;
    }
    }
    }
}

static inline float predict_gb_tree_68(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[3] <= -0.811539f) {
    if (x[3] <= -1.303720f) {
    return 0.013660f;
    } else {
    return 0.010232f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.006876f;
    } else {
    return 0.003516f;
    }
    }
    } else {
    if (x[4] <= -0.486136f) {
    if (x[2] <= 1.661639f) {
    return -0.008639f;
    } else {
    return -0.014188f;
    }
    } else {
    if (x[2] <= 0.160158f) {
    return -0.000037f;
    } else {
    return -0.003129f;
    }
    }
    }
}

static inline float predict_gb_tree_69(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[2] <= -0.849588f) {
    if (x[2] <= -1.442673f) {
    return 0.012294f;
    } else {
    return 0.009209f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.005046f;
    } else {
    return 0.001464f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.433585f) {
    return -0.001259f;
    } else {
    return -0.004855f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.009525f;
    } else {
    return -0.012769f;
    }
    }
    }
}

static inline float predict_gb_tree_70(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[1] <= 0.822167f) {
    if (x[1] <= 0.363674f) {
    return 0.005684f;
    } else {
    return 0.002839f;
    }
    } else {
    if (x[2] <= -1.442673f) {
    return 0.011064f;
    } else {
    return 0.008288f;
    }
    }
    } else {
    if (x[4] <= -0.486136f) {
    if (x[0] <= 0.981957f) {
    return -0.005541f;
    } else {
    return -0.010032f;
    }
    } else {
    if (x[3] <= 0.013940f) {
    return -0.000043f;
    } else {
    return -0.002511f;
    }
    }
    }
}

static inline float predict_gb_tree_71(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[3] <= 0.013940f) {
    return -0.000039f;
    } else {
    return -0.002260f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.010489f;
    } else {
    return -0.006278f;
    }
    }
    } else {
    if (x[0] <= -0.953668f) {
    if (x[0] <= -1.678304f) {
    return 0.009958f;
    } else {
    return 0.007459f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.005115f;
    } else {
    return 0.002555f;
    }
    }
    }
}

static inline float predict_gb_tree_72(const float* x) {
    if (x[4] <= 0.122091f) {
    if (x[4] <= -0.854546f) {
    if (x[4] <= -1.657338f) {
    return -0.009440f;
    } else {
    return -0.006941f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000890f;
    } else {
    return -0.003540f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.003732f;
    } else {
    return 0.001052f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.008962f;
    } else {
    return 0.006713f;
    }
    }
    }
}

static inline float predict_gb_tree_73(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[3] <= -0.811539f) {
    if (x[4] <= 1.488630f) {
    return 0.006042f;
    } else {
    return 0.008066f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.004231f;
    } else {
    return 0.002060f;
    }
    }
    } else {
    if (x[0] <= 0.652281f) {
    if (x[1] <= -0.058812f) {
    return -0.001812f;
    } else {
    return -0.000043f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.008496f;
    } else {
    return -0.005126f;
    }
    }
    }
}

static inline float predict_gb_tree_74(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[0] <= -0.953668f) {
    if (x[3] <= -1.303720f) {
    return 0.007259f;
    } else {
    return 0.005438f;
    }
    } else {
    if (x[1] <= 0.363674f) {
    return 0.003807f;
    } else {
    return 0.001854f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[3] <= 1.767393f) {
    return -0.005734f;
    } else {
    return -0.007647f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000384f;
    } else {
    return -0.002839f;
    }
    }
    }
}

static inline float predict_gb_tree_75(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.006533f;
    } else {
    return 0.004160f;
    }
    } else {
    if (x[3] <= -0.399663f) {
    return 0.001669f;
    } else {
    return 0.000302f;
    }
    }
    } else {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -0.000670f;
    } else {
    return -0.002555f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.005161f;
    } else {
    return -0.006882f;
    }
    }
    }
}

static inline float predict_gb_tree_76(const float* x) {
    if (x[1] <= 0.202841f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.652281f) {
    return -0.000951f;
    } else {
    return -0.002953f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.004645f;
    } else {
    return -0.006194f;
    }
    }
    } else {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.002469f;
    } else {
    return 0.000674f;
    }
    } else {
    if (x[0] <= -1.678304f) {
    return 0.005880f;
    } else {
    return 0.004478f;
    }
    }
    }
}

static inline float predict_gb_tree_77(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.004180f;
    } else {
    return -0.005574f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.001213f;
    } else {
    return -0.002657f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[4] <= 0.712278f) {
    return 0.002764f;
    } else {
    return 0.004661f;
    }
    } else {
    if (x[3] <= -0.399663f) {
    return 0.001345f;
    } else {
    return 0.000032f;
    }
    }
    }
}

static inline float predict_gb_tree_78(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.004826f;
    } else {
    return 0.003026f;
    }
    } else {
    if (x[0] <= -0.147430f) {
    return 0.001210f;
    } else {
    return 0.000029f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[3] <= 1.767393f) {
    return -0.003762f;
    } else {
    return -0.005017f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.001092f;
    } else {
    return -0.002392f;
    }
    }
    }
}

static inline float predict_gb_tree_79(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.004343f;
    } else {
    return 0.002723f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.000026f;
    } else {
    return 0.001089f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.652281f) {
    return -0.000983f;
    } else {
    return -0.002153f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.004515f;
    } else {
    return -0.003386f;
    }
    }
    }
}

static inline float predict_gb_tree_80(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[1] <= 0.656533f) {
    return 0.001913f;
    } else {
    return 0.003449f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.000023f;
    } else {
    return 0.000980f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.004064f;
    } else {
    return -0.003047f;
    }
    } else {
    if (x[4] <= -0.486136f) {
    return -0.001937f;
    } else {
    return -0.000885f;
    }
    }
    }
}

static inline float predict_gb_tree_81(const float* x) {
    if (x[2] <= -0.110109f) {
    if (x[0] <= -0.953668f) {
    if (x[2] <= -1.442673f) {
    return 0.003564f;
    } else {
    return 0.002644f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.001470f;
    } else {
    return 0.000370f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[0] <= 1.481369f) {
    return -0.002743f;
    } else {
    return -0.003657f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000305f;
    } else {
    return -0.001439f;
    }
    }
    }
}

static inline float predict_gb_tree_82(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[3] <= -1.303720f) {
    return 0.003208f;
    } else {
    return 0.001977f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.000018f;
    } else {
    return 0.000790f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.652281f) {
    return -0.000709f;
    } else {
    return -0.001600f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.002468f;
    } else {
    return -0.003292f;
    }
    }
    }
}

static inline float predict_gb_tree_83(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.001185f;
    } else {
    return 0.000292f;
    }
    } else {
    if (x[4] <= 1.488630f) {
    return 0.002182f;
    } else {
    return 0.002887f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -0.000240f;
    } else {
    return -0.001180f;
    }
    } else {
    if (x[3] <= 1.767393f) {
    return -0.002222f;
    } else {
    return -0.002962f;
    }
    }
    }
}

static inline float predict_gb_tree_84(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.849588f) {
    if (x[1] <= 1.386281f) {
    return 0.001964f;
    } else {
    return 0.002598f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.001066f;
    } else {
    return 0.000142f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[4] <= -0.486136f) {
    return -0.001322f;
    } else {
    return -0.000567f;
    }
    } else {
    if (x[1] <= -1.712747f) {
    return -0.002666f;
    } else {
    return -0.001999f;
    }
    }
    }
}

static inline float predict_gb_tree_85(const float* x) {
    if (x[2] <= -0.110109f) {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.000960f;
    } else {
    return 0.000249f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.002338f;
    } else {
    return 0.001767f;
    }
    }
    } else {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -0.000195f;
    } else {
    return -0.000967f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.001800f;
    } else {
    return -0.002400f;
    }
    }
    }
}

static inline float predict_gb_tree_86(const float* x) {
    if (x[1] <= 0.202841f) {
    if (x[4] <= -0.486136f) {
    if (x[0] <= 1.481369f) {
    return -0.001356f;
    } else {
    return -0.002160f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000175f;
    } else {
    return -0.000648f;
    }
    }
    } else {
    if (x[1] <= 0.822167f) {
    if (x[2] <= -0.425420f) {
    return 0.000864f;
    } else {
    return 0.000224f;
    }
    } else {
    if (x[3] <= -1.303720f) {
    return 0.002105f;
    } else {
    return 0.001591f;
    }
    }
    }
}

static inline float predict_gb_tree_87(const float* x) {
    if (x[0] <= -0.147430f) {
    if (x[3] <= -0.811539f) {
    if (x[1] <= 1.386281f) {
    return 0.001432f;
    } else {
    return 0.001894f;
    }
    } else {
    if (x[2] <= -0.590583f) {
    return 0.000969f;
    } else {
    return 0.000462f;
    }
    }
    } else {
    if (x[0] <= 0.652281f) {
    if (x[0] <= 0.433585f) {
    return -0.000084f;
    } else {
    return -0.000584f;
    }
    } else {
    if (x[3] <= 0.770627f) {
    return -0.000957f;
    } else {
    return -0.001714f;
    }
    }
    }
}

static inline float predict_gb_tree_88(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[2] <= -0.849588f) {
    if (x[3] <= -1.303720f) {
    return 0.001705f;
    } else {
    return 0.001288f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.000706f;
    } else {
    return 0.000183f;
    }
    }
    } else {
    if (x[2] <= 0.824563f) {
    if (x[0] <= 0.433585f) {
    return -0.000149f;
    } else {
    return -0.000693f;
    }
    } else {
    if (x[4] <= -1.657338f) {
    return -0.001772f;
    } else {
    return -0.001313f;
    }
    }
    }
}

static inline float predict_gb_tree_89(const float* x) {
    if (x[3] <= 0.013940f) {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -1.678304f) {
    return 0.001534f;
    } else {
    return 0.000981f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.000000f;
    } else {
    return 0.000372f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[4] <= -1.657338f) {
    return -0.001595f;
    } else {
    return -0.001181f;
    }
    } else {
    if (x[0] <= 0.652281f) {
    return -0.000336f;
    } else {
    return -0.000792f;
    }
    }
    }
}

static inline float predict_gb_tree_90(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[1] <= 0.656533f) {
    return 0.000704f;
    } else {
    return 0.001221f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return 0.000000f;
    } else {
    return 0.000334f;
    }
    }
    } else {
    if (x[4] <= -0.854546f) {
    if (x[2] <= 1.661639f) {
    return -0.001063f;
    } else {
    return -0.001436f;
    }
    } else {
    if (x[4] <= -0.486136f) {
    return -0.000713f;
    } else {
    return -0.000302f;
    }
    }
    }
}

static inline float predict_gb_tree_91(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[2] <= -0.849588f) {
    if (x[4] <= 1.488630f) {
    return 0.000939f;
    } else {
    return 0.001259f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.000516f;
    } else {
    return 0.000129f;
    }
    }
    } else {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.433585f) {
    return -0.000102f;
    } else {
    return -0.000517f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.000957f;
    } else {
    return -0.001292f;
    }
    }
    }
}

static inline float predict_gb_tree_92(const float* x) {
    if (x[4] <= 0.306609f) {
    if (x[0] <= 0.652281f) {
    if (x[0] <= 0.433585f) {
    return -0.000048f;
    } else {
    return -0.000340f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.000726f;
    } else {
    return -0.001163f;
    }
    }
    } else {
    if (x[3] <= -0.811539f) {
    if (x[4] <= 1.488630f) {
    return 0.000845f;
    } else {
    return 0.001133f;
    }
    } else {
    if (x[4] <= 0.393167f) {
    return 0.000582f;
    } else {
    return 0.000269f;
    }
    }
    }
}

static inline float predict_gb_tree_93(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[3] <= -1.303720f) {
    return 0.001020f;
    } else {
    return 0.000642f;
    }
    } else {
    if (x[0] <= -0.147430f) {
    return 0.000242f;
    } else {
    return 0.000004f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[0] <= 1.481369f) {
    return -0.000789f;
    } else {
    return -0.001047f;
    }
    } else {
    if (x[4] <= -0.486136f) {
    return -0.000517f;
    } else {
    return -0.000221f;
    }
    }
    }
}

static inline float predict_gb_tree_94(const float* x) {
    if (x[2] <= -0.110109f) {
    if (x[0] <= -0.953668f) {
    if (x[1] <= 1.386281f) {
    return 0.000697f;
    } else {
    return 0.000918f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.000377f;
    } else {
    return 0.000093f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[4] <= -1.657338f) {
    return -0.000942f;
    } else {
    return -0.000710f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000077f;
    } else {
    return -0.000375f;
    }
    }
    }
}

static inline float predict_gb_tree_95(const float* x) {
    if (x[4] <= -0.156040f) {
    if (x[3] <= 0.797331f) {
    if (x[0] <= 0.652281f) {
    return -0.000177f;
    } else {
    return -0.000428f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.000639f;
    } else {
    return -0.000848f;
    }
    }
    } else {
    if (x[2] <= -0.590583f) {
    if (x[0] <= -0.881858f) {
    return 0.000726f;
    } else {
    return 0.000422f;
    }
    } else {
    if (x[4] <= 0.349912f) {
    return 0.000003f;
    } else {
    return 0.000194f;
    }
    }
    }
}

static inline float predict_gb_tree_96(const float* x) {
    if (x[0] <= 0.061474f) {
    if (x[2] <= -0.849588f) {
    if (x[4] <= 1.488630f) {
    return 0.000554f;
    } else {
    return 0.000753f;
    }
    } else {
    if (x[2] <= -0.425420f) {
    return 0.000309f;
    } else {
    return 0.000074f;
    }
    }
    } else {
    if (x[0] <= 0.652281f) {
    if (x[0] <= 0.433585f) {
    return -0.000060f;
    } else {
    return -0.000229f;
    }
    } else {
    if (x[3] <= 0.770627f) {
    return -0.000385f;
    } else {
    return -0.000669f;
    }
    }
    }
}

static inline float predict_gb_tree_97(const float* x) {
    if (x[2] <= 0.160158f) {
    if (x[2] <= -0.590583f) {
    if (x[4] <= 1.488630f) {
    return 0.000424f;
    } else {
    return 0.000678f;
    }
    } else {
    if (x[4] <= 0.349912f) {
    return 0.000002f;
    } else {
    return 0.000156f;
    }
    }
    } else {
    if (x[0] <= 0.981957f) {
    if (x[0] <= 0.652281f) {
    return -0.000145f;
    } else {
    return -0.000347f;
    }
    } else {
    if (x[0] <= 1.481369f) {
    return -0.000508f;
    } else {
    return -0.000696f;
    }
    }
    }
}

static inline float predict_gb_tree_98(const float* x) {
    if (x[0] <= 0.433585f) {
    if (x[2] <= -0.590583f) {
    if (x[3] <= -1.303720f) {
    return 0.000610f;
    } else {
    return 0.000381f;
    }
    } else {
    if (x[1] <= 0.373276f) {
    return -0.000022f;
    } else {
    return 0.000140f;
    }
    }
    } else {
    if (x[3] <= 0.770627f) {
    if (x[4] <= -0.411951f) {
    return -0.000312f;
    } else {
    return -0.000192f;
    }
    } else {
    if (x[2] <= 1.661639f) {
    return -0.000457f;
    } else {
    return -0.000626f;
    }
    }
    }
}

static inline float predict_gb_tree_99(const float* x) {
    if (x[3] <= -0.238028f) {
    if (x[4] <= 0.823386f) {
    if (x[2] <= -0.425420f) {
    return 0.000223f;
    } else {
    return 0.000053f;
    }
    } else {
    if (x[2] <= -1.442673f) {
    return 0.000549f;
    } else {
    return 0.000418f;
    }
    }
    } else {
    if (x[1] <= -0.831769f) {
    if (x[1] <= -1.712747f) {
    return -0.000564f;
    } else {
    return -0.000411f;
    }
    } else {
    if (x[0] <= 0.433585f) {
    return -0.000045f;
    } else {
    return -0.000227f;
    }
    }
    }
}

static inline float predict_gradient_boosting(const float* x) {
    float pred = GB_INIT_VAL;
    pred += GB_LEARNING_RATE * predict_gb_tree_0(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_1(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_2(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_3(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_4(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_5(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_6(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_7(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_8(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_9(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_10(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_11(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_12(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_13(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_14(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_15(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_16(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_17(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_18(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_19(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_20(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_21(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_22(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_23(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_24(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_25(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_26(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_27(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_28(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_29(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_30(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_31(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_32(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_33(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_34(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_35(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_36(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_37(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_38(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_39(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_40(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_41(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_42(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_43(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_44(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_45(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_46(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_47(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_48(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_49(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_50(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_51(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_52(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_53(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_54(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_55(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_56(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_57(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_58(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_59(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_60(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_61(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_62(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_63(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_64(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_65(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_66(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_67(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_68(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_69(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_70(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_71(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_72(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_73(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_74(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_75(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_76(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_77(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_78(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_79(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_80(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_81(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_82(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_83(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_84(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_85(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_86(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_87(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_88(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_89(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_90(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_91(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_92(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_93(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_94(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_95(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_96(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_97(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_98(x);
    pred += GB_LEARNING_RATE * predict_gb_tree_99(x);
    return pred;
}

#endif // GRADIENT_BOOSTING_MODEL_H
