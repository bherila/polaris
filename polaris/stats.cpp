// mtrand.cpp, see include file mtrand.h for information

#include "stdafx.h"
#include "stats.h"

real_t Sampler::urand(real_t lower, real_t upper) {
    return mts_drand(&m_state) * (upper-lower)+lower; //rds_uniform(&m_state, lower, upper);
}
