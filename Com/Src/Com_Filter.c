#include "Com_Filter.h"

int16_t Com_LPF(int16_t last_output, int16_t current_measure, float alpha)
{
    return (int16_t)(alpha * last_output + (1 - alpha) * current_measure);
}

/* 卡尔曼滤波 https://www.mwrf.net/tech/basic/2023/30081.html */

/* 卡尔曼滤波参数 */
KalmanFilter_Struct kfs[3] = {
    {0.02, 0, 0, 0, 0.001, 0.543},
    {0.02, 0, 0, 0, 0.001, 0.543},
    {0.02, 0, 0, 0, 0.001, 0.543}};
int16_t Common_Filter_KalmanFilter(KalmanFilter_Struct *kf, int16_t input)
{
    kf->Now_P = kf->LastP + kf->Q;
    kf->Kg = kf->Now_P / (kf->Now_P + kf->R);
    kf->out = kf->out + kf->Kg * (input - kf->out);
    kf->LastP = (1 - kf->Kg) * kf->Now_P;
    return kf->out;
}
