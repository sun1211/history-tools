// copyright defined in LICENSE.txt

#pragma once

#include "abieos_exception.hpp"

namespace query_config {

template <typename Defs>
struct field {
    std::string                name       = {};
    std::string                short_name = {};
    std::string                type       = {};
    const typename Defs::type* type_obj   = {};
};

template <typename Defs, typename F>
constexpr void for_each_field(field<Defs>*, F f) {
    f("name", abieos::member_ptr<&field<Defs>::name>{});
    f("short_name", abieos::member_ptr<&field<Defs>::short_name>{});
    f("type", abieos::member_ptr<&field<Defs>::type>{});
};

template <typename Defs>
struct key {
    std::string           name           = {};
    std::string           new_name       = {};
    std::string           type           = {};
    std::string           expression     = {};
    std::string           arg_expression = {};
    bool                  desc           = {};
    typename Defs::field* field          = {};
};

template <typename Defs, typename F>
constexpr void for_each_field(key<Defs>*, F f) {
    f("name", abieos::member_ptr<&key<Defs>::name>{});
    f("new_name", abieos::member_ptr<&key<Defs>::new_name>{});
    f("type", abieos::member_ptr<&key<Defs>::type>{});
    f("expression", abieos::member_ptr<&key<Defs>::expression>{});
    f("arg_expression", abieos::member_ptr<&key<Defs>::arg_expression>{});
    f("desc", abieos::member_ptr<&key<Defs>::desc>{});
};

template <typename Defs>
struct table {
    std::string                                  name         = {};
    std::vector<typename Defs::field>            fields       = {};
    std::vector<typename Defs::type>             types        = {};
    std::vector<typename Defs::key>              history_keys = {};
    std::vector<typename Defs::key>              keys         = {};
    std::map<std::string, typename Defs::field*> field_map    = {};
};

template <typename Defs, typename F>
constexpr void for_each_field(table<Defs>*, F f) {
    f("name", abieos::member_ptr<&table<Defs>::name>{});
    f("fields", abieos::member_ptr<&table<Defs>::fields>{});
    f("history_keys", abieos::member_ptr<&table<Defs>::history_keys>{});
    f("keys", abieos::member_ptr<&table<Defs>::keys>{});
};

template <typename Defs>
struct query {
    abieos::name                     wasm_name            = {};
    std::string                      index                = {};
    std::string                      function             = {};
    std::string                      table                = {};
    bool                             is_state             = {};
    bool                             limit_block_index    = {};
    uint32_t                         max_results          = {};
    std::string                      join                 = {};
    abieos::name                     join_query_wasm_name = {};
    std::vector<typename Defs::key>  args                 = {};
    std::vector<typename Defs::key>  sort_keys            = {};
    std::vector<typename Defs::key>  join_key_values      = {};
    std::vector<typename Defs::key>  fields_from_join     = {};
    std::vector<std::string>         conditions           = {};
    std::vector<typename Defs::type> arg_types            = {};
    std::vector<typename Defs::type> range_types          = {};
    std::vector<typename Defs::type> result_types         = {};
    typename Defs::table*            table_obj            = {};
    typename Defs::table*            join_table           = {};
    query*                           join_query           = {};
};

template <typename Defs, typename F>
constexpr void for_each_field(query<Defs>*, F f) {
    f("wasm_name", abieos::member_ptr<&query<Defs>::wasm_name>{});
    f("index", abieos::member_ptr<&query<Defs>::index>{});
    f("function", abieos::member_ptr<&query<Defs>::function>{});
    f("table", abieos::member_ptr<&query<Defs>::table>{});
    f("is_state", abieos::member_ptr<&query<Defs>::is_state>{});
    f("limit_block_index", abieos::member_ptr<&query<Defs>::limit_block_index>{});
    f("max_results", abieos::member_ptr<&query<Defs>::max_results>{});
    f("join", abieos::member_ptr<&query<Defs>::join>{});
    f("join_query_wasm_name", abieos::member_ptr<&query<Defs>::join_query_wasm_name>{});
    f("args", abieos::member_ptr<&query<Defs>::args>{});
    f("sort_keys", abieos::member_ptr<&query<Defs>::sort_keys>{});
    f("join_key_values", abieos::member_ptr<&query<Defs>::join_key_values>{});
    f("fields_from_join", abieos::member_ptr<&query<Defs>::fields_from_join>{});
    f("conditions", abieos::member_ptr<&query<Defs>::conditions>{});
};

template <typename Defs, typename Key>
void set_key_fields(const table<Defs>& tab, std::vector<Key>& keys) {
    for (auto& k : keys) {
        auto it = tab.field_map.find(k.name);
        if (it == tab.field_map.end())
            throw std::runtime_error("key references unknown field " + k.name + " in table " + k.name);
        k.field = it->second;
    }
}

template <typename Defs>
struct config {
    std::vector<typename Defs::table>             tables    = {};
    std::vector<typename Defs::query>             queries   = {};
    std::map<std::string, typename Defs::table*>  table_map = {};
    std::map<abieos::name, typename Defs::query*> query_map = {};

