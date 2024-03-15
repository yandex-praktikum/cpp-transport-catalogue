#include <fstream>
#include "serialization.h"


void SerializeCatalogue(const TransportCatalogue& catalogue, transport_catalogue_serialize::TransportCatalogue& transportCatalogue) {

    std::unordered_map<std::string_view, int> string_to_id;

    //возможно код работает долго из-за строк
    int id = 0;
    for (const auto &[stop, stop_pointer]: catalogue.GetAllStops()) {
        transport_catalogue_serialize::Stop *stop_proto = transportCatalogue.mutable_stops()->add_stops();
        stop_proto->set_name(std::move(std::string(stop_pointer->name)));
        stop_proto->mutable_cords()->set_lat(stop_pointer->coordinates.lat);
        stop_proto->mutable_cords()->set_long_(stop_pointer->coordinates.lng);
        if (!string_to_id.count(stop_pointer->name)) {
            stop_proto->set_id(id);
            string_to_id[stop_pointer->name] = id++;

        }
    }
    for (const auto &[ignore, bus]: catalogue.GetAllBusesS()) {
        transport_catalogue_serialize::Bus *proto_bus = transportCatalogue.mutable_buses()->add_buses();
        proto_bus->set_name(std::move(std::string(bus->name)));
        for (const auto &stop: bus->route) {
            transport_catalogue_serialize::Stop *stop_proto = proto_bus->add_route();
            stop_proto->set_name(std::move(std::string(stop->name)));
            stop_proto->mutable_cords()->set_lat(stop->coordinates.lat);
            stop_proto->mutable_cords()->set_long_(stop->coordinates.lng);
        }
        proto_bus->set_is_roundtrip(bus->is_roundtrip);
    }

    // использую словарь, чтобы в итоговом векторе были только уникальные пары расстояний

    for (const auto &[pair_stop, dist]: catalogue.GetAllDistances()) {
        const auto &[stop_name_1, stop_name_2] = pair_stop;

        transport_catalogue_serialize::PairStopDistance *distance = transportCatalogue.add_dists();
        distance->set_distance(dist);
        distance->set_stop_name_1(string_to_id[stop_name_1->name]);
        distance->set_stop_name_2(string_to_id[stop_name_2->name]);
    }

}

TransportCatalogue DeserializeCatalogue(const transport_catalogue_serialize::TransportCatalogue& protoCatalogue) {
    {
        TransportCatalogue catalogue;

        //собираем id по остановкам

        std::unordered_map<int, std::string_view > id_to_stop;
        for (const auto &proto_stop: protoCatalogue.stops().stops()) {
            auto iter = catalogue.InsertWordInAllWords(proto_stop.name());
            auto name = iter.first->c_str();
            catalogue.AddStop({name, {proto_stop.cords().lat(), proto_stop.cords().long_()}});
            if (!id_to_stop.count(proto_stop.id())) {
                id_to_stop[proto_stop.id()] = name;
            }
        }

        for (const auto &proto_bus: protoCatalogue.buses().buses()) {
            auto iter = catalogue.InsertWordInAllWords(proto_bus.name());
            auto name = iter.first->c_str();
            std::vector<std::string_view> route;

            for (const auto &proto_stop: proto_bus.route()) {
                //auto stop_iter = catalogue.InsertWordInAllWords(proto_stop.name());
                route.emplace_back(proto_stop.name().c_str());
            }
            catalogue.AddBus(name, route, proto_bus.is_roundtrip());
        }

        for (const auto &proto_dist: protoCatalogue.dists()) {
           // auto iter1 = catalogue.InsertWordInAllWords(id_to_stop[proto_dist.stop_name_1()]);
            //auto name1 = iter1.first->c_str();
            //auto iter2 = catalogue.InsertWordInAllWords(proto_dist.stop_name_2());
            //auto name2 = iter2.first->c_str();
            catalogue.AddDistanceInfo(id_to_stop[proto_dist.stop_name_1()], id_to_stop[proto_dist.stop_name_2()], proto_dist.distance());
        }
        return catalogue;
    }
}
json::Dict DeserializeSetting(const map_rendered_serialize::RenderSetting& proto_settings) {
    json::Dict  ans;

    ans["width"] = proto_settings.width();
    ans["height"] = proto_settings.height();
    ans["line_width"] = proto_settings.line_width();
    ans["padding"] = proto_settings.padding();
    ans["stop_radius"] = proto_settings.stop_radius();
    ans["bus_label_font_size"] = proto_settings.bus_label_font_size();
    ans["stop_label_font_size"] = proto_settings.stop_label_font_size();
    ans["underlayer_width"] = proto_settings.underlayer_width();

    ans["stop_label_offset"] = json::Array{proto_settings.stop_label_offset().dx(), proto_settings.stop_label_offset().dy()};


    ans["bus_label_offset"] = json::Array{proto_settings.bus_label_offset().dx(), proto_settings.bus_label_offset().dy()};

    auto tmp = DeserializeColor(proto_settings.underlayer_color());
    if (tmp.IsString()) {
        ans["underlayer_color"] = tmp.AsString();
    } else {
        ans["underlayer_color"] = tmp.AsArray();
    }


    json::Array tmp_array;
    for (const auto& color : proto_settings.color_palette()) {
        tmp_array.push_back(std::move(DeserializeColor(color)));
    }

    ans["color_palette"] = tmp_array;

    return ans;
}

