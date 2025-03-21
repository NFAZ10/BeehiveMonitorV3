#include "NetWizard.h"
#include "nwp.h"

#define NETWIZARD_STATUS_JSON_SIZE    1024
#define NETWIZARD_SCAN_JSON_SIZE      1024
#define NETWIZARD_CONFIG_JSON_SIZE    1024
#define NETWIZARD_CONNECT_JSON_SIZE   512

// Integral type to string pairs events
// ID, type
struct NetWizardParameterTypeNames paramTypes[] = {
  {NW_HEADER, "h"},
  {NW_DIVIDER, "d"},
  {NW_SPACER, "s"},
  {NW_INPUT, "i"},
};

NetWizard::NetWizard(NETWIZARD_WEBSERVER *server) {
  _server = server;
  _nw.sta.configured = false;
  _nw.portal.config.counter = 0;
}

void NetWizard::setAuthentication(String& username, String& password) {
  this->setAuthentication(username.c_str(), password.c_str());
}

void NetWizard::setAuthentication(const char* username, const char* password) {
  if (strlen(username) > 0 && strlen(password) > 0) {
    _nw.portal.auth.enabled = true;
    _nw.portal.auth.username = username;
    _nw.portal.auth.password = password;
  }
}

void NetWizard::setHostname(String& hostname) {
  this->setHostname(hostname.c_str());
}

void NetWizard::setHostname(const char* hostname) {
  _nw.hostname = hostname;
}

void NetWizard::setStrategy(NetWizardStrategy strategy) {
  _nw.strategy = strategy;
}

void NetWizard::setConnectTimeout(unsigned long timeout) {
  _nw.sta.timeout = timeout;
}

void NetWizard::setPortalTimeout(unsigned long timeout) {
  _nw.portal.timeout = timeout;
}

void NetWizard::setCredentials(const char* ssid, const char* password) {
  _nw.sta.ssid = ssid;
  _nw.sta.password = password;
  _nw.sta.configured = true;
  _saveSTACredentials();
}

void NetWizard::autoConnect(const char* ssid, const char* password) {
  // Reset network
  if (WiFi.getMode() != WIFI_OFF) {
    // Disconnect before turning off radio
    if (WiFi.status() == WL_CONNECTED) {
      _disconnect();
    }
    // Turn off radio
    WiFi.mode(WIFI_OFF);
  }
  // Load AP credentials
  _nw.portal.ap.ssid = ssid;
  _nw.portal.ap.password = password;
  // Load STA credentials
  _loadSTACredentials();
  NETWIZARD_DEBUG_MSG("Loaded saved credentials\n");

  // Flag for captive portal
  bool start_captive_portal = true;

  // Check if saved credentials are valid and network is connected
  if (this->isConfigured()) {
    NETWIZARD_DEBUG_MSG("Connecting to saved credentials\n");
    NETWIZARD_DEBUG_MSG("SSID: " + _nw.sta.ssid + " \n");
    Serial.printf("Connecting to %s, %s\n", _nw.sta.ssid.c_str(), _nw.sta.password.c_str());
    // Set hostname
    if (_nw.hostname != "") {
      WiFi.setHostname(_nw.hostname.c_str());
    }
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);

    // Check if connected within connection timeout
    unsigned long startMillis = millis();
    unsigned long lastConnectMillis = startMillis;

    while ((unsigned long)(millis() - startMillis) < _nw.sta.timeout) {
      yield();
      this->loop();

      // Check if connected
      if (_nw.status == NetWizardConnectionStatus::CONNECTED) {
        break;
      }

      // Reconnect if not connected at interval of 5 seconds
      if (millis() - lastConnectMillis > 10000) {
        if (
          _nw.status == NetWizardConnectionStatus::DISCONNECTED
          || _nw.status == NetWizardConnectionStatus::CONNECTION_LOST
          || _nw.status == NetWizardConnectionStatus::CONNECTION_FAILED
        ) {
          NETWIZARD_DEBUG_MSG("Trying to connect again...\n");
          // Set hostname
          if (_nw.hostname != "") {
            WiFi.setHostname(_nw.hostname.c_str());
          }
          WiFi.mode(WIFI_STA);
          WiFi.persistent(false);
          _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);
          // last connect time
          lastConnectMillis = millis();
        }
      }
    }

    if (_nw.status != NetWizardConnectionStatus::CONNECTED) {
      // No connection within timeout
      NETWIZARD_DEBUG_MSG("Failed to connect\n");
    } else {
      NETWIZARD_DEBUG_MSG("Connected to saved credentials\n");
      start_captive_portal = false;
    }
  }
  
  if (start_captive_portal) {
    // Reset before starting Portal
    _stopPortal();
    // Start Portal
    NETWIZARD_DEBUG_MSG("Starting Captive Portal.\n");
    _startPortal();

    if (_nw.strategy == NetWizardStrategy::BLOCKING) {
      NETWIZARD_DEBUG_MSG("Entering blocking strategy loop.\n");
      // Loop
      while (_nw.portal.active && _nw.portal.blocking) {
        this->loop();
        yield();
      }
      // Stop portal once out of loop
      if (_nw.portal.active) {
        _stopPortal();
      }
    }
  }
}

