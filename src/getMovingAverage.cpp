#include <Arduino.h>
/**
 * @brief Get the Moving Average object
 *running average for previous 1/(1-beta) = 10 samples, for beta = 0.9
 * @param newValue
 * @param prevValue
 * @param beta
 * @return float
 */
float getMovingAverage(float newValue, float prevValue, float beta)
{
    //close proximity of 0
    if ((prevValue <= 0.001 && prevValue >= -0.001) || isnan(newValue))
        return newValue;
    return beta * prevValue + (1 - beta) * newValue;
}