    template <typename M>
    void prepare(const M& type_map) {
        for (auto& table : tables) {
            table_map[table.name] = &table;
            for (auto& field : table.fields) {
                table.field_map[field.name] = &field;
                auto it                     = type_map.find(field.type);
                if (it == type_map.end())
                    throw std::runtime_error("table " + table.name + " field " + field.name + ": unknown type: " + field.type);
                field.type_obj = &it->second;
                table.types.push_back(it->second);
            }
            set_key_fields(table, table.history_keys);
            set_key_fields(table, table.keys);
        }

        for (auto& query : queries) {
            query_map[query.wasm_name] = &query;
            auto it                    = table_map.find(query.table);
            if (it == table_map.end())
                throw std::runtime_error("query " + (std::string)query.wasm_name + ": unknown table: " + query.table);
            query.table_obj = it->second;
            set_key_fields(*query.table_obj, query.args);
            set_key_fields(*query.table_obj, query.sort_keys);
            set_key_fields(*query.table_obj, query.join_key_values);
            for (auto& arg : query.args) {
                auto type_it = type_map.find(arg.type);
                if (type_it == type_map.end())
                    throw std::runtime_error("query " + (std::string)query.wasm_name + " arg " + arg.name + ": unknown type: " + arg.type);
                query.arg_types.push_back(type_it->second);
            }
            auto add_types = [&](auto& dest, auto& fields, auto* t) {
                for (auto& key : fields) {
                    std::string type = key.type;
                    if (type.empty()) {
                        auto field_it = t->field_map.find(key.name);
                        if (field_it == t->field_map.end())
                            throw std::runtime_error("query " + (std::string)query.wasm_name + ": unknown field: " + key.name);
                        type = field_it->second->type;
                    }

                    auto type_it = type_map.find(type);
                    if (type_it == type_map.end())
                        throw std::runtime_error("query " + (std::string)query.wasm_name + " key " + key.name + ": unknown type: " + type);
                    dest.push_back(type_it->second);
                }
            };
            add_types(query.range_types, query.sort_keys, query.table_obj);

            query.result_types = query.table_obj->types;
            if (!query.join.empty()) {
                auto it = table_map.find(query.join);
                if (it == table_map.end())
                    throw std::runtime_error("query " + (std::string)query.wasm_name + ": unknown table: " + query.join);
                query.join_table = it->second;
                add_types(query.result_types, query.fields_from_join, query.join_table);
                set_key_fields(*query.join_table, query.fields_from_join);

                auto it2 = query_map.find(query.join_query_wasm_name);
                if (it2 == query_map.end())
                    throw std::runtime_error(
                        "query " + (std::string)query.wasm_name +
                        ": unknown join_query_wasm_name: " + (std::string)query.join_query_wasm_name);
                query.join_query = it2->second;
            }
        }
    } // prepare()
};    // config

template <typename Defs, typename F>
constexpr void for_each_field(config<Defs>*, F f) {
    f("tables", abieos::member_ptr<&config<Defs>::tables>{});
    f("queries", abieos::member_ptr<&config<Defs>::queries>{});
};

} // namespace query_config