void NetWizard::onConnectionStatus(NetWizardOnConnectionStatusCallback callback) {
  _nw.status_cb = callback;
}

void NetWizard::onPortalState(NetWizardOnPortalStateCallback callback) {
  _nw.portal.state_cb = callback;
}

void NetWizard::onConfig(NetWizardOnConfigCallback callback) {
  _nw.portal.config.callback = callback;
}

bool NetWizard::isConfigured() {
  return _nw.sta.configured;
}

NetWizardConnectionStatus NetWizard::getConnectionStatus() {
  return _nw.status;
}

NetWizardPortalState NetWizard::getPortalState() {
  return _nw.portal.state;
}

const char* NetWizard::getSSID() {
  return _nw.sta.ssid.c_str();
}

const char* NetWizard::getPassword() {
  return _nw.sta.password.c_str();
}

void NetWizard::getBSSID(uint8_t* bssid) {
  // return current WiFi BSSID
  WiFi.BSSID(bssid);
}

uint8_t NetWizard::getChannel() {
  return WiFi.channel();
}

IPAddress NetWizard::localIP() {
  return WiFi.localIP();
}

IPAddress NetWizard::gatewayIP() {
  return WiFi.gatewayIP();
}

IPAddress NetWizard::subnetMask() {
  return WiFi.subnetMask();
}

bool NetWizard::connect() {
  if (_nw.sta.configured) {
    _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);
    return true;
  }
  return false;
}

bool NetWizard::connect(const char* ssid, const char* password) {
  // clear existing credentials
  _nw.sta.configured = true;
  _nw.sta.ssid = ssid;
  _nw.sta.password = password;
  // save credentials
  _saveSTACredentials();
  // connect
  _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);
  return true;
}

void NetWizard::erase() {
  Preferences prefs;
  prefs.begin("nw_creds", false);
  prefs.putString("ssid", "");
  prefs.putString("password", "");
  // prefs.putChar("b0", _nw.sta.bssid[0]);
  // prefs.putChar("b1", _nw.sta.bssid[1]);
  // prefs.putChar("b2", _nw.sta.bssid[2]);
  // prefs.putChar("b3", _nw.sta.bssid[3]);
  // prefs.putChar("b4", _nw.sta.bssid[4]);
  // prefs.putChar("b5", _nw.sta.bssid[5]);
  // prefs.putUChar("channel", _nw.sta.channel);
  prefs.end();
}

void NetWizard::disconnect() {
  _disconnect();
}

void NetWizard::reset() {
  _nw.sta.configured = false;
  _nw.sta.ssid = "";
  _nw.sta.password = "";
  _saveSTACredentials();

  // disconnect from wifi 
  _disconnect();
  _nw.status = NetWizardConnectionStatus::DISCONNECTED;

  // stop captive portal
  if (_nw.portal.active) {
    // set exit flag
    _nw.portal.exit.flag = true;
    _nw.portal.exit.millis = millis();
  }
}

