#include "svg.h"

#include <utility>

namespace svg {
std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
  switch (line_join) {
    case StrokeLineJoin::ARCS:os << "arcs";
      break;
    case StrokeLineJoin::BEVEL:os << "bevel";
      break;
    case StrokeLineJoin::MITER:os << "miter";
      break;
    case StrokeLineJoin::MITER_CLIP:os << "miter-clip";
      break;
    case StrokeLineJoin::ROUND:os << "round";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
  switch (line_cap) {
    case StrokeLineCap::BUTT:os << "butt";
      break;
    case StrokeLineCap::ROUND:os << "round";
      break;
    case StrokeLineCap::SQUARE:os << "square";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Color& color){
  std::ostringstream strm;
  std::visit(ColorPrinter{strm}, color);
  os << strm.str();
  return os;
}

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);

  context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle& Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
  out << " r=\""sv << radius_ << "\""sv;
  RenderAttrs(context.out);
  out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
  points.push_back(point);
  return *this;
}
void Polyline::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<polyline points=\"";
  bool flag = true;
  for (auto point : points) {
    if (!flag) {
      out << " ";
    }
    out << point.x << "," << point.y;
    flag = false;
  }
  out << "\"";
  RenderAttrs(context.out);
  out << "/>";
}

void Document::Render(std::ostream& out) const {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
  bool flag = true;
  for (auto& obj : objects_) {
    if (!flag) {
      //out <<"\n";
    }
    out << "  ";
    obj->Render(out);
    flag = false;
  }
  out << "</svg>"sv;
}
Text& Text::SetPosition(Point pos) {
  position_ = pos;
  return *this;
}
Text& Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}
Text& Text::SetFontSize(uint32_t size) {
  size_ = size;
  return *this;
}
Text& Text::SetFontFamily(std::string font_family) {
  font_family_ = std::move(font_family);
  return *this;
}
Text& Text::SetFontWeight(const std::string& font_weight) {
  font_weight_ = font_weight;
  return *this;
}
Text& Text::SetData(const std::string& data) {
  data_ = data;
  return *this;
}
std::string Text::ParseString(const std::string& query) const {
  std::string ans;
  for (auto i : query) {
    if (i == '"') {
      ans += "&quot;";
    } else if (i == '\'') {
      ans += "&apos;";
    } else if (i == '<') {
      ans += "&lt;";
    } else if (i == '>') {
      ans += "&gt;";
    } else if (i == '&') {
      ans += "&amp;";
    } else {
      ans.push_back(i);
    }

  }
  return ans;
}

void Text::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<text";
  RenderAttrs(context.out);
  out << " x=\"" << position_.x << "\" y=\"" << position_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
      << "\" ";
  out << "font-size=\"" << size_ << "\"";
  if (!font_family_.empty()) {
    out << " font-family=\"" << font_family_ << "\"";
  }
  if (!font_weight_.empty()) {
    out << " font-weight=\"" << font_weight_ << "\"";
  }

  out << ">";
  auto text = this->ParseString(data_);
  out << text << "</text>";

}

}  // namespace svg

namespace shapes {
Star::Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
  for (int i = 0; i <= num_rays; ++i) {
    double angle = 2 * M_PI * (i % num_rays) / num_rays;
    polyline_.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
    if (i == num_rays) {
      break;
    }
    angle += M_PI / num_rays;
    polyline_.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    polyline_.SetFillColor("red").SetStrokeColor("black");
  }
}

Snowman::Snowman(svg::Point centre, double rad) {
  c1.SetCenter(centre).SetRadius(rad);
  c2.SetCenter({centre.x, centre.y + 2 * rad}).SetRadius(rad * 1.5);
  c3.SetCenter({centre.x, centre.y + 5 * rad}).SetRadius(rad * 2);
  c1.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");
  c2.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");
  c3.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");

}
}