#include "Filesystem.h"

#include <Log.h>

#include "Boards.h"

#if MCU_VARIANT == MCU_ESP32
#include <FS.h>
#include <LittleFS.h>
#elif MCU_VARIANT == MCU_NRF52
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;
#endif

void listDir(const char* dir, const char* prefix) {
	RNS::extreme("Listing filesystem...");
	Serial.print(prefix);
	//Serial.print("DIR: ");
	Serial.print(dir);
	Serial.println("/");
	std::string pre(prefix);
	pre.append("  ");
	try {
#if MCU_VARIANT == MCU_ESP32
		File root = LittleFS.open(dir);
#elif MCU_VARIANT == MCU_NRF52
		File root = InternalFS.open(dir);
#endif
		File file = root.openNextFile();
		while (file) {
			if (file.isDirectory()) {
				listDir(file.name(), pre.c_str());
			}
			else {
				Serial.print(pre.c_str());
				//Serial.print("FILE: ");
				Serial.println(file.name());
			}
			file = root.openNextFile();
		}
	}
	catch (std::exception& e) {
		RNS::error("listDir Exception: " + std::string(e.what()));
	}
	RNS::extreme("Finished listing");
}

void dumpDir(const char* dir) {
	RNS::extreme("Dumping filesystem...");
	Serial.print("DIR: ");
	Serial.print(dir);
	Serial.println("/");
	try {
#if MCU_VARIANT == MCU_ESP32
		File root = LittleFS.open(dir);
#elif MCU_VARIANT == MCU_NRF52
		File root = InternalFS.open(dir);
#endif
		File file = root.openNextFile();
		while (file) {
			if (file.isDirectory()) {
				dumpDir(file.name());
			}
			else {
				Serial.print("\nFILE: ");
				Serial.println(file.name());
				char data[4096];
				size_t size = file.size();
				size_t read = file.readBytes(data, (size < sizeof(data)) ? size : sizeof(data));
				Serial.write(data, read);
				Serial.println("");
			}
			file = root.openNextFile();
		}
	}
	catch (std::exception& e) {
		RNS::error("dumpDir Exception: " + std::string(e.what()));
	}
	RNS::extreme("Finished dumping");
}

/*virtua*/ bool Filesystem::init() {
	RNS::extreme("Initializing filesystem...");

	try {
#if MCU_VARIANT == MCU_ESP32
		// Setup filesystem
		RNS::info("LittleFS mounting filesystem");
		if (!LittleFS.begin(true, "")) {
			RNS::error("LittleFS filesystem mount failed");
			return false;
		}
/*
		uint32_t size = LittleFS.totalBytes() / 1024;
		Serial.print("size: ");
		Serial.print(size);
		Serial.println(" kB");
		uint32_t used = LittleFS.usedBytes() / 1024;
		Serial.print("used: ");
		Serial.print(used);
		Serial.println(" kB");
		// ensure filesystem is writable and format if not
		RNS::Bytes test;
		if (!write_file(test, "/test")) {
			RNS::info("LittleFS filesystem is being formatted, please wait...");
			LittleFS.format();
		}
		else {
			remove_file("/test");
		}
*/
		RNS::debug("LittleFS filesystem is ready");
#elif MCU_VARIANT == MCU_NRF52
		// Initialize Internal File System
		RNS::info("InternalFS mounting filesystem");
		InternalFS.begin();
		RNS::info("InternalFS filesystem is ready");
#endif
	}
	catch (std::exception& e) {
		//RNS::error("Filesystem init Exception: " + std::string(e.what()));
	}

	// CBA DEBUG
	//listDir("/", "");
	//dumpDir("/");

	RNS::extreme("Finished initializing");
}

