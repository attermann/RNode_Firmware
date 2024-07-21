#include "Filesystem.h"

#ifdef HAS_RNS

#include <Log.h>

#include "Boards.h"

#define FS_TYPE_SPIFFS 0
#define FS_TYPE_LITTLEFS 1
#define FS_TYPE_INTERNALFS 2

#if MCU_VARIANT == MCU_ESP32
	//#define FS_TYPE FS_TYPE_SPIFFS
	#define FS_TYPE FS_TYPE_LITTLEFS
#elif MCU_VARIANT == MCU_NRF52
	#define FS_TYPE FS_TYPE_INTERNALFS
#else
	#define FS_TYPE FS_TYPE_SPIFFS
#endif

#if FS_TYPE == FS_TYPE_SPIFFS
#include <SPIFFS.h>
#define FS SPIFFS
#elif FS_TYPE == FS_TYPE_LITTLEFS
#include <LittleFS.h>
#define FS LittleFS
#elif FS_TYPE == FS_TYPE_INTERNALFS
#include <InternalFileSystem.h>
#define FS InternalFS
using namespace Adafruit_LittleFS_Namespace;
#else
#error "Filesystem type not specified"
#endif

#if FS_TYPE == FS_TYPE_INTERNALFS

inline int _countLfsBlock(void *p, lfs_block_t block) {
	lfs_size_t *size = (lfs_size_t*) p;
	*size += 1;
	return 0;
}

lfs_ssize_t getUsedBlockCount() {
    lfs_size_t size = 0;
    lfs_traverse(FS._getFS(), _countLfsBlock, &size);
    return size;
}

size_t totalBytes() {
	const lfs_config* config = FS._getFS()->cfg;
	return config->block_size * config->block_count;
}

size_t usedBytes() {
	const lfs_config* config = FS._getFS()->cfg;
	const size_t usedBlockCount = getUsedBlockCount();
	return config->block_size * usedBlockCount;
}

#endif

#ifndef NDEBUG

void Filesystem::listDir(const char* dir, const char* prefix /*= ""*/) {
	Serial.print(prefix);
	std::string full_dir(dir);
	if (full_dir.compare("/") != 0) {
		full_dir += "/";
	}
	Serial.println(full_dir.c_str());
	std::string pre(prefix);
	pre.append("  ");
	try {
		File root = FS.open(dir);
		if (!root) {
			Serial.print(pre.c_str());
			Serial.println("(failed to open directory)");
			return;
		}
		File file = root.openNextFile();
		while (file) {
			char* name = (char*)file.name();
			std::string recurse_dir(full_dir);
			if (file.isDirectory()) {
				recurse_dir += name;
				listDir(recurse_dir.c_str(), pre.c_str());
			}
			else {
				Serial.print(pre.c_str());
				//Serial.print("FILE: ");
				Serial.print(name);
				Serial.print(" (");
				Serial.print(file.size());
				Serial.println(" bytes)");
			}
			file.close();
			file = root.openNextFile();
		}
		root.close();
	}
	catch (std::exception& e) {
		Serial.print("listDir Exception: ");
		Serial.println(e.what());
	}
}

void Filesystem::dumpDir(const char* dir) {
	Serial.print("DIR: ");
	std::string full_dir(dir);
	if (full_dir.compare("/") != 0) {
		full_dir += "/";
	}
	Serial.println(full_dir.c_str());
	try {
		File root = FS.open(dir);
		if (!root) {
			Serial.println("(failed to open directory)");
			return;
		}
		File file = root.openNextFile();
		while (file) {
			char* name = (char*)file.name();
			if (file.isDirectory()) {
				std::string recurse_dir(full_dir);
				recurse_dir += name;
				dumpDir(recurse_dir.c_str());
			}
			else {
				Serial.print("\nFILE: ");
				Serial.print(name);
				Serial.print(" (");
				Serial.print(file.size());
				Serial.println(" bytes)");
				char data[4096];
				size_t size = file.size();
				size_t read = file.readBytes(data, (size < sizeof(data)) ? size : sizeof(data));
				Serial.write(data, read);
				Serial.println("");
			}
			file.close();
			file = root.openNextFile();
		}
		root.close();
	}
	catch (std::exception& e) {
		Serial.print("dumpDir Exception: ");
		Serial.println(e.what());
	}
}

