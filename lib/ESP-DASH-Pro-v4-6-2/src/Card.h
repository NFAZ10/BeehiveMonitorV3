#ifndef __CARD_H
#define __CARD_H


#include <functional>
#include "Arduino.h"

#include "ESPDashPro.h"
#include "ArduinoJson.h"

struct CardNames {
  int value;
  const char* type;
};

// functions defaults to zero (number card)
enum {
  GENERIC_CARD,
  TEMPERATURE_CARD,
  HUMIDITY_CARD,
  STATUS_CARD,
  SLIDER_CARD,
  BUTTON_CARD,
  PROGRESS_CARD,
  AIR_CARD,
  ENERGY_CARD,
  TEXT_INPUT_CARD,
  JOYSTICK_CARD,
  DROPDOWN_CARD,
  IMAGE_CARD,
  PUSH_BUTTON_CARD,
  WEEK_SELECTOR_CARD,
  LINK_CARD,
  PASSWORD_CARD,
  TIME_SYNC_CARD,
  COLOR_PICKER_CARD,
};

struct CardSize {
  uint8_t xs = DASH_DEFAULT_CARD_SIZE_XS;
  uint8_t sm = DASH_DEFAULT_CARD_SIZE_SM;
  uint8_t md = DASH_DEFAULT_CARD_SIZE_MD;
  uint8_t lg = DASH_DEFAULT_CARD_SIZE_LG;
  uint8_t xl = DASH_DEFAULT_CARD_SIZE_XL;
  uint8_t xxl = DASH_DEFAULT_CARD_SIZE_XXL;
};

// Forward Declaration
class Tab;
class ESPDash;

// Card Class
class Card {
  private:
    ESPDash *_dashboard = nullptr;
    Tab *_tab = nullptr;

    uint32_t _id;
    uint8_t _index;
    const char* _name;
    int   _type;
    enum { INTEGER, FLOAT, STRING } _value_type;
    union alignas(4) {
        float _value_f;
        int _value_i;
    };
    String _value_s;
    int _value_min;
    int _value_max;
    int _value_step;
    String _symbol;
    bool _display = true;

    // Size Structure
    CardSize _size;
    
    bool _value_changed;
    bool _index_changed;
    bool _tab_changed;
    bool _size_changed;
    bool _display_changed;

    std::function<void(int value)> _callback = nullptr;
    std::function<void(const char*)> _callback_str = nullptr;
    std::function<void(int8_t x, int8_t y)> _callback_xy = nullptr;
    std::function<void()> _callback_void = nullptr;

  public:
    Card(ESPDash *dashboard, const int type, const char* name, const char* symbol = "", const int min = 0, const int max = 0, const int step = 1);
    void setIndex(uint8_t index);
    // true (default) to display the card, false to hide it
    void setDisplay(bool display);
    void attachCallback(std::function<void()> cb);
    void attachCallback(std::function<void(int)> cb);
    void attachCallback(std::function<void(const char*)> cb);
    void attachCallback(std::function<void(int8_t x, int8_t y)> cb);
    void update(int value);
    void update(int value, const char* symbol);
    void update(bool value);
    void update(bool value, const char* symbol);
    void update(float value);
    void update(float value, const char* symbol);
    void update(const char* value);
    void update(const char* value, const char* symbol);
    void update(const String &value);
    void update(const String &value, const char* symbol);
    // Size functions
    void setSize(const uint8_t xs, const uint8_t sm, const uint8_t md, const uint8_t lg, const uint8_t xl, const uint8_t xxl);
    void setSize(const CardSize &size);
    // Tab functions
    void setTab(Tab *tab);
    void removeTab();
    ~Card();
  
  friend class ESPDash;
};


#endif
