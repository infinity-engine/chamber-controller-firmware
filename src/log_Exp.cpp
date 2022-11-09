#include <Arduino.h>
#include "functionPrototype.h"
void log_(struct ExperimentParameters *exp)
    {

        Serial.println();
        Serial.print(exp->currentRate);
        Serial.print(" ");
        Serial.print(exp->curToll);
        Serial.print(" ");
        Serial.print(exp->mode);
        Serial.print(" ");
        Serial.print(exp->powVal);
        Serial.print(" ");
        Serial.print(exp->prevTime);
        Serial.print(" ");
        Serial.print(exp->resVal);
        Serial.print(" ");
        Serial.print(exp->startTime);
        Serial.print(" ");
        Serial.print(exp->timeLimit);
        Serial.print(" ");
        Serial.print(exp->total_n_samples);
        Serial.print(" ");
        Serial.print(exp->sampleIndicator);
        Serial.println();
        for (uint8_t i = 0; i < 19; i++)
        {
            Serial.print(exp->samples_batch[i]);
            Serial.print(" ");
        }
        Serial.println();
}