void NetWizard::loop() {
  #if NETWIZARD_USE_ASYNC_WEBSERVER == 0
    if (_server != nullptr) {
      _server->handleClient();    
    }
  #endif
  
  if (_dns != nullptr) {
    // Check if DNS server is running
    if (!_dns_running) {
      delay(100);  // Add a small delay here before checking ESP32 condition
      #if defined(ESP32)
        if (WiFi.AP.hasIP()) {
          _dns->start(53, "*", WiFi.AP.localIP());
          _dns_running = true;
          NETWIZARD_DEBUG_MSG("Started DNS Server (loop)\n");
        }
      #else
       _dns->start(53, "*", WiFi.softAPIP());
       _dns_running = true;
        NETWIZARD_DEBUG_MSG("Started DNS Server (loop)\n");
      #endif
    } else {
      _dns->processNextRequest();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    _nw.status = NetWizardConnectionStatus::CONNECTED;
  } else if (WiFi.status() == WL_DISCONNECTED) {
    _nw.status = NetWizardConnectionStatus::DISCONNECTED;
  } else if (WiFi.status() == WL_CONNECT_FAILED) {
    _nw.status = NetWizardConnectionStatus::CONNECTION_FAILED;
  } else if (WiFi.status() == WL_CONNECTION_LOST) {
    _nw.status = NetWizardConnectionStatus::CONNECTION_LOST;
  } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
    _nw.status = NetWizardConnectionStatus::NOT_FOUND;
  }

  // Check portal timeout
  if (_nw.portal.active) {
    if (WiFi.softAPgetStationNum() > 0 || (_nw.portal.state == NetWizardPortalState::CONNECTING_WIFI || _nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION) || ((unsigned long)(millis() - _nw.portal.start_millis) < _nw.portal.timeout)) {
      if (WiFi.softAPgetStationNum() > 0) {
        _nw.portal.start_millis = millis();
      }
      _nw.portal.blocking = true;
    } else {
      _stopPortal();
      _nw.portal.state = NetWizardPortalState::IDLE;
      _nw.portal.blocking = false;
      NETWIZARD_DEBUG_MSG("Portal timeout. Disconnected.\n");
    }

    // Go through portal steps
    switch (_nw.portal.state) {
      case NetWizardPortalState::IDLE:
        break;
      case NetWizardPortalState::CONNECTING_WIFI:
        // Disconnect from WiFi
        _disconnect();
        NETWIZARD_DEBUG_MSG("Connecting to temporary credentials\n");
        NETWIZARD_DEBUG_MSG("SSID: " + _nw.portal.sta.ssid + " \n");
        NETWIZARD_DEBUG_MSG("Password: " + _nw.portal.sta.password + " \n");
        // Connect to temporary credentials
        WiFi.persistent(false);
        _connect(_nw.portal.sta.ssid.c_str(), _nw.portal.sta.password.c_str(), false);
        _nw.portal.connect_millis = millis();
        _nw.portal.state = NetWizardPortalState::WAITING_FOR_CONNECTION;
        break;
      case NetWizardPortalState::WAITING_FOR_CONNECTION: {
        if (WiFi.status() == WL_CONNECTED) {
          NETWIZARD_DEBUG_MSG("Connected to temporary credentials\n");
          // Copy temporary credentials to STA credentials
          _nw.sta.ssid = _nw.portal.sta.ssid.c_str();
          _nw.sta.password = _nw.portal.sta.password.c_str();
          _nw.sta.configured = true;
          // Save credentials
          _saveSTACredentials();
          // Exit portal
          // _nw.portal.exit.millis = millis();
          // _nw.portal.exit.flag = true;
          _nw.portal.state = NetWizardPortalState::SUCCESS;
        } else {
          if ((unsigned long)(millis() - _nw.portal.connect_millis) > NETWIZARD_CONNECT_TIMEOUT) {
            NETWIZARD_DEBUG_MSG("Error: connection to temporary credentials timeout!\n");
            _nw.portal.sta.ssid = "";
            _nw.portal.sta.password = "";
            // Disconnect from WiFi
            _disconnect();
            _nw.portal.state = NetWizardPortalState::TIMEOUT;
          }
        }
        break;
      }
      case NetWizardPortalState::SUCCESS:
      case NetWizardPortalState::FAILED:
      case NetWizardPortalState::TIMEOUT:
        break;
    }
  }

  // If exit flag is set and more than NETWIZARD_EXIT_TIMEOUT milliseconds have passed, stop portal
  if (_nw.portal.exit.flag && ((unsigned long)(millis() - _nw.portal.exit.millis) > NETWIZARD_EXIT_TIMEOUT)) {
    _stopPortal();
    _nw.portal.exit.flag = false;
    _nw.portal.exit.millis = 0;
  }

  // Connection Status Listener
  static NetWizardConnectionStatus _last_status;
  if (_nw.status_cb != nullptr && _nw.status != _last_status) {
    _last_status = _nw.status;
    _nw.status_cb(_nw.status);
  }

  // Portal State Listener
  static NetWizardPortalState _last_state;
  if (_nw.portal.state_cb != nullptr && _nw.portal.state != _last_state) {
    _last_state = _nw.portal.state;
    _nw.portal.state_cb(_nw.portal.state);
  }
}

void NetWizard::startPortal() {
  return _startPortal();
}

void NetWizard::stopPortal() {
  return _stopPortal();
}

uint32_t NetWizard::nextId() {
  return _nw.portal.config.counter++;
}

void NetWizard::addParameter(NetWizardParameter* parameter) {
  _nw.portal.config.parameters.PushBack(parameter);
}

void NetWizard::removeParameter(NetWizardParameter* parameter) {
  for(int i=0; i < _nw.portal.config.parameters.Size(); i++){
    NetWizardParameter *p = _nw.portal.config.parameters[i];
    if(p->_id == parameter->_id){
      _nw.portal.config.parameters.Erase(i);
      return;
    }
  }
}

void NetWizard::_connect(const char* ssid, const char* password, bool autoreconnect) {
  #if defined(ESP8266) || defined(ESP32)
    // Set auto reconnect
    if (autoreconnect) {
      WiFi.setAutoReconnect(true);
    }
  #endif
  // Connect to WiFi
  WiFi.begin(ssid, password);
  _nw.status = NetWizardConnectionStatus::CONNECTING;
}

void NetWizard::_disconnect() {
  // Disconnect from WiFi
  #if defined(ESP8266)
    WiFi.disconnect(false, true);
  #elif defined(ESP32)
    WiFi.disconnect(false, true, 200);
  #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
    WiFi.disconnect(false);
  #endif
  _nw.status = NetWizardConnectionStatus::DISCONNECTED;
}

void NetWizard::_saveSTACredentials() {
  Preferences prefs;
  prefs.begin("nw_creds", false);
  prefs.putString("ssid", _nw.sta.ssid);
  prefs.putString("password", _nw.sta.password);
  // prefs.putChar("b0", _nw.sta.bssid[0]);
  // prefs.putChar("b1", _nw.sta.bssid[1]);
  // prefs.putChar("b2", _nw.sta.bssid[2]);
  // prefs.putChar("b3", _nw.sta.bssid[3]);
  // prefs.putChar("b4", _nw.sta.bssid[4]);
  // prefs.putChar("b5", _nw.sta.bssid[5]);
  // prefs.putUChar("channel", _nw.sta.channel);
  prefs.end();
}

