#include "Filesystem.h"

#include <Log.h>

#include "Boards.h"

#if MCU_VARIANT == MCU_ESP32
#include <FS.h>
#include <SPIFFS.h>
#elif MCU_VARIANT == MCU_NRF52
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;
#endif

void listDir(char * dir){
  Serial.print("DIR: ");
  Serial.println(dir);
#if MCU_VARIANT == MCU_ESP32
  File root = SPIFFS.open(dir);
  File file = root.openNextFile();
  while(file){
      Serial.print("  FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
  }
#elif MCU_VARIANT == MCU_NRF52
  File root = InternalFS.open(dir);
  File file = root.openNextFile();
  while(file){
      Serial.print("  FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
  }
#endif
}

bool Filesystem::init() {
	RNS::extreme("Filesystem initializing...");

#if MCU_VARIANT == MCU_ESP32
	// Setup filesystem
	RNS::info("SPIFFS mounting filesystem");
	if (!SPIFFS.begin(true, "")){
		RNS::error("SPIFFS filesystem mount failed");
		return;
	}
	uint32_t size = SPIFFS.totalBytes() / (1024 * 1024);
	Serial.print("size: ");
	Serial.print(size);
	Serial.println(" MB");
	uint32_t used = SPIFFS.usedBytes() / (1024 * 1024);
	Serial.print("used: ");
	Serial.print(used);
	Serial.println(" MB");
	// ensure filesystem is writable and format if not
	RNS::Bytes test;
	if (!OS::write_file(test, "/test")) {
		RNS::info("SPIFFS filesystem is being formatted, please wait...");
		SPIFFS.format();
	}
	else {
		OS::remove_file("/test");
	}
	debug("SPIFFS filesystem is ready");
#elif MCU_VARIANT == MCU_NRF52
	// Initialize Internal File System
	RNS::info("InternalFS mounting filesystem");
	InternalFS.begin();
	RNS::info("InternalFS filesystem is ready");
#endif
		listDir("/");

}

bool Filesystem::file_exists(const char* file_path) {
#if MCU_VARIANT == MCU_ESP32
	File file = SPIFFS.open(file_path, FILE_READ);
	if (file) {
#elif MCU_VARIANT == MCU_NRF52
	File file(InternalFS);
	file.open(file_path, FILE_O_READ);
	if (file) {
#else
	if (false) {
#endif
		//RNS::extreme("file_exists: file exists, closing file");
#if MCU_VARIANT == MCU_ESP32
		file.close();
#elif MCU_VARIANT == MCU_NRF52
		file.close();
#endif
		return true;
	}
	else {
		RNS::error("file_exists: failed to open file " + std::string(file_path));
		return false;
	}
}

const RNS::Bytes Filesystem::read_file(const char* file_path) {
    RNS::Bytes data;
#if MCU_VARIANT == MCU_ESP32
	File file = SPIFFS.open(file_path, FILE_READ);
	if (file) {
		size_t size = file.size();
		size_t read = file.readBytes((char*)data.writable(size), size);
#elif MCU_VARIANT == MCU_NRF52
	File file(InternalFS);
	file.open(file_path, FILE_O_READ);
	if (file) {
		size_t size = file.size();
		size_t read = file.readBytes((char*)data.writable(size), size);
#else
	if (false) {
		size_t size = 0;
		size_t read = 0;
#endif
		RNS::extreme("read_file: read " + std::to_string(read) + " bytes from file " + std::string(file_path));
		if (read != size) {
			RNS::error("read_file: failed to read file " + std::string(file_path));
            data.clear();
		}
		//RNS::extreme("read_file: closing input file");
#if MCU_VARIANT == MCU_ESP32
		file.close();
#elif MCU_VARIANT == MCU_NRF52
		file.close();
#endif
	}
	else {
		RNS::error("read_file: failed to open input file " + std::string(file_path));
	}
    return data;
}

bool Filesystem::write_file(const RNS::Bytes& data, const char* file_path) {
    bool success = false;
#if MCU_VARIANT == MCU_ESP32
	File file = SPIFFS.open(file_path, FILE_WRITE);
	if (file) {
		size_t wrote = file.write(data.data(), data.size());
#elif MCU_VARIANT == MCU_NRF52
	File file(InternalFS);
	file.open(file_path, FILE_O_WRITE);
	if (file) {
		size_t wrote = file.write(data.data(), data.size());
#else
	if (false) {
		size_t wrote = 0;
#endif
        RNS::extreme("write_file: wrote " + std::to_string(wrote) + " bytes to file " + std::string(file_path));
        if (wrote == data.size()) {
            success = true;
        }
        else {
			RNS::error("write_file: failed to write file " + std::string(file_path));
		}
		//RNS::extreme("write_file: closing output file");
#if MCU_VARIANT == MCU_ESP32
		file.close();
#elif MCU_VARIANT == MCU_NRF52
		file.close();
#endif
	}
	else {
		RNS::error("write_file: failed to open output file " + std::string(file_path));
	}
    return success;
}

bool Filesystem::remove_file(const char* file_path) {
#if MCU_VARIANT == MCU_ESP32
	return SPIFFS.remove(file_path);
#elif MCU_VARIANT == MCU_NRF52
	return InternalFS.remove(file_path);
#else
	return false;
#endif
}

bool Filesystem::rename_file(const char* from_file_path, const char* to_file_path) {
#if MCU_VARIANT == MCU_ESP32
	return SPIFFS.rename(from_file_path, to_file_path);
#elif MCU_VARIANT == MCU_NRF52
	return InternalFS.rename(from_file_path, to_file_path);
#else
	return false;
#endif
}

bool Filesystem::create_directory(const char* directory_path) {
#if MCU_VARIANT == MCU_ESP32
	if (!SPIFFS.mkdir(directory_path)) {
		RNS::error("create_directory: failed to create directorty " + std::string(directory_path));
		return false;
	}
	return true;
#elif MCU_VARIANT == MCU_NRF52
	if (!InternalFS.mkdir(directory_path)) {
		RNS::error("create_directory: failed to create directorty " + std::string(directory_path));
		return false;
	}
	return true;
#else
	return false;
#endif
}
