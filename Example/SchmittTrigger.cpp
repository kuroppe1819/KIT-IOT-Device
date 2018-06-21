#include "Arduino.h"
#include "SchmittTrigger.h"

SchmittTrigger::SchmittTrigger(float threshold_voltage) {
    m_threshold_voltage = threshold_voltage;
    m_variation = threshold_voltage;
}

boolean SchmittTrigger::isHigh(float current_value) {
    if (current_value > m_threshold_voltage) {
        m_threshold_voltage = 5 - m_variation;
    } else {
        m_threshold_voltage = m_variation;
    }
    return current_value > m_threshold_voltage;
}