void NetWizard::_loadSTACredentials() {
  Preferences prefs;
  prefs.begin("nw_creds", true);
  String ssid = prefs.getString("ssid", "");
  String password = prefs.getString("password", "");
  // set ssid
  if (ssid.length() > 0) {
    _nw.sta.configured = true;
    _nw.sta.ssid = ssid;
  } else {
    _nw.sta.ssid = "";
    _nw.sta.configured = false;
  }
  // set password
  if (password.length() > 0) {
    _nw.sta.password = password;
  } else {
    _nw.sta.password = "";
  }
  // set bssid
  // for (int i = 0; i < 6; i++) {
  //   _nw.sta.bssid[i] = (uint8_t)(prefs.getChar(("b" + String(i)).c_str()));
  // }

  // set channel
  // _nw.sta.channel = prefs.getUChar("channel", 0);
  prefs.end();
}

bool NetWizard::_isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

void NetWizard::_generateStatusJson(String& str) {
  // Return NetWizard Status
  #if ARDUINOJSON_VERSION_MAJOR == 7
    JsonDocument json;
  #else
    DynamicJsonDocument json(NETWIZARD_STATUS_JSON_SIZE);
  #endif
  json["conn"]["status"] = (uint8_t)_nw.status;
  json["conn"]["ssid"] = _nw.sta.ssid.c_str();
  json["conn"]["mac"] = WiFi.macAddress();
  json["conn"]["ip"] = WiFi.localIP().toString();
  json["portal"]["state"] = (uint8_t)_nw.portal.state;

  // Serialize JSON to string
  serializeJson(json, str);

  // Clear JSON document
  json.clear();
}

void NetWizard::_generateSchemaJson(String& str) {
  #if ARDUINOJSON_VERSION_MAJOR == 7
    JsonDocument json;
  #else
    DynamicJsonDocument json(NETWIZARD_STATUS_JSON_SIZE);
  #endif
  JsonArray arr = json.to<JsonArray>();

  // Create nested objects
  for(int i=0; i < _nw.portal.config.parameters.Size(); i++){
    NetWizardParameter *p = _nw.portal.config.parameters[i];
    #if ARDUINOJSON_VERSION_MAJOR == 7
      JsonObject obj = arr.add<JsonObject>();
    #else
      JsonObject obj = arr.createNestedObject();
    #endif
    obj["id"] = p->_id;
    obj["t"] = paramTypes[p->_type].type;
    obj["n"] = p->_name;
    obj["v"] = p->_value;
    obj["p"] = p->_placeholder;
    obj["r"] = p->_required;
  }

  // Serialize JSON to string
  serializeJson(json, str);

  // Clear JSON document
  json.clear();
}

void NetWizard::_generateScanJson(String& str) {
  // Return NetWizard Scan
  #if ARDUINOJSON_VERSION_MAJOR == 7
    JsonDocument json;
  #else
    DynamicJsonDocument json(NETWIZARD_SCAN_JSON_SIZE);
  #endif
  JsonArray arr = json.to<JsonArray>();
  // Add scan result to JSON array
  for (uint16_t i = 0; i < WiFi.scanComplete(); i++) {
    #if ARDUINOJSON_VERSION_MAJOR == 7
      JsonObject obj = arr.add<JsonObject>();
    #else
      JsonObject obj = arr.createNestedObject();
    #endif
    obj["s"] = WiFi.SSID(i);
    #if defined(ESP8266) || defined(ESP32)
      obj["b"] = WiFi.BSSIDstr(i);
    #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
      String bssid;
      uint8_t b[6];
      WiFi.BSSID(i, b);
      for (int j = 0; j < 6; j++) {
        bssid += String(b[j], HEX);
        if (j < 5) {
          bssid += ":";
        }
      }
      obj["b"] = bssid;
    #endif
    obj["r"] = WiFi.RSSI(i);
    obj["c"] = WiFi.channel(i);

    // {
    //   OPEN = 0,
    //   WEP,
    //   WPA_PSK,
    //   WPA2_PSK,
    //   WPA_WPA2_PSK,
    //   ENTERPRISE,
    //   WPA2_ENTERPRISE,
    //   WPA3_PSK,
    //   WPA2_WPA3_PSK,
    //   WAPI_PSK,
    //   OWE,
    //   WPA3_ENT_192,
    //   WPA3_EXT_PSK,
    //   WPA3_EXT_PSK_MIXED_MODE,
    //   MAX,
    // }

    NetWizardEncryptionType enc = NetWizardEncryptionType::OPEN;
    #if defined(ESP8266) || defined(ESP32)
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
          enc = NetWizardEncryptionType::OPEN;
          break;
        case WIFI_AUTH_WEP:
          enc = NetWizardEncryptionType::WEP;
          break;
        case WIFI_AUTH_WPA_PSK:
          enc = NetWizardEncryptionType::WPA_PSK;
          break;
        case WIFI_AUTH_WPA2_PSK:
          enc = NetWizardEncryptionType::WPA2_PSK;
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          enc = NetWizardEncryptionType::WPA_WPA2_PSK;
          break;
        // case WIFI_AUTH_ENTERPRISE:
        //   enc = NetWizardEncryptionType::ENTERPRISE;
        //   break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          enc = NetWizardEncryptionType::WPA2_ENTERPRISE;
          break;
        case WIFI_AUTH_WPA3_PSK:
          enc = NetWizardEncryptionType::WPA3_PSK;
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          enc = NetWizardEncryptionType::WPA2_WPA3_PSK;
          break;
        case WIFI_AUTH_WAPI_PSK:
          enc = NetWizardEncryptionType::WAPI_PSK;
          break;
        case WIFI_AUTH_OWE:
          enc = NetWizardEncryptionType::OWE;
          break;
        case WIFI_AUTH_WPA3_ENT_192:
          enc = NetWizardEncryptionType::WPA3_ENT_192;
          break;
        // case WIFI_AUTH_WPA3_EXT_PSK:
        //   enc = NetWizardEncryptionType::WPA3_EXT_PSK;
        //   break;
        // case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
        //   enc = NetWizardEncryptionType::WPA3_EXT_PSK_MIXED_MODE;
        //   break;
        case WIFI_AUTH_MAX:
          enc = NetWizardEncryptionType::MAX;
          break;
        default:
          enc = NetWizardEncryptionType::UNKNOWN;
          break;
      }
    #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
      switch (WiFi.encryptionType(i)) {
        case ENC_TYPE_NONE:
          enc = NetWizardEncryptionType::OPEN;
          break;
        case ENC_TYPE_TKIP:
          enc = NetWizardEncryptionType::WPA_PSK;
          break;
        case ENC_TYPE_CCMP:
          enc = NetWizardEncryptionType::WPA2_PSK;
          break;
        case ENC_TYPE_AUTO:
          enc = NetWizardEncryptionType::WPA_WPA2_PSK;
          break;
        default:
          enc = NetWizardEncryptionType::UNKNOWN;
          break;
      }
    #endif
    obj["e"] = (uint8_t)enc;
  }
  serializeJson(json, str);
  json.clear();
}