bool Filesystem::format() {
	INFO("Formatting filesystem...");
	try {
		if (!FS.format()) {
			ERROR("Format failed!");
			return false;
		}
		return true;
	}
	catch (std::exception& e) {
		ERROR("Filesystem reformat Exception: " + std::string(e.what()));
	}
	return false;
}

bool Filesystem::reformat() {
	INFO("Reformatting filesystem...");
	try {
		RNS::Bytes eeprom;
		read_file("/eeprom", eeprom);
		RNS::Bytes transport_identity;
		read_file("/transport_identity", transport_identity);
		//RNS::Bytes time_offset;
		//read_file("/time_offset", time_offset);
		if (!FS.format()) {
			ERROR("Format failed!");
			return false;
		}
		if (eeprom) {
			write_file("/eeprom", eeprom);
		}
		if (transport_identity) {
			write_file("/transport_identity", transport_identity);
		}
		//if (time_offset) {
		//	write_file("/time_offset", time_offset);
		//}
		return true;
	}
	catch (std::exception& e) {
		ERROR("Filesystem reformat Exception: " + std::string(e.what()));
	}
	return false;
}

#endif

bool Filesystem::init() {
	TRACE("Initializing filesystem...");
	try {
#if FS_TYPE == FS_TYPE_SPIFFS
		// Initialize SPIFFS
		INFO("SPIFFS mounting filesystem");
		if (!SPIFFS.begin(true, "")) {
			ERROR("SPIFFS filesystem mount failed");
			return false;
		}
		INFO("SPIFFS filesystem is ready");
#elif FS_TYPE == FS_TYPE_LITTLEFS
		// Initialize LittleFS
		INFO("LittleFS mounting filesystem");
		if (!LittleFS.begin(true, "")) {
			ERROR("LittleFS filesystem mount failed");
			return false;
		}
		DEBUG("LittleFS filesystem is ready");
#elif FS_TYPE == FS_TYPE_INTERNALFS
		// Initialize InternalFileSystem
		INFO("InternalFS mounting filesystem");
		if (!InternalFS.begin()) {
			ERROR("InternalFS filesystem mount failed");
			return false;
		}
		INFO("InternalFS filesystem is ready");
#endif
		// Ensure filesystem is writable and reformat if not
		RNS::Bytes test("test");
		if (write_file("/test", test) < 4) {
			HEAD("Failed to write test file, filesystem is being reformatted...", RNS::LOG_CRITICAL);
			//FS.format();
			reformat();
		}
		else {
			remove_file("/test");
		}
	}
	catch (std::exception& e) {
		//ERROR("Filesystem init Exception: " + std::string(e.what()));
		return false;
	}
	TRACE("Finished initializing");
	return true;
}

/*virtua*/ bool Filesystem::file_exists(const char* file_path) {
	TRACEF("file_exists: checking for existence of file %s", file_path);
/*
#if FS_TYPE == FS_TYPE_INTERNALFS
	File file(InternalFS);
	if (file.open(file_path, FILE_O_READ)) {
#else
	File file = FS.open(file_path, FILE_READ);
	if (file) {
#endif
		bool is_directory = file.isDirectory();
		file.close();
		return !is_directory;
	}
	return false;
*/
	return FS.exists(file_path);
}

