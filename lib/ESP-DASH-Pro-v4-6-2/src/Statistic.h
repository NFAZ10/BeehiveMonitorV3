#ifndef __STAT_H
#define __STAT_H

#include <functional>
#include "Arduino.h"
#include "vector.h"

#include "ESPDashPro.h"
#include "ArduinoJson.h"

// Forward Declaration
class ESPDash;

class Statistic {
    private:
        ESPDash *_dashboard;
        uint32_t _id;
        const char *_key;
        String _value;
        bool _display = true;
        bool _changed = false;

    public:
        Statistic(ESPDash *dashboard, const char *key, const char *value = "");
        // true (default) to display the stat, false to hide it
        void setDisplay(bool display);
        void set(const char *value);
        ~Statistic();

    friend class ESPDash;
};

#endif