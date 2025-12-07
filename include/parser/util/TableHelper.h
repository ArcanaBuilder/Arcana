#ifndef __ARCANA_UTIL_TABLE_HELPER__H__
#define __ARCANA_UTIL_TABLE_HELPER__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>
#include <regex>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

#include <fnmatch.h>

#include "Support.h"
#include "Defines.h"





//    ███████╗ ██████╗ ██████╗ ██╗    ██╗ █████╗ ██████╗ ██████╗ ███████╗
//    ██╔════╝██╔═══██╗██╔══██╗██║    ██║██╔══██╗██╔══██╗██╔══██╗██╔════╝
//    █████╗  ██║   ██║██████╔╝██║ █╗ ██║███████║██████╔╝██║  ██║███████╗
//    ██╔══╝  ██║   ██║██╔══██╗██║███╗██║██╔══██║██╔══██╗██║  ██║╚════██║
//    ██║     ╚██████╔╝██║  ██║╚███╔███╔╝██║  ██║██║  ██║██████╔╝███████║
//    ╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝ ╚══════╝
//                                                                                                                                                                                                                                                                                                       


BEGIN_MODULE(Semantic::Attr)

enum class Type;

END_MODULE(Semantic::Attr)











BEGIN_MODULE(Table)




//    ██████╗ ██████╗ ██╗██╗   ██╗ █████╗ ████████╗███████╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
//    ██╔══██╗██╔══██╗██║██║   ██║██╔══██╗╚══██╔══╝██╔════╝    ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
//    ██████╔╝██████╔╝██║██║   ██║███████║   ██║   █████╗      █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
//    ██╔═══╝ ██╔══██╗██║╚██╗ ██╔╝██╔══██║   ██║   ██╔══╝      ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
//    ██║     ██║  ██║██║ ╚████╔╝ ██║  ██║   ██║   ███████╗    ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
//    ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝  ╚═╝  ╚═╝   ╚═╝   ╚══════╝    ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                                                                                                        


template <typename T>
inline static bool HasAttrOnMapped(const T& value, const Arcana::Semantic::Attr::Type attr)
{
    return value.hasAttribute(attr);
}



template <typename T>
inline static bool HasAttrOnMapped(T* ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
}



template <typename T>
inline static bool HasAttrOnMapped(const std::shared_ptr<T>& ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
}



template <typename T>
inline static bool HasAttrOnMapped(const std::unique_ptr<T>& ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
}



static std::optional<std::filesystem::path>
extract_base_dir(const std::string& pattern)
{
    // trova il primo carattere "glob"
    const std::string glob_chars = "*?[";
    std::size_t pos = pattern.find_first_of(glob_chars);

    std::string base = pattern;
    if (pos != std::string::npos)
    {
        // taglia alla directory che precede il primo glob
        pos = pattern.rfind('/', pos);
        if (pos != std::string::npos)
            base = pattern.substr(0, pos);
        else
            base.clear();
    }
    else
    {
        return std::nullopt;
    }

    if (base.empty())
        return std::filesystem::current_path();

    return std::filesystem::path(base);
}









//    ██████╗ ███████╗███████╗██╗  ██╗     ██████╗ ██████╗  ██████╗ ██╗   ██╗██████╗ 
//    ██╔══██╗██╔════╝██╔════╝██║ ██╔╝    ██╔════╝ ██╔══██╗██╔═══██╗██║   ██║██╔══██╗
//    ██████╔╝█████╗  █████╗  █████╔╝     ██║  ███╗██████╔╝██║   ██║██║   ██║██████╔╝
//    ██╔═══╝ ██╔══╝  ██╔══╝  ██╔═██╗     ██║   ██║██╔══██╗██║   ██║██║   ██║██╔═══╝ 
//    ██║     ███████╗███████╗██║  ██╗    ╚██████╔╝██║  ██║╚██████╔╝╚██████╔╝██║     
//    ╚═╝     ╚══════╝╚══════╝╚═╝  ╚═╝     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝     
//                                                                                   