/*virtua*/ size_t Filesystem::read_file(const char* file_path, RNS::Bytes& data) {
	TRACEF("read_file: reading from file %s", file_path);
	size_t read = 0;
#if FS_TYPE == FS_TYPE_INTERNALFS
	File file(InternalFS);
	if (file.open(file_path, FILE_O_READ)) {
#else
	File file = FS.open(file_path, FILE_READ);
	if (file) {
#endif
		size_t size = file.size();
		read = file.readBytes((char*)data.writable(size), size);
		TRACEF("read_file: read %u bytes from file %s", read, file_path);
		if (read != size) {
			ERRORF("read_file: failed to read file %s", file_path);
            data.resize(read);
		}
		//TRACE("read_file: closing input file");
		file.close();
	}
	else {
		ERRORF("read_file: failed to open input file %s", file_path);
	}
    return read;
}

/*virtua*/ size_t Filesystem::write_file(const char* file_path, const RNS::Bytes& data) {
	TRACEF("write_file: writing to file %s", file_path);
	// CBA TODO Replace remove with working truncation
	if (FS.exists(file_path)) {
		FS.remove(file_path);
	}
	size_t wrote = 0;
#if FS_TYPE == FS_TYPE_INTERNALFS
	File file(InternalFS);
	if (file.open(file_path, FILE_O_WRITE)) {
#else
	File file = FS.open(file_path, FILE_WRITE);
	if (file) {
#endif
		// Seek to beginning to overwrite
		//file.seek(0);
		//file.truncate(0);
		wrote = file.write(data.data(), data.size());
        TRACEF("write_file: wrote %u bytes to file %s", wrote, file_path);
        if (wrote < data.size()) {
			WARNINGF("write_file: not all data was written to file %s", file_path);
		}
		//TRACE("write_file: closing output file");
		file.close();
	}
	else {
		ERRORF("write_file: failed to open output file %s", file_path);
	}
    return wrote;
}

/*virtua*/ bool Filesystem::remove_file(const char* file_path) {
	TRACEF("remove_file: removing file %s", file_path);
	return FS.remove(file_path);
}

/*virtua*/ bool Filesystem::rename_file(const char* from_file_path, const char* to_file_path) {
	TRACEF("rename_file: renaming file %s to %s", from_file_path, to_file_path);
	return FS.rename(from_file_path, to_file_path);
}

/*virtua*/ bool Filesystem::directory_exists(const char* directory_path) {
	TRACEF("directory_exists: checking for existence of directory %s", directory_path);
#if FS_TYPE == FS_TYPE_INTERNALFS
	File file(InternalFS);
	if (file.open(directory_path, FILE_O_READ)) {
#else
	File file = FS.open(directory_path, FILE_READ);
	if (file) {
#endif
		bool is_directory = file.isDirectory();
		file.close();
		return is_directory;
	}
	return false;
}

/*virtua*/ bool Filesystem::create_directory(const char* directory_path) {
	TRACEF("create_directory: creating directory %s", directory_path);
	if (!FS.mkdir(directory_path)) {
		ERROR("create_directory: failed to create directory " + std::string(directory_path));
		return false;
	}
	return true;
}

/*virtua*/ bool Filesystem::remove_directory(const char* directory_path) {
	TRACEF("remove_directory: removing directory %s", directory_path);
#if FS_TYPE == FS_TYPE_INTERNALFS
	if (!FS.rmdir_r(directory_path)) {
#else
	if (!FS.rmdir(directory_path)) {
#endif
		ERROR("remove_directory: failed to remove directory " + std::string(directory_path));
		return false;
	}
	return true;
}

/*virtua*/ std::list<std::string> Filesystem::list_directory(const char* directory_path) {
	TRACEF("list_directory: listing directory %s", directory_path);
	std::list<std::string> files;
	File root = FS.open(directory_path);
	if (!root) {
		ERROR("list_directory: failed to open directory " + std::string(directory_path));
		return files;
	}
	File file = root.openNextFile();
	while (file) {
		if (!file.isDirectory()) {
			char* name = (char*)file.name();
			files.push_back(name);
		}
		// CBA Following close required to avoid leaking memory
		file.close();
		file = root.openNextFile();
	}
	root.close();
	TRACE("list_directory: returning directory listing");
	return files;
}

/*virtual*/ size_t Filesystem::storage_size() {
#if FS_TYPE == FS_TYPE_INTERNALFS
	return totalBytes();
#else
	return FS.totalBytes();
#endif
}

/*virtual*/ size_t Filesystem::storage_available() {
#if FS_TYPE == FS_TYPE_INTERNALFS
	return (totalBytes() - usedBytes());
#else
	return (FS.totalBytes() - FS.usedBytes());
#endif
}

#endif