/*virtua*/ bool Filesystem::file_exists(const char* file_path) {
	RNS::extreme("file_exists: checking for existence of file " + std::string(file_path));
#if MCU_VARIANT == MCU_ESP32
	File file = LittleFS.open(file_path, FILE_READ);
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

/*virtua*/ const RNS::Bytes Filesystem::read_file(const char* file_path) {
	RNS::extreme("read_file: reading from file " + std::string(file_path));
    RNS::Bytes data;
#if MCU_VARIANT == MCU_ESP32
	File file = LittleFS.open(file_path, FILE_READ);
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

/*virtua*/ bool Filesystem::write_file(const RNS::Bytes& data, const char* file_path) {
	RNS::extreme("write_file: writing to file " + std::string(file_path));
    bool success = false;
#if MCU_VARIANT == MCU_ESP32
	File file = LittleFS.open(file_path, FILE_WRITE);
	if (file) {
		size_t wrote = file.write(data.data(), data.size());
#elif MCU_VARIANT == MCU_NRF52
	File file(InternalFS);
	file.open(file_path, FILE_O_WRITE);
	// Seek to beginning to overwrite
	file.seek(0);
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

/*virtua*/ bool Filesystem::remove_file(const char* file_path) {
	RNS::extreme("remove_file: removing file " + std::string(file_path));
#if MCU_VARIANT == MCU_ESP32
	return LittleFS.remove(file_path);
#elif MCU_VARIANT == MCU_NRF52
	return InternalFS.remove(file_path);
#else
	return false;
#endif
}

/*virtua*/ bool Filesystem::rename_file(const char* from_file_path, const char* to_file_path) {
	RNS::extreme("rename_file: renaming file " + std::string(from_file_path) + " to " + std::string(to_file_path));
#if MCU_VARIANT == MCU_ESP32
	return LittleFS.rename(from_file_path, to_file_path);
#elif MCU_VARIANT == MCU_NRF52
	return InternalFS.rename(from_file_path, to_file_path);
#else
	return false;
#endif
}

/*virtua*/ bool Filesystem::directory_exists(const char* directory_path) {
	RNS::extreme("directory_exists: checking for existence of directory " + std::string(directory_path));
#if MCU_VARIANT == MCU_ESP32
	File file = LittleFS.open(directory_path, FILE_READ);
	if (file) {
		bool is_directory = file.isDirectory();
		file.close();
		return is_directory;
	}
#elif MCU_VARIANT == MCU_NRF52
	File file(InternalFS);
	file.open(directory_path, FILE_O_READ);
	if (file) {
		bool is_directory = file.isDirectory();
		file.close();
		return is_directory;
	}
#else
	if (false) {
		return false;
	}
#endif
	else {
		return false;
	}
}

/*virtua*/ bool Filesystem::create_directory(const char* directory_path) {
	RNS::extreme("create_directory: creating directory " + std::string(directory_path));
#if MCU_VARIANT == MCU_ESP32
	if (!LittleFS.mkdir(directory_path)) {
		RNS::error("create_directory: failed to create directorty " + std::string(directory_path));
		return false;
	}
	return true;
#elif MCU_VARIANT == MCU_NRF52
	if (!InternalFS.mkdir(directory_path)) {
		RNS::error("create_directory: failed to create directory " + std::string(directory_path));
		return false;
	}
	return true;
#else
	return false;
#endif
}

/*virtua*/ bool Filesystem::remove_directory(const char* directory_path) {
	RNS::extreme("remove_directory: removing directory " + std::string(directory_path));
#if MCU_VARIANT == MCU_ESP32
	//if (!LittleFS.rmdir_r(directory_path)) {
	if (!LittleFS.rmdir(directory_path)) {
		RNS::error("remove_directory: failed to remove directorty " + std::string(directory_path));
		return false;
	}
	return true;
#elif MCU_VARIANT == MCU_NRF52
	if (!InternalFS.rmdir_r(directory_path)) {
		RNS::error("remove_directory: failed to remove directory " + std::string(directory_path));
		return false;
	}
	return true;
#else
	return false;
#endif
}

/*virtua*/ std::list<std::string> Filesystem::list_directory(const char* directory_path) {
	RNS::extreme("list_directory: listing directory " + std::string(directory_path));
	std::list<std::string> files;
#if MCU_VARIANT == MCU_ESP32
	File root = LittleFS.open(directory_path);
	File file = root.openNextFile();
	while (file) {
		files.push_back(std::string(directory_path) + file.name());
		file = root.openNextFile();
	}
#elif MCU_VARIANT == MCU_NRF52
	File root = InternalFS.open(directory_path);
	File file = root.openNextFile();
	while (file) {
		files.push_back(std::string(directory_path) + file.name());
		file = root.openNextFile();
	}
#endif
}