json::Node DeserializeColor(const map_rendered_serialize::Color& color) {
    if (color.type() == 0) {
        return color.color();
    }
    else if(color.type() == 1) {
        return json::Array {color.rgb().r(), color.rgb().g(), color.rgb().b()};
    } else {
        return json::Array {color.rgba().rgb().r(), color.rgba().rgb().g(), color.rgba().rgb().b(), color.rgba().opacity()};
    }
}

void SerializeRenderSetting(const json::Dict& rend_info, map_rendered_serialize::RenderSetting& proto_setting) {

    proto_setting.set_line_width(rend_info.at("line_width").AsDouble());
    proto_setting.set_width(rend_info.at("width").AsDouble());
    proto_setting.set_height(rend_info.at("height").AsDouble());
    proto_setting.set_padding(rend_info.at("padding").AsDouble());
    proto_setting.set_stop_radius(rend_info.at("stop_radius").AsDouble());
    proto_setting.set_bus_label_font_size(rend_info.at("bus_label_font_size").AsInt());
    proto_setting.set_stop_label_font_size(rend_info.at("stop_label_font_size").AsInt());


    map_rendered_serialize::LabelOffset stop_new_offset;

    stop_new_offset.set_dx(rend_info.at("stop_label_offset").AsArray()[0].AsDouble());
    stop_new_offset.set_dy(rend_info.at("stop_label_offset").AsArray()[1].AsDouble());

    *proto_setting.mutable_stop_label_offset() = stop_new_offset;

    auto underlayer_color = rend_info.at("underlayer_color");

    if (underlayer_color.IsString()) {
        map_rendered_serialize::Color new_color;
        new_color.set_color((underlayer_color.AsString()));
        new_color.set_type(0);
        *proto_setting.mutable_underlayer_color() = new_color;
    } else {
        *proto_setting.mutable_underlayer_color() = SerializeColor( underlayer_color.AsArray());
    }
    auto color = rend_info.at("color_palette").AsArray();
    for (auto col : color) {
        if (col.IsString()) {
            map_rendered_serialize::Color new_color;
            new_color.set_color(col.AsString());
            new_color.set_type(0);
            *proto_setting.add_color_palette() = std::move(new_color);
        }
        else if (col.IsArray()){
            *proto_setting.add_color_palette() = SerializeColor(col.AsArray());
        }
    }
    proto_setting.set_underlayer_width(rend_info.at("underlayer_width").AsDouble());
    map_rendered_serialize::LabelOffset bus_new_offset;
    bus_new_offset.set_dx(rend_info.at("bus_label_offset").AsArray()[0].AsDouble());
    bus_new_offset.set_dy(rend_info.at("bus_label_offset").AsArray()[1].AsDouble());

    *proto_setting.mutable_bus_label_offset() = bus_new_offset;
}




void Serialize(const Path& file, const TransportCatalogue& catalogue, const json::Dict& rend_indo, const json::Dict& router_info)
{ //из-за перезаписи файлов объединям в одну струтуру
    transport_catalogue_serialize::CommonInfo all_info;
    SerializeCatalogue(catalogue, *all_info.mutable_catalogue());
    SerializeRenderSetting( rend_indo, *all_info.mutable_render_settings());
    SerializeRouterInfo(router_info, *all_info.mutable_router());


    std::string serialized_data;
    if (!all_info.SerializeToString(&serialized_data)) {
        std::cerr << "Failed to serialize data." << std::endl;
        // Обработка ошибки
    }

    std::ofstream out(file, std::ios::binary);
    out.write(serialized_data.data(), serialized_data.size());
    out.close();
}

map_rendered_serialize::Color SerializeColor(const json::Array& color) {
    map_rendered_serialize::Color new_color;
    if (color.size() == 3) {
        // RGB color
        new_color.mutable_rgb()->set_r(color[0].AsInt());
        new_color.mutable_rgb()->set_g(color[1].AsInt());
        new_color.mutable_rgb()->set_b(color[2].AsInt());
        new_color.set_type(1);
    } else if (color.size() == 4) {
        // RGBA color
        new_color.mutable_rgba()->mutable_rgb()->set_r(color[0].AsInt());
        new_color.mutable_rgba()->mutable_rgb()->set_g(color[1].AsInt());
        new_color.mutable_rgba()->mutable_rgb()->set_b(color[2].AsInt());
        new_color.mutable_rgba()->set_opacity(color[3].AsDouble());
        new_color.set_type(2);
    }

    return new_color;
}

void SerializeRouterInfo(const json::Dict& router_info, transport_catalogue_serialize::RoutInfo& proto_router) {
    int time = router_info.at("bus_wait_time").AsInt();
    int velocity = router_info.at("bus_velocity").AsInt();
    proto_router.set_time(time);
    proto_router.set_velocity(velocity);
}

std::pair<int, int> DeserializeRouter(const transport_catalogue_serialize::RoutInfo& proto_router) {
    int time = proto_router.time();
    int velocity = proto_router.velocity();
    return {time , velocity};
}