bool NetWizard::_parseConfigJson(JsonArray& arr) {
  // validate parameters
  for (uint8_t i = 0; i < arr.size(); i++) {
    JsonObject obj = arr[i];
    // check if id and value are present
    if (!obj["id"].is<JsonVariant>() || !obj["v"].is<JsonVariant>()) {
      return false;
    }
    // value must be a string
    if (!obj["v"].is<const char*>()) {
      return false;
    }
  }
  
  // Parse parameters
  for (uint8_t i = 0; i < arr.size(); i++) {
    JsonObject obj = arr[i];
    for (int j = 0; j < _nw.portal.config.parameters.Size(); j++) {
      NetWizardParameter *p = _nw.portal.config.parameters[j];
      if (p->_id == obj["id"].as<uint32_t>()) {
        p->_value = obj["v"].as<const char*>();
        break;
      }
    }
  }

  if (_nw.portal.config.callback != nullptr) {
    return _nw.portal.config.callback();
  } else {
    return true;
  }
}

bool NetWizard::_parseCredentialsJson(JsonObject& obj) {
  if (
    obj["ssid"].is<const char*>()
    && obj["ssid"].is<const char*>()
    && obj["psk"].is<const char*>()
    && obj["psk"].is<const char*>()
  ) {
    _nw.portal.sta.ssid = obj["ssid"].as<const char*>();
    _nw.portal.sta.password = obj["psk"].as<const char*>();

    // if (
    //   obj.containsKey("bssid")
    //   && obj["bssid"].is<JsonArray>()
    //   && obj.containsKey("channel")
    //   && obj["channel"].is<uint8_t>()
    // ) {
    //   for (uint8_t i = 0; i < obj["bssid"].size(); i++) {
    //     _nw.portal.sta.bssid[i] = obj["bssid"][i];
    //   }
    //   _nw.portal.sta.channel = obj["channel"];
    // }

    // Set portal state
    _nw.portal.state = NetWizardPortalState::CONNECTING_WIFI;
    
    return true;
  } else {
    return false;
  }
}

void NetWizard::_restartScan() {
  WiFi.scanDelete();
  #if defined(ESP32)
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  #endif
  WiFi.scanNetworks(true);
  _nw.portal.scan.start_millis = millis();
  _nw.portal.scan.active = true;
}

