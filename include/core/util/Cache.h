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






//     ██████╗██╗      █████╗ ███████╗███████╗███████╗███████╗
//    ██╔════╝██║     ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝
//    ██║     ██║     ███████║███████╗███████╗█████╗  ███████╗
//    ██║     ██║     ██╔══██║╚════██║╚════██║██╔══╝  ╚════██║
//    ╚██████╗███████╗██║  ██║███████║███████║███████╗███████║
//     ╚═════╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝╚══════╝
// 



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
     */
    void LoadCache() noexcept;


    /**
     * @brief Checks whether a file has changed since the last cache update.
     *
     * @param[in] path Path to the file to check.
     * @return true if the file content differs from the cached version.
     */
    bool HasFileChanged(const std::string& path) noexcept;


    /**
     * @brief Handles cache invalidation due to profile changes.
     *
     * @param[in] profile Active build profile name.
     */
    void HandleProfileChange(const std::string& profile) noexcept;


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

    fs::path _cache_folder;                             ///< Cache root directory.
    fs::path _script_path;                              ///< Script output directory.
    fs::path _input_path;                               ///< Cached input directory.
    fs::path _profile;                                  ///< Active profile path.
    std::map<std::string, std::string> _cached_inputs;  ///< Cached input hashes.
    std::string _cached_profile;                        ///< Cached profile identifier.
};



END_MODULE(Cache)



#endif /* __ARCANA_CACHE_H__ */