#ifndef __ARCANA_TABLE_HELPER__H__
#define __ARCANA_TABLE_HELPER__H__


///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "Support.h"
#include "Defines.h"





//    ████████  █████  ██████  ██      ███████ ██   ██ ███████ ██      ██████  ███████ ██████            
//       ██    ██   ██ ██   ██ ██      ██      ██   ██ ██      ██      ██   ██ ██      ██   ██           
//       ██    ███████ ██████  ██      █████   ███████ █████   ██      ██████  █████   ██████            
//       ██    ██   ██ ██   ██ ██      ██      ██   ██ ██      ██      ██      ██      ██   ██           
//       ██    ██   ██ ██████  ███████ ███████ ██   ██ ███████ ███████ ██      ███████ ██   ██           
//                                                                                                       
//                                                                                                       
//    ███████  ██████  ██████  ██     ██  █████  ██████  ██████  ███████ ██████                          
//    ██      ██    ██ ██   ██ ██     ██ ██   ██ ██   ██ ██   ██ ██      ██   ██                         
//    █████   ██    ██ ██████  ██  █  ██ ███████ ██████  ██   ██ █████   ██   ██                         
//    ██      ██    ██ ██   ██ ██ ███ ██ ██   ██ ██   ██ ██   ██ ██      ██   ██                         
//    ██       ██████  ██   ██  ███ ███  ██   ██ ██   ██ ██████  ███████ ██████                          
//                                                                                                       
//                                                                                                       
//    ██████  ███████ ███    ███  ██████  ████████ ███████     ████████ ██    ██ ██████  ███████ ███████ 
//    ██   ██ ██      ████  ████ ██    ██    ██    ██             ██     ██  ██  ██   ██ ██      ██      
//    ██████  █████   ██ ████ ██ ██    ██    ██    █████          ██      ████   ██████  █████   ███████ 
//    ██   ██ ██      ██  ██  ██ ██    ██    ██    ██             ██       ██    ██      ██           ██ 
//    ██   ██ ███████ ██      ██  ██████     ██    ███████        ██       ██    ██      ███████ ███████ 
//                                                                                                       
//                                                                                                                                                                                                                                 


BEGIN_MODULE(Semantic::Attr)

enum class Type;

END_MODULE(Semantic::Attr)




///////////////////////////////////////////////////////////////////////////////
// MODULE NEMASPACEs
///////////////////////////////////////////////////////////////////////////////

BEGIN_MODULE(Table)




//    ████████  █████  ██████  ██      ███████ ██   ██ ███████ ██      ██████  ███████ ██████                                   
//       ██    ██   ██ ██   ██ ██      ██      ██   ██ ██      ██      ██   ██ ██      ██   ██                                  
//       ██    ███████ ██████  ██      █████   ███████ █████   ██      ██████  █████   ██████                                   
//       ██    ██   ██ ██   ██ ██      ██      ██   ██ ██      ██      ██      ██      ██   ██                                  
//       ██    ██   ██ ██████  ███████ ███████ ██   ██ ███████ ███████ ██      ███████ ██   ██                                  
//                                                                                                                              
//                                                                                                                              
//    ██████  ██    ██ ██████  ██      ██  ██████     ███████ ██    ██ ███    ██  ██████ ████████ ██  ██████  ███    ██ ███████ 
//    ██   ██ ██    ██ ██   ██ ██      ██ ██          ██      ██    ██ ████   ██ ██         ██    ██ ██    ██ ████   ██ ██      
//    ██████  ██    ██ ██████  ██      ██ ██          █████   ██    ██ ██ ██  ██ ██         ██    ██ ██    ██ ██ ██  ██ ███████ 
//    ██      ██    ██ ██   ██ ██      ██ ██          ██      ██    ██ ██  ██ ██ ██         ██    ██ ██    ██ ██  ██ ██      ██ 
//    ██       ██████  ██████  ███████ ██  ██████     ██       ██████  ██   ████  ██████    ██    ██  ██████  ██   ████ ███████ 
//                                                                                                                              
//                                                                                                                              



template <typename T>
inline bool HasAttrOnMapped(const T& value, const Arcana::Semantic::Attr::Type attr)
{
    return value.hasAttribute(attr);
}

template <typename T>
inline bool HasAttrOnMapped(T* ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
}

template <typename T>
inline bool HasAttrOnMapped(const std::shared_ptr<T>& ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
}

template <typename T>
inline bool HasAttrOnMapped(const std::unique_ptr<T>& ptr, const Arcana::Semantic::Attr::Type attr)
{
    return ptr && ptr->hasAttribute(attr);
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




END_MODULE(Table)


#endif /* __ARCANA_TABLE_HELPER__H__ */