void NetWizard::_startHTTP() {
  // Start HTTP server
  #if NETWIZARD_USE_ASYNC_WEBSERVER == 1

    _index_handler = &_server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){
      if(_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())){
        return request->requestAuthentication();
      }
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", NETWIZARD_HTML, sizeof(NETWIZARD_HTML));
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }).setFilter(this->_onAPFilter);

    _status_handler = &_server->on("/netwizard/status", HTTP_GET, [&](AsyncWebServerRequest *request){
      if(_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())){
        return request->requestAuthentication();
      }
      String output;
      _generateStatusJson(output);
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", output);
      request->send(response);
    }).setFilter(this->_onAPFilter);

    _schema_handler = &_server->on("/netwizard/schema", HTTP_GET, [&](AsyncWebServerRequest *request){
      if(_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())){
        return request->requestAuthentication();
      }
      String output;
      _generateSchemaJson(output);
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", output);
      request->send(response);
    }).setFilter(this->_onAPFilter);

    _scan_handler = &_server->on("/netwizard/scan", HTTP_GET, [&](AsyncWebServerRequest *request){
      if(_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())){
        return request->requestAuthentication();
      }

      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return request->send(503, "text/plain", "Busy");
      }

      // return scan data
      int16_t n = WiFi.scanComplete();
      #if defined(ESP8266) || defined(ESP32)
        if (n == WIFI_SCAN_RUNNING) {
          return request->send(202, "text/plain", "");
        } else if (n == WIFI_SCAN_FAILED) {
          // restart scan
          _restartScan();
          return request->send(202, "text/plain", "");
      #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
        if (!n) {
          return request->send(202, "text/plain", "");
      #endif
      } else {
        // serialize scan data
        String output;
        _generateScanJson(output);
        // restart scan
        _restartScan();
        return request->send(200, "application/json", output);
      }
    }).setFilter(this->_onAPFilter);

    _save_handler = new AsyncCallbackJsonWebHandler("/netwizard/save", [&](AsyncWebServerRequest *request, JsonVariant &json) {
      if (_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return request->requestAuthentication();
      }
      
      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return request->send(503, "text/plain", "Busy");
      }

      // check if JSON object is valid
      if (!json.is<JsonObject>() || json.size() == 0) {
        NETWIZARD_DEBUG_MSG("Invalid json\n");
        return request->send(400, "text/plain", "Invalid request data");
      }

      JsonObject obj = json.as<JsonObject>();

      if (obj["params"].is<JsonArray>()) {
        JsonArray params = obj["params"];
        if (!_parseConfigJson(params)) {
          return request->send(400, "text/plain", "Invalid request data");
        } else {
          // If we only got params, then we can set the state to SUCCESS
          if (!obj["credentials"].is<JsonObject>()) {
            _nw.portal.state = NetWizardPortalState::SUCCESS;
          }
        }
      }

      if (obj["credentials"].is<JsonObject>()) {
        JsonObject credentials = obj["credentials"];
        if (!_parseCredentialsJson(credentials)) {
          return request->send(400, "text/plain", "Invalid request data");
        }
      }

      return request->send(200, "text/plain", "OK");
    });

    _save_handler->setFilter(this->_onAPFilter);
    _server->addHandler(_save_handler);

    _clear_handler = &_server->on("/netwizard/clear", HTTP_POST, [&](AsyncWebServerRequest *request){
      if (_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return request->requestAuthentication();
      }
      
      if (_nw.portal.state == NetWizardPortalState::IDLE || _nw.portal.state == NetWizardPortalState::SUCCESS || _nw.portal.state == NetWizardPortalState::FAILED || _nw.portal.state == NetWizardPortalState::TIMEOUT) {
        _nw.portal.state = NetWizardPortalState::IDLE;
        request->send(200, "text/plain", "OK");
      } else {
        request->send(503, "text/plain", "Busy");
      }
    }).setFilter(this->_onAPFilter);

    _exit_handler = &_server->on("/netwizard/exit", HTTP_POST, [&](AsyncWebServerRequest *request){
      if(_nw.portal.auth.enabled && !request->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())){
        return request->requestAuthentication();
      }

      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return request->send(503, "text/plain", "Busy");
      }

      if (!_nw.portal.exit.flag) {
        _nw.portal.exit.millis = millis();
        _nw.portal.exit.flag = true;
      }
      return request->send(200, "text/plain", "OK");
    }).setFilter(this->_onAPFilter);

    _server->onNotFound([](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
      response->addHeader("Location", String("http://") + request->client()->localIP().toString());
      request->send(response);
    });

  #else
  
    _index_handler = &_server->on("/", HTTP_GET, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }
      _server->sendHeader("Content-Encoding", "gzip");
      _server->send_P(200, "text/html", (const char*)NETWIZARD_HTML, sizeof(NETWIZARD_HTML));
    }).setFilter(this->_onAPFilter);

    _status_handler = &_server->on("/netwizard/status", HTTP_GET, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }

      String output;
      _generateStatusJson(output);
      _server->send(200, "application/json", output);
    }).setFilter(this->_onAPFilter);

    _schema_handler = &_server->on("/netwizard/schema", HTTP_GET, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }

      // generate schema
      String output;
      _generateSchemaJson(output);
      _server->send(200, "application/json", output);
    }).setFilter(this->_onAPFilter);

    _scan_handler = &_server->on("/netwizard/scan", HTTP_GET, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }

      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return _server->send(503, "text/plain", "Busy");
      }

      // return scan data
      int16_t n = WiFi.scanComplete();
      #if defined(ESP8266) || defined(ESP32)
        if (n == WIFI_SCAN_RUNNING) {
          _server->send(202, "application/json", "[]");
          return _server->client().stop(); // Stop is needed because we sent no content length
        } else if (n == WIFI_SCAN_FAILED) {
          // restart scan
          _restartScan();
          _server->send(202, "application/json", "[]");
          return _server->client().stop(); // Stop is needed because we sent no content length
      #elif defined (TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
        if (!n) {
        _server->send(202, "application/json", "[]");
        return _server->client().stop(); // Stop is needed because we sent no content length
      #endif
      } else {
        // serialize scan data
        String output;
        _generateScanJson(output);
        // restart scan
        _restartScan();
        return _server->send(200, "application/json", output);
      }
    }).setFilter(this->_onAPFilter);

    _save_handler = &_server->on("/netwizard/save", HTTP_POST, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }
      
      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return _server->send(503, "text/plain", "Busy");
      }

      // check if JSON object is available
      if (_server->hasArg("plain") == false) {
        NETWIZARD_DEBUG_MSG("Invalid request data\n");
        return _server->send(400, "text/plain", "Request body not found");
      }
      
      #if ARDUINOJSON_VERSION_MAJOR == 7
        JsonDocument json;
      #else
        DynamicJsonDocument json(NETWIZARD_CONFIG_JSON_SIZE);
      #endif

      // parse JSON object
      deserializeJson(json, _server->arg("plain"));

      // check if JSON object is valid
      if (!json.is<JsonObject>() || json.size() == 0) {
        NETWIZARD_DEBUG_MSG("Invalid json\n");
        return _server->send(400, "text/plain", "Invalid request data");
      }

      JsonObject obj = json.as<JsonObject>();

      if (obj["params"].is<JsonArray>()) {
        JsonArray params = obj["params"];
        if (!_parseConfigJson(params)) {
          return _server->send(400, "text/plain", "Invalid request data");
        } else {
          // If we only got params, then we can set the state to SUCCESS
          if (!obj["credentials"].is<JsonObject>()) {
            _nw.portal.state = NetWizardPortalState::SUCCESS;
          }
        }
      }

      if (obj["credentials"].is<JsonObject>()) {
        JsonObject credentials = obj["credentials"];
        if (!_parseCredentialsJson(credentials)) {
          return _server->send(400, "text/plain", "Invalid request data");
        }
      }

      return _server->send(200, "text/plain", "OK");
    }).setFilter(this->_onAPFilter);

    _clear_handler = &_server->on("/netwizard/clear", HTTP_POST, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }
      
      if (_nw.portal.state == NetWizardPortalState::IDLE || _nw.portal.state == NetWizardPortalState::SUCCESS || _nw.portal.state == NetWizardPortalState::FAILED || _nw.portal.state == NetWizardPortalState::TIMEOUT) {
        _nw.portal.state = NetWizardPortalState::IDLE;
        _server->send(200, "text/plain", "OK");
      } else {
        _server->send(503, "text/plain", "Busy");
      }
    }).setFilter(this->_onAPFilter);

    _exit_handler = &_server->on("/netwizard/exit", HTTP_POST, [&](){
      if (_nw.portal.auth.enabled && !_server->authenticate(_nw.portal.auth.username.c_str(), _nw.portal.auth.password.c_str())) {
        return _server->requestAuthentication();
      }

      if (_nw.portal.state == NetWizardPortalState::WAITING_FOR_CONNECTION || _nw.portal.state == NetWizardPortalState::CONNECTING_WIFI) {
        return _server->send(503, "text/plain", "Busy");
      }

      // set exit flag
      if (!_nw.portal.exit.flag) {
        _nw.portal.exit.millis = millis();
        _nw.portal.exit.flag = true;
      }
      _server->send(200, "text/plain", "OK");
    }).setFilter(this->_onAPFilter);

    _server->onNotFound([&](){
      _server->sendHeader("Location", String("http://") + _server->client().localIP().toString(), true);
      _server->send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
      _server->client().stop(); // Stop is needed because we sent no content length
    });
  #endif

  _server->begin();
  _server_running = true;
}

