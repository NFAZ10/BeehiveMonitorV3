#ifndef __TAB_H
#define __TAB_H

#include <Arduino.h>
#include "ESPDashPro.h"
#include "Card.h"
#include "Chart.h"

// Forward Declaration
class Card;
class Chart;
class ESPDash;

class Tab {
    private:
        ESPDash *_dashboard = nullptr;

        uint32_t _id;
        const char *_title;
        bool _display = true; 
        bool  _changed;

    public:
        Tab(ESPDash *dashboard, const char* title);
        // true (default) to display the tab, false to hide it
        void setDisplay(bool display);
        void update(const char* title);
        ~Tab();

    friend class ESPDash;
    friend class Card;
    friend class Chart;
};

#endif