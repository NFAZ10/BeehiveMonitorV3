#ifndef __CHART_H
#define __CHART_H

#include <functional>
#include "Arduino.h"

#include "ESPDashPro.h"
#include "ArduinoJson.h"

#ifndef DASH_USE_LEGACY_CHART_STORAGE
  #define DASH_USE_LEGACY_CHART_STORAGE 0
#endif

#if DASH_USE_LEGACY_CHART_STORAGE == 1
  #include "vector.h"
#endif

// Default to Line Chart
enum {
  BAR_CHART,
  LINE_CHART,
  AREA_CHART,
  PIE_CHART
};

struct ChartNames {
  int value;
  const char* type;
};

enum GraphAxisType { INTEGER, FLOAT, CHAR, STRING };

struct ChartSize {
  uint8_t xs = DASH_DEFAULT_CHART_SIZE_XS;
  uint8_t sm = DASH_DEFAULT_CHART_SIZE_SM;
  uint8_t md = DASH_DEFAULT_CHART_SIZE_MD;
  uint8_t lg = DASH_DEFAULT_CHART_SIZE_LG;
  uint8_t xl = DASH_DEFAULT_CHART_SIZE_XL;
  uint8_t xxl = DASH_DEFAULT_CHART_SIZE_XXL;
};

// Forward Declaration
class Tab;
class ESPDash;

// Chart Class
class Chart {
  private:
    ESPDash *_dashboard = nullptr;
    Tab *_tab = nullptr;

    uint32_t _id;
    uint8_t _index;
    const char *_name;
    int   _type;
    GraphAxisType _x_axis_type;
    GraphAxisType _y_axis_type;
    bool _display = true;

    #if DASH_USE_LEGACY_CHART_STORAGE == 1
      /* X-Axis */
      Vector<int> _x_axis_i;
      Vector<float> _x_axis_f;
      Vector<String> _x_axis_s;
      /* Y-Axis */
      Vector<int> _y_axis_i;
      Vector<float> _y_axis_f;

      void emptyXAxisVectors();
      void emptyYAxisVectors();
    #else 
      /* X-Axis */
      int *_x_axis_i_ptr = nullptr;
      float *_x_axis_f_ptr = nullptr;
      const char **_x_axis_char_ptr = nullptr;
      String *_x_axis_s_ptr = nullptr;
      unsigned int _x_axis_ptr_size = 0;
      /* Y-Axis */
      int *_y_axis_i_ptr = nullptr;
      float *_y_axis_f_ptr = nullptr;
      unsigned int _y_axis_ptr_size = 0;

      void clearXAxisPointers();
      void clearYAxisPointers();
    #endif

    ChartSize _size;

    bool  _x_changed;
    bool  _y_changed;
    bool _index_changed;
    bool _tab_changed;
    bool _size_changed;
    bool _display_changed;

  public:
    Chart(ESPDash *dashboard, const int type, const char* name);
    void setIndex(uint8_t index);
    // true (default) to display the chart, false to hide it
    void setDisplay(bool display);
    void updateX(int arr_x[], size_t x_size);
    void updateX(float arr_x[], size_t x_size);
    void updateX(String arr_x[], size_t x_size);
    void updateX(const char* arr_x[], size_t x_size);
    void updateY(int arr_y[], size_t y_size);
    void updateY(float arr_y[], size_t y_size);
    // Size functions
    void setSize(const uint8_t xs, const uint8_t sm, const uint8_t md, const uint8_t lg, const uint8_t xl, const uint8_t xxl);
    void setSize(const ChartSize &size);
    // Tab functions
    void setTab(Tab *tab);
    void removeTab();
    ~Chart();

  friend class ESPDash;
};

#endif