void NetWizard::_stopHTTP() {
  if (!_server_running) {
    return;
  }

  if (_index_handler != nullptr) {
    _server->removeHandler(_index_handler);
    _index_handler = nullptr;
  }

  if (_status_handler != nullptr) {
    _server->removeHandler(_status_handler);
    _status_handler = nullptr;
  }

  if (_schema_handler != nullptr) {
    _server->removeHandler(_schema_handler);
    _schema_handler = nullptr;
  }

  if (_scan_handler != nullptr) {
    _server->removeHandler(_scan_handler);
    _scan_handler = nullptr;
  }

  if (_save_handler != nullptr) {
    _server->removeHandler(_save_handler);
    _save_handler = nullptr;
  }

  if (_clear_handler != nullptr) {
    _server->removeHandler(_clear_handler);
    _clear_handler = nullptr;
  }

  if (_exit_handler != nullptr) {
    _server->removeHandler(_exit_handler);
    _exit_handler = nullptr;
  }

  _server->onNotFound(nullptr);
  _server_running = false;
}

void NetWizard::_startPortal() {
  // Set hostname
  if (_nw.hostname != "") {
    WiFi.setHostname(_nw.hostname.c_str());
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.persistent(false);
  if (this->isConfigured()) {
    // If connection status is not NOT_FOUND, then disconnect
    if (_nw.status == NetWizardConnectionStatus::NOT_FOUND) {
      NETWIZARD_DEBUG_MSG("Configured connection not found. Starting portal with AP only.\n");
      _disconnect();
    } else {
      NETWIZARD_DEBUG_MSG("Starting portal in AP+STA mode\n");
      _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);
    }
    WiFi.softAP(_nw.portal.ap.ssid.c_str(), _nw.portal.ap.password.c_str());
  } else {
    WiFi.softAP(_nw.portal.ap.ssid.c_str(), _nw.portal.ap.password.c_str());
  }

  // Start DNS
  if (_dns == nullptr) {
    _dns = new DNSServer();
    _dns->setErrorReplyCode(DNSReplyCode::NoError);
    NETWIZARD_DEBUG_MSG("Initialized DNS Server\n");
    #if defined(ESP32)
      if (WiFi.AP.hasIP()) {
        NETWIZARD_DEBUG_MSG("AP IP:\n");
        NETWIZARD_DEBUG_MSG(WiFi.AP.localIP().toString() + "\n");
        _dns->start(53, "*", WiFi.AP.localIP());
        _dns_running = true;      
        NETWIZARD_DEBUG_MSG("Started DNS Server\n");
      }
    #else
      NETWIZARD_DEBUG_MSG("AP IP:\n");
      NETWIZARD_DEBUG_MSG(WiFi.softAPIP().toString() + "\n");
      _dns->start(53, "*", WiFi.softAPIP());
      _dns_running = true;
      NETWIZARD_DEBUG_MSG("Started DNS Server\n");
    #endif
  }
  
  // Start HTTP
  _startHTTP();
  NETWIZARD_DEBUG_MSG("Started HTTP\n");

  // Store time of Portal start
  _nw.portal.active = true;
  _nw.portal.blocking = true;
  _nw.portal.start_millis = millis();
}

