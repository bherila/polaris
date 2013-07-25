#include "sampler.h"
#include <math.h>
#include <stdlib.h>

Sampler *createSampler() {
    Sampler *to_return = (Sampler*)malloc(sizeof (Sampler));
    mts_goodseed(&to_return->mState);
    return to_return;
}

void releaseSampler(Sampler *s) {
    free(s);
}

inline float urand(Sampler *s) {
    return mts_drand(&s->mState);
}

inline void urand_sphere(Sampler *s, float r, Vector4 *out) {
    float u = urand(s), v = urand(s), theta = TWO_PI * u, phi = acos(2 * v - 1),
            sphi = sin(phi);
    vec4_set(out, r * cos(theta) * sphi, r * cos(phi), r * sin(theta) * sphi, 1.0f);
}
