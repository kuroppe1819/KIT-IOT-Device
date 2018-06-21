#ifndef SchmittTrigger_h
#define SchmittTrigger_h

class SchmittTrigger {
    public:
      SchmittTrigger(float threshold_voltage);
      boolean isHigh(float current_value);

    private:
      float m_threshold_voltage;
      float m_standard;
};

#endif