#include "map_renderer.h"

bool IsZero(double value) {
  return std::abs(value) < EPSILON;
}
namespace render {

void LoadRenderInformation(std::ostream& out, const TransportCatalogue& catalogue, const json::Dict& info) {
  svg::Document doc;
  FirstLayer(catalogue, info, doc);
  // дальше запустятся второй, третий и четвертый слои
  doc.Render(out);
}
void PushCoordinates(std::vector<geo::Coordinates>& ans, const std::vector<TransportCatalogue::Stop*>& stops ) {
  for(auto stop: stops) {
    ans.push_back(stop->coordinates);
  }
}

void FirstLayer(const TransportCatalogue& catalogue, const json::Dict& dict,   svg::Document& doc) {
  auto buses = catalogue.GetAllBuses();
  double line_width = dict.at("line_width").AsDouble();
  double width = dict.at("width").AsDouble();
  double height = dict.at("height").AsDouble();
  double padding = dict.at("padding").AsDouble();
  std::vector<geo::Coordinates> cords;
  json::Array color_palette = dict.at("color_palette").AsArray();
  for (auto bus: buses) {
    PushCoordinates(cords, catalogue.FindBus(bus)->route);
  }
  SphereProjector projector(cords.begin(), cords.end(), width, height, padding);
  int cnt = 0;
  for (auto bus: buses) {
    auto tmp_route = catalogue.FindBus(bus)->route;
    svg::Polyline line;
    for(auto stop: tmp_route) {
      line.AddPoint(projector(stop->coordinates));
    }

    auto color = GetColorFromArray(color_palette, cnt);
    if (!tmp_route.empty()) {
      ++cnt;
    }
    line.SetFillColor(svg::NoneColor).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeWidth(line_width).SetStrokeColor(color);

    doc.Add(line);
  }
  SecondLayer(catalogue, dict, doc, projector, buses);
}


svg::Color GetColorFromArray(const json::Array& colors, int curbus) {
  int pos = curbus % static_cast<int>(colors.size());
  auto curcolor = colors[pos];
  if (curcolor.IsString()) {
    return curcolor.AsString();
  }
  if (curcolor.IsArray()) {
    auto tmp_col = curcolor.AsArray();
    auto r = static_cast<uint8_t>(tmp_col[0].AsInt());
    auto g = static_cast<uint8_t>(tmp_col[1].AsInt());
    auto b = static_cast<uint8_t>(tmp_col[2].AsInt());
    if (tmp_col.size() == 4) {
      auto o = tmp_col[3].AsDouble();
      return svg::Rgba(r, g, b, o);
    } else {
      return svg::Rgb(r, g, b);
    }
  }

  return std::monostate();
}

void SecondLayer(const TransportCatalogue& catalogue, const json::Dict& dict,  svg::Document& doc, const SphereProjector& projector
, const std::set<std::string_view>& buses) {
  int cnt = 0;
  //получаю из json информацию для рендеринга. Эта часть кода отвечает за рендеринг названий остановок
  json::Array underlayer_color;
  underlayer_color.push_back(dict.at("underlayer_color"));
  auto undcolor = GetColorFromArray(underlayer_color, cnt);
  json::Array color_palette = dict.at("color_palette").AsArray();
  int bus_label_font_size = dict.at("bus_label_font_size").AsInt();
  json::Array bus_label_offset = dict.at("bus_label_offset").AsArray();
  svg::Text text_und, text_main;
  double und_width = dict.at("underlayer_width").AsDouble();


  text_main.SetFontFamily("Verdana").SetFontWeight("bold");
  text_main.SetFontSize((bus_label_font_size));
  text_main.SetOffset({bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()});
  text_und = text_main;
  text_und.SetStrokeWidth(und_width);
  text_und.SetFillColor(undcolor).SetStrokeColor(undcolor);
  text_und.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND);
for(auto bus: buses) {
  auto tmp_route = catalogue.FindBus(bus)->route;
  if(tmp_route.empty()) {
    continue;
  }
  auto color = GetColorFromArray(color_palette, cnt);
  ++cnt;
  text_main.SetFillColor(color);
  auto tmp_bus = catalogue.FindBus(bus);\
  text_main.SetData(std::string(bus));
  text_und.SetData(std::string(bus));
  if(tmp_bus->is_roundtrip) {
    text_main.SetPosition(projector(tmp_route[0]->coordinates));
    text_und.SetPosition(projector(tmp_route[0]->coordinates));
    doc.Add(text_und);
    doc.Add(text_main);
  }
  else {
    if(tmp_route[0] ==tmp_route[tmp_route.size() / 2]) {
      text_main.SetPosition(projector(tmp_route[0]->coordinates));
      text_und.SetPosition(projector(tmp_route[0]->coordinates));
      doc.Add(text_und);
      doc.Add(text_main);
    }
    else {
      text_main.SetPosition(projector(tmp_route[0]->coordinates));
      text_und.SetPosition(projector(tmp_route[0]->coordinates));
      doc.Add(text_und);
      doc.Add(text_main);
      text_main.SetPosition(projector(tmp_route[tmp_route.size() / 2]->coordinates));
      text_und.SetPosition(projector(tmp_route[tmp_route.size() / 2]->coordinates));
      doc.Add(text_und);
      doc.Add(text_main);

    }

  }
}
  ThirdLayer(catalogue, dict, doc, projector, buses);
}

void ThirdLayer(const TransportCatalogue& catalogue, const json::Dict& dict,  svg::Document& doc, const SphereProjector& projector
    , const std::set<std::string_view>& buses) {
  std::set<std::string_view > stops;
  double stop_rad = dict.at("stop_radius").AsDouble();
  for(const auto bus : buses) {
    auto tmp_bus = catalogue.FindBus(bus);
    for(const auto stop: tmp_bus->route) {
      stops.insert(stop->name);
    }
  }
  for(const auto stop: stops) {
    doc.Add(svg::Circle().SetRadius(stop_rad).SetCenter(projector(catalogue.FindStop(stop)->coordinates)).SetFillColor("white"));
  }
  FourthLayer(catalogue, dict, doc, projector, stops);



}

void FourthLayer(const TransportCatalogue& catalogue, const json::Dict& dict, svg::Document& doc, const SphereProjector& projector, const std::set<std::string_view>& stops) {
  json::Array underlayer_color;
  underlayer_color.push_back(dict.at("underlayer_color"));
  auto undcolor = GetColorFromArray(underlayer_color, 0);
  int stop_label_font_size = dict.at("stop_label_font_size").AsInt();
  json::Array stop_label_offset = dict.at("stop_label_offset").AsArray();
  svg::Text text_und, text_main;
  double und_width = dict.at("underlayer_width").AsDouble();


  text_main.SetFontFamily("Verdana");
  text_main.SetFontSize((stop_label_font_size));
  text_main.SetOffset({stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()});
  text_und = text_main;
  text_und.SetStrokeWidth(und_width);
  text_und.SetFillColor(undcolor).SetStrokeColor(undcolor);
  text_und.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND);
  for( const auto stop: stops) {
    text_main.SetData(std::string (stop)).SetPosition(projector(catalogue.FindStop(stop)->coordinates));
    text_und.SetData(std::string (stop)).SetPosition(projector(catalogue.FindStop(stop)->coordinates));
    text_main.SetFillColor("black");
    doc.Add(text_und);
    doc.Add(text_main);
  }

}

}