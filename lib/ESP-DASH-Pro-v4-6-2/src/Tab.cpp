#include "Tab.h"

Tab::Tab(ESPDash *dashboard, const char* title) {
  _dashboard = dashboard;
  _id = dashboard->nextId();
  _title = title;
  _dashboard->add(this);
}

void Tab::update(const char* title) {
  _title = title;
  _changed = true;
}

void Tab::setDisplay(bool display) {
  if(_display == display)
    return;
  _display = display;
  _changed = true;
}

Tab::~Tab() {
  _dashboard->remove(this);
}