void NetWizard::_stopPortal() {
  // Stop HTTP
  _stopHTTP();
  NETWIZARD_DEBUG_MSG("Stopped HTTP\n");

  // Stop DNS
  if (_dns != nullptr) {
    _dns->stop();
    delete _dns;
    _dns = nullptr;
    NETWIZARD_DEBUG_MSG("Stopped DNS Server\n");
  }
  // Stop Portal
  WiFi.softAPdisconnect(true);
  if (this->isConfigured()) {
    NETWIZARD_DEBUG_MSG("Connecting to configured connection\n");
    // Set hostname
    if (_nw.hostname != "") {
      WiFi.setHostname(_nw.hostname.c_str());
    }
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    _connect(_nw.sta.ssid.c_str(), _nw.sta.password.c_str(), true);
  } else {
    NETWIZARD_DEBUG_MSG("Switching off wifi as the device is not configured\n");
    WiFi.mode(WIFI_OFF);
  }

  _nw.portal.active = false;
  _nw.portal.state = NetWizardPortalState::IDLE;
  _nw.portal.start_millis = 0;
}

#if NETWIZARD_USE_ASYNC_WEBSERVER == 1
  bool NetWizard::_onAPFilter(AsyncWebServerRequest *request) {
    #if defined(ESP32)
      // Serial.printf("AP IP: %s, Client IP: %s\n", WiFi.AP.localIP().toString().c_str(), request->client()->localIP().toString().c_str());
      return WiFi.AP.hasIP() && WiFi.AP.localIP() == request->client()->localIP();
    #elif defined(ESP8266)
      return WiFi.softAPIP() == request->client()->localIP();
    #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
      return WiFi.softAPIP() == request->client()->localIP();
    #endif
  }
#else
  #if defined(ESP8266) || defined(ESP32)
    bool NetWizard::_onAPFilter(NETWIZARD_WEBSERVER &server) {
  #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
    bool NetWizard::_onAPFilter(HTTPServer &server) {
  #endif
    #if defined(ESP32)
      // Serial.printf("AP IP: %s, Client IP: %s\n", WiFi.AP.localIP().toString().c_str(), server.client().localIP().toString().c_str());
      return WiFi.AP.hasIP() && WiFi.AP.localIP() == server.client().localIP();
    #elif defined(ESP8266)
      // Serial.printf("AP IP: %s, Client IP: %s\n", WiFi.softAPIP().toString().c_str(), server.client().localIP().toString().c_str());
      return WiFi.softAPIP() == server.client().localIP();
    #elif defined(TARGET_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2040) || defined(PICO_RP2350)
      // Serial.printf("AP IP: %s, Client IP: %s\n", WiFi.softAPIP().toString().c_str(), server.client().localIP().toString().c_str());
      return WiFi.softAPIP() == server.client().localIP();
    #endif
  }
#endif

NetWizard::~NetWizard() {
  // Clear all parameters
  for(int i=0; i < _nw.portal.config.parameters.Size(); i++){
    NetWizardParameter *p = _nw.portal.config.parameters[i];
    delete p;
    _nw.portal.config.parameters.Erase(i);
  }
}