template <typename TABLE>
std::optional<std::reference_wrapper<typename TABLE::mapped_type>>
GetValue(TABLE& table, const Arcana::Semantic::Attr::Type attr)
{
    for (auto& [k, v] : table)
    {
        if (HasAttrOnMapped(v, attr))
        {
            return std::ref(v);
        }
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<std::vector<std::reference_wrapper<typename TABLE::mapped_type>>>
GetValues(TABLE& table, const Arcana::Semantic::Attr::Type attr)
{
    std::vector<std::reference_wrapper<typename TABLE::mapped_type>> vec;

    for (auto& [k, v] : table)
    {
        if (HasAttrOnMapped(v, attr))
        {
            vec.push_back(std::ref(v));
        }
    }

    if (vec.empty())
    {
        return std::nullopt;
    }

    return vec;
}



template <typename TABLE>
std::optional<std::reference_wrapper<typename TABLE::mapped_type>>
GetValue(TABLE& table, const typename TABLE::key_type& key, const std::vector<std::string>& profiles)
{
    auto it = table.find(key);

    if (it == table.end())
    {
        for (const auto& profile : profiles)
        {
            const auto mangled = Support::generate_mangling(key, profile);

            it = table.find(mangled);

            if (it != table.end())
            {
                return std::ref(it->second);
            }
        }
    }
    else
    {
        return std::ref(it->second);
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<std::reference_wrapper<typename TABLE::mapped_type>>
GetValue(TABLE& table, const typename TABLE::key_type& key, const std::vector<std::string>& profiles, const Arcana::Semantic::Attr::Type attr)
{
    auto it = table.find(key);
    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        return std::ref(it->second);
    }

    for (const auto& profile : profiles)
    {
        const auto mangled = Support::generate_mangling(key, profile);

        it = table.find(mangled);
        if (it != table.end() && HasAttrOnMapped(it->second, attr))
        {
            return std::ref(it->second);
        }
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<std::reference_wrapper<typename TABLE::mapped_type>>
GetValue(TABLE& table, const typename TABLE::key_type& key, const std::string& profile)
{
    auto it = table.find(key);

    if (it == table.end())
    {
        const auto mangled = Support::generate_mangling(key, profile);

        it = table.find(mangled);

        if (it != table.end())
        {
            return std::ref(it->second);
        }
    }
    else
    {
        return std::ref(it->second);
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<std::reference_wrapper<typename TABLE::mapped_type>>
GetValue(TABLE& table, const typename TABLE::key_type& key, const std::string& profile, const Arcana::Semantic::Attr::Type attr)
{
    auto it = table.find(key);

    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        return std::ref(it->second);
    }

    const auto mangled = Support::generate_mangling(key, profile);

    it = table.find(mangled);
    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        return std::ref(it->second);
    }

    return std::nullopt;
}



template <typename TABLE>
std::vector<std::reference_wrapper<typename TABLE::mapped_type>>
GetValues(TABLE& table, const std::string& profile, const Arcana::Semantic::Attr::Type attr)
{
    std::vector<std::reference_wrapper<typename TABLE::mapped_type>> vec;

    for (const auto& [k, v] : table)
    {
        auto result = GetValue(table, k, profile, attr);

        if (result)
        {
            vec.push_back(std::ref(result.value()));
        }
    }

    return vec;
}



template <typename TABLE>
std::vector<std::reference_wrapper<typename TABLE::mapped_type>>
GetValues(TABLE& table, const std::vector<std::string>& profiles, const Arcana::Semantic::Attr::Type attr)
{
    std::vector<std::reference_wrapper<typename TABLE::mapped_type>> vec;

    for (const auto& [k, v] : table)
    {
        auto result = GetValue(table, k, profiles, attr);

        if (result)
        {
            vec.push_back(std::ref(result.value()));
        }
    }

    return vec;
}





//    ████████╗ █████╗ ██╗  ██╗███████╗     ██████╗ ██████╗  ██████╗ ██╗   ██╗██████╗ 
//    ╚══██╔══╝██╔══██╗██║ ██╔╝██╔════╝    ██╔════╝ ██╔══██╗██╔═══██╗██║   ██║██╔══██╗
//       ██║   ███████║█████╔╝ █████╗      ██║  ███╗██████╔╝██║   ██║██║   ██║██████╔╝
//       ██║   ██╔══██║██╔═██╗ ██╔══╝      ██║   ██║██╔══██╗██║   ██║██║   ██║██╔═══╝ 
//       ██║   ██║  ██║██║  ██╗███████╗    ╚██████╔╝██║  ██║╚██████╔╝╚██████╔╝██║     
//       ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝     ╚═════╝ ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝     
//                                                                                    



template <typename TABLE>
std::optional<typename TABLE::mapped_type>
TakeValue(TABLE& table, const typename TABLE::key_type& key, const std::string& profile)
{
    auto it = table.find(key);

    if (it != table.end())
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    const auto mangled = Support::generate_mangling(key, profile);

    it = table.find(mangled);

    if (it != table.end())
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<typename TABLE::mapped_type>
TakeValue(TABLE& table, const typename TABLE::key_type& key, const std::vector<std::string>& profiles)
{
    auto it = table.find(key);

    if (it != table.end())
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    for (const auto& profile : profiles)
    {
        const auto mangled = Support::generate_mangling(key, profile);

        it = table.find(mangled);

        if (it != table.end())
        {
            typename TABLE::mapped_type value = std::move(it->second);
            table.erase(it);
            return value;
        }
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<typename TABLE::mapped_type>
TakeValue(TABLE& table, const typename TABLE::key_type& key, const std::string& profile, const Arcana::Semantic::Attr::Type attr)
{
    auto it = table.find(key);

    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    const auto mangled = Support::generate_mangling(key, profile);

    it = table.find(mangled);

    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    return std::nullopt;
}



template <typename TABLE>
std::optional<typename TABLE::mapped_type>
TakeValue(TABLE& table, const typename TABLE::key_type& key, const std::vector<std::string>& profiles, const Arcana::Semantic::Attr::Type attr)
{
    auto it = table.find(key);

    if (it != table.end() && HasAttrOnMapped(it->second, attr))
    {
        typename TABLE::mapped_type value = std::move(it->second);
        table.erase(it);
        return value;
    }

    for (const auto& profile : profiles)
    {
        const auto mangled = Support::generate_mangling(key, profile);

        it = table.find(mangled);

        if (it != table.end() && HasAttrOnMapped(it->second, attr))
        {
            typename TABLE::mapped_type value = std::move(it->second);
            table.erase(it);
            return value;
        }
    }

    return std::nullopt;
}



template <typename TABLE>
std::vector<typename TABLE::mapped_type>
TakeValues(TABLE& table, const std::string& profile, const Arcana::Semantic::Attr::Type attr)
{
    std::vector<typename TABLE::mapped_type> vec;

    for (auto it = table.begin(); it != table.end(); )
    {
        auto key = it->first;
        ++it;
        auto result = TakeValue(table, key, profile, attr);

        if (result)
        {
            vec.push_back(result.value());
        }
    }

    return vec;
}



template <typename TABLE>
std::vector<typename TABLE::mapped_type>
TakeValues(TABLE& table, const std::vector<std::string>& profiles, const Arcana::Semantic::Attr::Type attr)
{
    std::vector<typename TABLE::mapped_type> vec;

    for (auto it = table.begin(); it != table.end(); )
    {
        auto key = it->first;
        ++it;
        auto result = TakeValue(table, key, profiles, attr);

        if (result)
        {
            vec.push_back(result.value());
        }
    }

    return vec;
}






//    ██████╗ ██╗   ██╗██████╗ ██╗     ██╗ ██████╗    ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
//    ██╔══██╗██║   ██║██╔══██╗██║     ██║██╔════╝    ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
//    ██████╔╝██║   ██║██████╔╝██║     ██║██║         █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
//    ██╔═══╝ ██║   ██║██╔══██╗██║     ██║██║         ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
//    ██║     ╚██████╔╝██████╔╝███████╗██║╚██████╗    ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
//    ╚═╝      ╚═════╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝    ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝
//                                                                                               


inline std::string
ExpandGlob(Semantic::VTable& vtable)
{
    std::stringstream ss;

    for (auto& [var, stmt] : vtable)
    {
        std::vector<std::string> result;
        std::string              full_pattern = stmt.var_value;
        auto                     path         = extract_base_dir(full_pattern);

        if (!path.has_value() || stmt.hasAttribute(Semantic::Attr::Type::MAP))
        {
            continue;
        }

        std::filesystem::path              base_dir     = path.value();
        std::string                        base_str     = base_dir.generic_string();
    
        // pattern relativo rispetto a base_dir
        std::string rel_pattern;
        if (full_pattern.rfind(base_str, 0) == 0)
        {
            std::size_t start = base_str.size();
            if (start < full_pattern.size() && full_pattern[start] == '/')
                ++start;
            rel_pattern = full_pattern.substr(start);
        }
        else
        {
            // fallback: niente base, usa pattern intero
            rel_pattern = full_pattern;
            base_dir    = std::filesystem::current_path();
            base_str    = base_dir.generic_string();
        }

        using RDI = std::filesystem::recursive_directory_iterator;
        
        try
        {
            if (!std::filesystem::exists(base_dir) || 
                !std::filesystem::is_directory(base_dir))
            {
                ss << "Glob expansion failed for variable " << ANSI_BMAGENTA << var << ANSI_RESET << std::endl
                   << "        base directory " << ANSI_BOLD << base_dir << ANSI_RESET << " does not exist";
                break;
            }

            for (auto it = RDI(base_dir); it != RDI(); ++it)
            {
                if (!it->is_regular_file())
                    continue;

                std::filesystem::path rel = std::filesystem::relative(it->path(), base_dir);
                std::string rel_str = rel.generic_string();
        
                if (fnmatch(rel_pattern.c_str(), rel_str.c_str(), 0) == 0)
                {
                    result.push_back(it->path().generic_string());
                }
            }
    
            stmt.glob_expansion = result;
        }
        catch(const std::exception& e)
        {
            ss << "Glob expansion failed for variable " << ANSI_BMAGENTA << var << ANSI_RESET << std::endl
               << "        base directory " << ANSI_BOLD << base_dir << ANSI_RESET << " does not exist";
            break;
        }
    }

    return ss.str();
}



inline std::string
Map(Semantic::VTable& vtable)
{
    std::stringstream ss;

    auto glob_regex = [] (const std::string &pattern) noexcept -> std::string
    {
        static const std::string regex_meta = R"(.^$|()[]+?{}\)";
        std::string re;
        re.reserve(pattern.size() * 2);
        re.push_back('^');

        for (std::size_t i = 0; i < pattern.size(); )
        {
            if (i + 1 < pattern.size() && pattern[i] == '*' && pattern[i + 1] == '*')
            {
                re += "(.*)";  // un gruppo catturante
                i += 2;
                continue;
            }
            else if (i + 1 < pattern.size() && pattern[i] == '*')
            {
                re += "(.*)";  // un gruppo catturante
                i += 1;
                continue;
            }

            char c = pattern[i++];
            if (regex_meta.find(c) != std::string::npos)
                re.push_back('\\'); // escape metacaratteri regex

            re.push_back(c);
        }

        re.push_back('$');
        return re;
    };

    namespace fs = std::filesystem;

    auto map_required = Table::GetValues(vtable, Semantic::Attr::Type::MAP);
    
    if (!map_required) return ss.str();
    
    for (auto& stmt : map_required.value())
    {
        auto& map_to   = stmt.get();
        auto& map_from = vtable[map_to.getProperties(Semantic::Attr::Type::MAP).at(0)];

        if (map_from.glob_expansion.size() == 0)
        {
            ss << "MAP error: cannot use non glob varaiable to map " << ANSI_BMAGENTA << map_to.var_name << ANSI_RESET;
            return ss.str();
        }

        std::string re_str = glob_regex(map_from.var_value);
        std::regex  re(re_str);

        std::vector<std::string> result;
    
        for (const auto& src : map_from.glob_expansion)
        {
            std::smatch m;
    
            if (!std::regex_match(src, m, re))
            {
                continue;
            }
    
            // gruppo catturato (quello che stava al posto di **)
            std::string middle = m[1].str();
            std::string out    = map_to.var_value;

            auto pos = out.find("**");
            if (pos == std::string::npos)
            {
                ss << "MAP error: destination pattern for " ANSI_BMAGENTA << map_to.var_name << ANSI_RESET " does not contain '**'";
                return ss.str();
            }

            out.replace(pos, 2, middle);
            result.emplace_back(out);
        }
    
        map_to.glob_expansion = result;
    }

    return "";
}


template <typename TABLE>
std::vector<typename TABLE::key_type>
Keys(const TABLE& table)
{
    std::vector<typename TABLE::key_type> keys;

    for (const auto& [k, v] : table)
    {
        keys.push_back(k);
    }

    return keys;
}



template <typename TABLE>
void
AlignOnProfile(TABLE& table, const std::string& profile)
{
    using Key = typename TABLE::key_type;

    std::vector<Key> mangled_keys;
    mangled_keys.reserve(table.size());

    for (const auto& [k, v] : table)
    {
        auto pos = k.find("@@");
        if (pos != std::string::npos)
            mangled_keys.push_back(k);
    }

    for (const auto& mk : mangled_keys)
    {
        auto it = table.find(mk);

        if (it == table.end()) continue;

        const Key& key = it->first;
        const auto pos = key.find("@@");
        if (pos == std::string::npos) continue;

        const std::string base     = key.substr(0, pos);
        const std::string prof_key = key.substr(pos + 2);

        if (prof_key != profile)
        {
            table.erase(it);
            continue;
        }

        auto base_it = table.find(base);
        if (base_it != table.end())
        {
            base_it->second = std::move(it->second);
            table.erase(it);
        }
        else
        {
            auto node = table.extract(it);
            node.key() = base;
            table.insert(std::move(node));
        }
    }
}



END_MODULE(Table)


#endif /* __ARCANA_UTIL_TABLE_HELPER__H__ */