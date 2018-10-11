#include "Arduino.h"
#include "SchmittTrigger.h"

SchmittTrigger::SchmittTrigger(float threshold_voltage) {
    m_threshold_voltage = threshold_voltage;
    m_standard = threshold_voltage;
}

boolean SchmittTrigger::isHigh(float current_value) {
    if (current_value >= m_threshold_voltage) {
        m_threshold_voltage = m_standard / 3.0;
    } else {
        m_threshold_voltage = m_standard;
    }
    return current_value >= m_threshold_voltage;
}