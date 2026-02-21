#ifndef __ARCANA_CACHE_H__
#define __ARCANA_CACHE_H__

/**
 * @defgroup Cache Cache Management
 * @brief Input and script caching facilities.
 *
 * This module provides caching services used by Arcana to:
 * - track input file changes
 * - manage profile-dependent cache invalidation
 * - persist generated scripts
 *
 * The cache is global, singleton-based, and intentionally exception-free.
 */

/**
 * @addtogroup Cache
 * @{
 */

#include "Defines.h"
#include "Semantic.h"

#include <fstream>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <map>
#include <filesystem>



BEGIN_MODULE(Cache)
USE_MODULE(Arcana);




/** @brief Filesystem namespace alias used by the cache module. */
namespace fs = std::filesystem;




/**
 * @brief Computes the MD5 hash of a memory buffer.
 *
 * Used internally to detect file content changes.
 *
 * @param[in] data Input data.
 * @return MD5 hash string.
 */
std::string MD5(const std::string& data) noexcept;

std::string MD5_bin(const std::string& data) noexcept;




//     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
// 

class BinFile {
public:
    static constexpr std::size_t DefaultBlock = 16;

    BinFile() = default;
    ~BinFile()
    {
        close();
    }

    // Apre un file; se uno era già aperto lo chiude
    bool open(const std::string& path)
    {
        close(); // chiudi quello precedente se esiste

        file_.open(path,
                   std::ios::binary |
                   std::ios::in    |
                   std::ios::out);

        // Se non esiste, proviamo a crearlo:
        if (!file_) {
            file_.clear();
            file_.open(path,
                       std::ios::binary |
                       std::ios::in    |
                       std::ios::out   |
                       std::ios::trunc);
        }

        return static_cast<bool>(file_);
    }

    void close()
    {
        if (file_.is_open())
            file_.close();
    }

    bool is_open() const
    {
        return file_.is_open();
    }

    // Legge 'size' byte a partire da offset (default 32)
    // buffer deve avere almeno 'size' elementi
    bool read_at(std::uint64_t offset,
                std::string& out,
                std::size_t size = DefaultBlock)
    {
        if (!is_open()) return false;

        file_.seekg(0, std::ios::beg);

        out.resize(size);

        file_.clear();
        file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file_) return false;

        file_.read(out.data(), static_cast<std::streamsize>(size));

        // Se EOF, può aver letto meno byte. Se vuoi mantenerli:
        out.resize(static_cast<std::size_t>(file_.gcount()));

        return static_cast<bool>(file_);
    }

    // Scrive 'size' byte a partire da offset (default 32)
    bool write_at(std::uint64_t offset,
                const std::string& data,
                std::size_t size = DefaultBlock)
    {
        if (!is_open()) return false;

        file_.seekg(0, std::ios::beg);

        if (data.size() < size) return false;   // meglio errore esplicito

        file_.clear();
        file_.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file_) return false;

        file_.write(data.data(), static_cast<std::streamsize>(size));
        return static_cast<bool>(file_);
    }

private:
    std::fstream file_;
};



/**
 * @brief Global cache manager.
 *
 * The cache manager tracks input file hashes and generated scripts
 * to avoid unnecessary rebuilds.
 *
 * This class is a singleton and cannot be copied or moved.
 */
class Manager
{
public:
    Manager(const Manager&)              = delete;
    Manager& operator = (const Manager&) = delete;

    Manager(Manager&&)                    noexcept = delete;
    Manager& operator = (const Manager&&) noexcept = delete;

    /**
     * @brief Returns the global cache manager instance.
     */
    static Manager& Instance()
    {
        static Manager m;
        return m;
    }


    /**
     * @brief Removes all cached data from disk.
     */
    void EraseCache() noexcept;


    /**
     * @brief Clears the in-memory cache state.
     */
    void ClearCache() noexcept;


    /**
     * @brief Loads cached data from disk.
     *
     * @param[in] profile Active build profile name.
     */
    void LoadCache(const std::string& profile) noexcept;


    /**
     * @brief Checks whether a file has changed since the last cache update.
     *
     * @param[in] path Path to the file to check.
     * @return true if the file content differs from the cached version.
     */
    bool HasFileChanged(const std::string& path) noexcept;


    /**
     * @brief Writes a generated script to the cache.
     *
     * @param[in] jobname Job name associated with the script.
     * @param[in] idx Script index.
     * @param[in] content Script content.
     * @param[in] ext Optional file extension.
     *
     * @return Path to the generated script file.
     */
    fs::path WriteScript(const std::string& jobname,
                         const std::size_t idx,
                         const std::string& content,
                         const std::string& ext = "") noexcept;

private:
    /** @brief Private constructor for singleton enforcement. */
    Manager();

    ~Manager() = default;


    class Pair
    {
    public:
        std::string key;
        std::string value;

        bool operator == (const Pair& other) 
        {
            return key.compare(other.key)     == 0 &&
                   value.compare(other.value) == 0;
        } 

        static const Pair Create(const char* source, const size_t len = 32)
        {
            static const char hexmap[] = "0123456789ABCDEF";

            Pair p;
            p.key.reserve(len);
            p.value.reserve(len);

            std::size_t i = 0;

            for (i = 0; i < len / 2; ++i) 
            {
                unsigned char b = source[i];

                p.key.push_back(hexmap[b >> 4]);
                p.key.push_back(hexmap[b & 0xF]);
            }

            for (; i < len; ++i) 
            {
                unsigned char b = source[i];

                p.value.push_back(hexmap[b >> 4]);
                p.value.push_back(hexmap[b & 0xF]);
            }

            return p;
        }
    };

    class PairMap : public std::map<std::string, std::string>
    {
    public:
        // Inserisci una coppia Pair
        bool insert(const std::string& k, const std::string& v)
        {
            auto [it, inserted] = emplace(k, v);
            if (!inserted)
            {
                // se vuoi sovrascrivere invece di fallire:
                // it->second = p.value;
                // return true;
            }
            return inserted;
        }
    }



    fs::path _cache_folder;                             ///< Cache root directory.
    fs::path _script_path;                              ///< Script output directory.
    fs::path _binary;                                   ///< Cached items file.

    BinFile             _mnt_binary;
    std::string         _cached_profile;                        ///< Cached profile identifier.
    std::vector<Pair>   _cached_files;
};



END_MODULE(Cache)



#endif /* __ARCANA_CACHE_H__ */