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
#include <cstring>
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
class BinFile
{
public:
    BinFile() = default;

    ~BinFile()
    {
        close();
    }

    bool open(const std::string& path)
    {
        close();

        file_.open(path,
                   std::ios::binary |
                   std::ios::in |
                   std::ios::out);

        if (!file_)
        {
            file_.clear();
            file_.open(path,
                       std::ios::binary |
                       std::ios::in |
                       std::ios::out |
                       std::ios::trunc);
        }

        return static_cast<bool>(file_);
    }

    void close()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }

    bool is_open() const
    {
        return file_.is_open();
    }

    std::uint64_t file_size() noexcept
    {
        if (!is_open())
        {
            return 0;
        }

        file_.clear();

        file_.seekg(0, std::ios::end);
        if (!file_)
        {
            file_.clear();
            return 0;
        }

        const std::streampos endpos = file_.tellg();
        if (endpos < 0)
        {
            file_.clear();
            return 0;
        }

        return static_cast<std::uint64_t>(endpos);
    }

    bool read_exact(std::uint64_t offset, void* dst, std::size_t size) noexcept
    {
        if (!is_open())
        {
            return false;
        }

        file_.clear();
        file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file_)
        {
            file_.clear();
            return false;
        }

        file_.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(size));

        if (file_.gcount() != static_cast<std::streamsize>(size))
        {
            file_.clear();
            return false;
        }

        return true;
    }

    bool write_exact(std::uint64_t offset, const void* src, std::size_t size) noexcept
    {
        if (!is_open())
        {
            return false;
        }

        file_.clear();
        file_.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file_)
        {
            file_.clear();
            return false;
        }

        file_.write(reinterpret_cast<const char*>(src), static_cast<std::streamsize>(size));
        file_.flush();

        return static_cast<bool>(file_);
    }

    bool read_block16(std::uint64_t offset, std::string& out) noexcept
    {
        out.assign(16, '\0');
        return read_exact(offset, out.data(), 16);
    }

    bool write_block16(std::uint64_t offset, const std::string& data) noexcept
    {
        if (data.size() != 16)
        {
            return false;
        }

        return write_exact(offset, data.data(), 16);
    }

    bool erase_record(std::uint64_t offset, std::size_t record_size) noexcept
    {
        std::vector<char> zeros(record_size, 0);
        return write_exact(offset, zeros.data(), record_size);
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
    void ClearCache(const std::vector<std::string>& keys = {}) noexcept;


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

    static constexpr std::size_t MD5_RAW_SIZE   = 16;
    static constexpr std::size_t FILE_REC_SIZE  = 32;

    struct FileRecord
    {
        char path_md5[MD5_RAW_SIZE];
        char content_md5[MD5_RAW_SIZE];
    };

    static bool IsMD5Raw16(const std::string& s) noexcept
    {
        return s.size() == MD5_RAW_SIZE;
    }

    static void RecordFrom(const std::string& k16, const std::string& v16, FileRecord& out) noexcept
    {
        std::memcpy(out.path_md5,    k16.data(), MD5_RAW_SIZE);
        std::memcpy(out.content_md5, v16.data(), MD5_RAW_SIZE);
    }

    static std::string KeyToString(const FileRecord& r) noexcept
    {
        return std::string(r.path_md5, MD5_RAW_SIZE);
    }

    static std::string ValToString(const FileRecord& r) noexcept
    {
        return std::string(r.content_md5, MD5_RAW_SIZE);
    }

    class PairMap : public std::map<std::string, std::pair<std::uint64_t, std::string>>
    {
    public:
        bool upsert(const std::string& k, const std::string& v, const std::uint64_t offset)
        {
            auto value = std::make_pair(offset, v);

            auto it = this->find(k);
            if (it == this->end())
            {
                this->emplace(k, value);
                return true;
            }

            it->second = value;
            return true;
        }
    };

    fs::path _cache_folder;                             ///< Cache root directory.
    fs::path _script_path;                              ///< Script output directory.
    fs::path _binary;                                   ///< Cached items file.

    BinFile             _mnt_binary;
    std::string         _cached_profile;                        ///< Cached profile identifier.
    PairMap             _cached_files;
};



END_MODULE(Cache)



#endif /* __ARCANA_CACHE_H__ */