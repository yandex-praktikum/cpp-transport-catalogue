#pragma once

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

#include <filesystem>
#include "json.h"

#include "transport_catalogue.h"

using Path = std::filesystem::path;



void Serialize(const Path& file, const TransportCatalogue& catalogue, const json::Dict& rend_info, const json::Dict& router_info);


void SerializeCatalogue( const TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& catalogue1);

// функция сериализует rend info из json::Dict
void SerializeRenderSetting( const json::Dict& rend_info, map_rendered_serialize::RenderSetting& setting);

TransportCatalogue DeserializeCatalogue(const transport_catalogue_serialize::TransportCatalogue& catalogue);
json::Dict DeserializeSetting(const map_rendered_serialize::RenderSetting& setting);

map_rendered_serialize::Color SerializeColor( const json::Array& color);

json::Node DeserializeColor(const map_rendered_serialize::Color& setting);

void SerializeRouterInfo(const json::Dict& router_info, transport_catalogue_serialize::RoutInfo& proto_router);

std::pair<int, int> DeserializeRouter(const transport_catalogue_serialize::RoutInfo& proto_router);

